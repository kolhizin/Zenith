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

	}
}
