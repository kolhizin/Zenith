#pragma once
#include "GeometryNodes.h"
#include "Params\Config.h"
#include <Utils\Math\categorical_distribution.h>
#include <random>

namespace zenith
{
	namespace terragen
	{
		class MountainTopNode : public PointNode
		{
			double sizeParam_;
		public:
			static constexpr char * ClassName = "Mountain/Top";

			inline MountainTopNode(const GVec3 &p, double sizeParam, const BaseNode * parent, const BaseNodeGenerator * generator)
				: PointNode(p, parent, generator), sizeParam_(sizeParam) {}
			inline virtual ~MountainTopNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }

			inline double sizeParam() const { return sizeParam_; }
			inline const MountainTopNode * topNode() const { return this; }
		};
		class MountainRidgeNode : public SegmentNode
		{
			GVec3 overallDirection_;
			double sizeParam_;
			const MountainTopNode * topNode_;
		public:
			static constexpr char * ClassName = "Mountain/Ridge";

			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, double sizeParam, const GVec3 &od, const MountainTopNode * topNode, const BaseNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0,p1,parent, generator), sizeParam_(sizeParam), overallDirection_(od), topNode_(topNode) {}

			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, double sizeParam, const GVec3 &od, const MountainTopNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0, p1, parent, generator), sizeParam_(sizeParam), overallDirection_(od), topNode_(parent) {}
			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, double sizeParam, const MountainRidgeNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0, p1, parent, generator), sizeParam_(sizeParam), overallDirection_(parent->overallDirection_), topNode_(parent->topNode_) {}
			inline virtual ~MountainRidgeNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }

			inline const MountainTopNode * topNode() const { return topNode_; }
			inline const GVec3 &overallDirection() const { return overallDirection_; }

			inline double sizeParam() const { return sizeParam_; }
		};

		class MountainMetaGenerator1 : public BaseMetaGenerator
		{
			util::math::categorical_distribution<util::nameid> genDistr_, afterNullDistr_, afterTopDistr_;
		public:
			inline MountainMetaGenerator1(const MountainMetaGenerator1_Params &p)
				: genDistr_(p.general.begin(), p.general.end(), [](const auto &x) {return x.value;}, [](const auto &x) {return x.prob;}),
				afterNullDistr_(p.afterNull.begin(), p.afterNull.end(), [](const auto &x) {return x.value;}, [](const auto &x) {return x.prob;}),
				afterTopDistr_(p.afterTop.begin(), p.afterTop.end(), [](const auto &x) {return x.value;}, [](const auto &x) {return x.prob;})
			{}
			inline virtual ~MountainMetaGenerator1() {}
			inline virtual util::nameid select(const GeneratorArguments * arg)
			{
				setSeed_(arg->seedNumber);
				if (!arg->previousNode)
					return afterNullDistr_(randomEngine_);
				if (strcmp(arg->previousNode->nodeClass(), "Mountain/Top") == 0)
					return afterTopDistr_(randomEngine_);
				return genDistr_(randomEngine_);
			}
		};

		class MountainTopGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;

			struct InternalStateVar_
			{
				GVec3 pts_[MaxNodes]; //no more 16 mountain tops at once
				double szs_[MaxNodes];
				uint32_t numTops_;
			} stateVar_;

			//struct InternalStateFix_; //empty

			uint32_t numTries_;
			double minDstNode_, minDstTop_;
			
			std::uniform_real_distribution<> xyDistr_;
			std::uniform_real_distribution<>::param_type xParam_, yParam_;

			std::exponential_distribution<> hDistr_, szDistr_;
			util::math::categorical_distribution<uint32_t> numDistr_;

			inline GVec2 xyRnd_()
			{
				double x = xyDistr_(randomEngine_, xParam_);
				double y = xyDistr_(randomEngine_, yParam_);
				return GVec2(x, y);
			}
			InternalStateVar_ genState_(const GeneratorArguments * arg);
			double evaluateState_(const InternalStateVar_ &state, const GeneratorArguments * arg); //negative inappropriate, maximum is better
		public:
			MountainTopGenerator1(const MountainTopGenerator1_Params &p);
			inline virtual ~MountainTopGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const;/*returns used up space*/
		};

		class MountainTopRidgeGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;
			
			struct InternalStateFix_
			{
				GVec3 p0_;
				double prevLen_;
				double sz_;
				const MountainTopNode * topNode_;
				bool isParentRidge_;
			} stateFix_;

			struct InternalStateVar_
			{
				GVec3 p1_[MaxNodes]; //no more 16 mountain ridges at once
				uint32_t numRidges_;
			} stateVar_;


			util::math::categorical_distribution<uint32_t> numDistr_;
			double b0_, b1_;
			std::normal_distribution<> aDistr_, bDistr_;

			uint32_t numTries_;
			double minDst_;

			double len0_, len1_;
			std::lognormal_distribution<> lenDistr_;
			inline double bRnd_()
			{
				double b = bDistr_(randomEngine_);
				return glm::clamp(b, b0_, b1_);
			}
			inline double lenRnd_()
			{
				double len = lenDistr_(randomEngine_);
				return glm::clamp(len, len0_, len1_);
			}
			void setFixedState_(const GeneratorArguments * arg);
			InternalStateVar_ genState_(const GeneratorArguments * arg);
			double evaluateState_(const InternalStateVar_ &state, const GeneratorArguments * arg);
		public:
			MountainTopRidgeGenerator1(const MountainTopRidgeGenerator1_Params &p);
			inline virtual ~MountainTopRidgeGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		
		class MountainContGenerator1 : public BaseNodeGenerator
		{
			struct InternalStateFix_
			{
				static const uint32_t MaxNumPrevPts = 8;
				GVec3 p0_, od_, prevPts_[MaxNumPrevPts];
				double sz_;
				double prevLen_;
				uint32_t numPrevPts_;
				const MountainTopNode * topNode_;
			} stateFix_;

			struct InternalStateVar_
			{
				GVec3 p1_; //only one node next
			} stateVar_;

			double mix0_, mix1_, a0_, a1_, b0_, b1_;
			std::normal_distribution<> mixDistr_, aDistr_, bDistr_;
			uint32_t numTries_;
			double minDst_;
			double len0_, len1_;
			std::lognormal_distribution<> lenDistr_;
			inline double mixRnd_()
			{
				double mix = mixDistr_(randomEngine_);
				return glm::clamp(mix, mix0_, mix1_);
			}
			inline double aRnd_()
			{
				double a = aDistr_(randomEngine_);
				return glm::clamp(a, a0_, a1_);
			}
			inline double bRnd_()
			{
				double b = bDistr_(randomEngine_);
				return glm::clamp(b, b0_, b1_);
			}
			inline double lenRnd_()
			{
				double len = lenDistr_(randomEngine_);
				return glm::clamp(len, len0_, len1_);
			}
			inline GVec3 mixDirs_(const GVec3 &d1, const GVec3 &d2)
			{
				double mix = mixRnd_();
				return d1 * mix + d2 * (1.0 - mix);
			}

			void setFixedState_(const GeneratorArguments * arg);
			InternalStateVar_ genState_(const GeneratorArguments * arg);
			double evaluateState_(const InternalStateVar_ &state, const GeneratorArguments * arg);
		public:
			MountainContGenerator1(const MountainContGenerator1_Params &p);
			inline virtual ~MountainContGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		
		class MountainForkGenerator1 : public BaseNodeGenerator
		{
			struct InternalStateFix_
			{
				static const uint32_t MaxNumPrevPts = 8;
				GVec3 p0_, od_, prevPts_[MaxNumPrevPts];
				double sz_;
				double prevLen_;
				uint32_t numPrevPts_;
				const MountainTopNode * topNode_;
			} stateFix_;

			struct InternalStateVar_
			{
				GVec3 p1_, p2_; //only two nodes next
			} stateVar_;

			double mix0_, mix1_, a00_, a01_, a10_, a11_, b0_, b1_;
			std::normal_distribution<> mixDistr_, a0Distr_, a1Distr_, bDistr_;
			uint32_t numTries_;
			double minDst_;
			double len0_, len1_;
			std::lognormal_distribution<> lenDistr_;
			inline double mixRnd_()
			{
				double mix = mixDistr_(randomEngine_);
				return glm::clamp(mix, mix0_, mix1_);
			}
			inline double a0Rnd_()
			{
				double a = a0Distr_(randomEngine_);
				return glm::clamp(a, a00_, a01_);
			}
			inline double a1Rnd_()
			{
				double a = a1Distr_(randomEngine_);
				return glm::clamp(a, a10_, a11_);
			}
			inline double bRnd_()
			{
				double b = bDistr_(randomEngine_);
				return glm::clamp(b, b0_, b1_);
			}
			inline double lenRnd_()
			{
				double len = lenDistr_(randomEngine_);
				return glm::clamp(len, len0_, len1_);
			}
			inline GVec3 mixDirs_(const GVec3 &d1, const GVec3 &d2)
			{
				double mix = mixRnd_();
				return d1 * mix + d2 * (1.0 - mix);
			}

			void setFixedState_(const GeneratorArguments * arg);
			InternalStateVar_ genState_(const GeneratorArguments * arg);
			double evaluateState_(const InternalStateVar_ &state, const GeneratorArguments * arg);
		public:
			MountainForkGenerator1(const MountainForkGenerator1_Params &p);
			inline virtual ~MountainForkGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		
	}
}
