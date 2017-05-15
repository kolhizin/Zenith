#pragma once
#include "GeometryNodes.h"
#include "Params\Config.h"
#include <random>

namespace zenith
{
	namespace terragen
	{
		class MountainTopNode : public PointNode
		{
		public:
			static constexpr char * ClassName = "Mountain/Top";

			inline MountainTopNode(const GVec3 &p, const BaseNode * parent, const BaseNodeGenerator * generator) : PointNode(p, parent, generator) {}
			inline virtual ~MountainTopNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }
		};
		class MountainRidgeNode : public SegmentNode
		{
			GVec3 overallDirection_;
			const MountainTopNode * topNode_;
		public:
			static constexpr char * ClassName = "Mountain/Ridge";

			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, const GVec3 &od, const MountainTopNode * topNode, const BaseNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0,p1,parent, generator), overallDirection_(od), topNode_(topNode) {}

			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, const GVec3 &od, const MountainTopNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0, p1, parent, generator), overallDirection_(od), topNode_(parent) {}
			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, const MountainRidgeNode * parent, const BaseNodeGenerator * generator)
				: SegmentNode(p0, p1, parent, generator), overallDirection_(parent->overallDirection_), topNode_(parent->topNode_) {}
			inline virtual ~MountainRidgeNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }

			inline const MountainTopNode * topNode() const { return topNode_; }
			inline const GVec3 &overallDirection() const { return overallDirection_; }
		};


		class MountainTopGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;
			GVec3 pts_[MaxNodes]; //no more 16 mountain tops at once

			uint32_t numTries_;
			double minDstNode_, minDstTop_;
			
			std::uniform_real_distribution<> xyDistr_;
			std::uniform_real_distribution<>::param_type xParam_, yParam_;

			std::exponential_distribution<> hDistr_;
			std::piecewise_constant_distribution<> numDistr_;

			inline GVec2 xyRnd_()
			{
				double x = xyDistr_(randomEngine_, xParam_);
				double y = xyDistr_(randomEngine_, yParam_);
				return GVec2(x, y);
			}
		public:
			MountainTopGenerator1(const MountainTopGenerator1_Params &p);
			inline virtual ~MountainTopGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};

		class MountainTopRidgeGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;
			GVec3 p0_, p1_[MaxNodes]; //no more 16 mountain ridges at once
			std::piecewise_constant_distribution<> numDistr_;
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
		public:
			MountainTopRidgeGenerator1(const MountainTopRidgeGenerator1_Params &p);
			inline virtual ~MountainTopRidgeGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		
		class MountainContGenerator1 : public BaseNodeGenerator
		{
			GVec3 p0_, p1_; //only one node next
			double mix0_, mix1_, a0_, a1_;
			std::normal_distribution<> mixDistr_, aDistr_;
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
			inline double lenRnd_()
			{
				double len = lenDistr_(randomEngine_);
				return glm::clamp(len, len0_, len1_);
			}
		public:
			MountainContGenerator1(const MountainContGenerator1_Params &p);
			inline virtual ~MountainContGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		/*
		class MountainForkGenerator1 : public BaseNodeGenerator
		{
			GVec3 p0_, p1_[2]; //only two nodes next
			std::normal_distribution<> nDistr_;
			uint32_t numTries_ = 1;
		public:
			inline MountainForkGenerator1(uint32_t numTries) : numTries_(numTries) {}
			inline virtual ~MountainForkGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		//};
		
	}
}
