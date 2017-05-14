#pragma once
#include "GeometryNodes.h"
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
		public:
			static constexpr char * ClassName = "Mountain/Ridge";

			inline MountainRidgeNode(const GVec3 &p0, const GVec3 &p1, const BaseNode * parent, const BaseNodeGenerator * generator) : SegmentNode(p0,p1,parent, generator) {}
			inline virtual ~MountainRidgeNode() {}

			inline virtual const char * nodeClass() const { return ClassName; }
		};

		class MountainTopGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;
			GVec3 pts_[MaxNodes]; //no more 16 mountain tops at once
			std::poisson_distribution<> pDistr_;
			std::normal_distribution<> nDistr_;
		public:
			inline MountainTopGenerator1() {}
			inline virtual ~MountainTopGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};

		class MountainTopRidgeGenerator1 : public BaseNodeGenerator
		{
			static const uint32_t MaxNodes = 16;
			GVec3 p0_, p1_[MaxNodes]; //no more 16 mountain ridges at once
			std::poisson_distribution<> pDistr_;
			std::normal_distribution<> nDistr_;
			std::lognormal_distribution<> lnDistr_;
		public:
			inline MountainTopRidgeGenerator1() : pDistr_(4.0) {}
			inline virtual ~MountainTopRidgeGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
		class MountainContGenerator1 : public BaseNodeGenerator
		{
			GVec3 p0_, p1_; //only one node next
			std::normal_distribution<> nDistr_;
			uint32_t numTries_ = 1;
		public:
			inline MountainContGenerator1(uint32_t numTries) : numTries_(numTries){}
			inline virtual ~MountainContGenerator1() {}

			virtual uint32_t generate(const GeneratorArguments * arg);
			virtual size_t get(uint32_t nodeId, void * buffPtr, size_t buffSize) const; /*returns used up space*/
		};
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
		};
	}
}
