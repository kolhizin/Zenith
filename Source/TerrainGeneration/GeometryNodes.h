#pragma once
#include "NodeGenerator.h"
#include <glm\glm.hpp>

namespace zenith
{
	namespace terragen
	{
		typedef glm::dvec3 GVec3;
		typedef glm::dvec2 GVec2;

		class PointNode : public BaseNode
		{
			GVec3 p_;
		public:
			static constexpr char * ClassName = "Geometry/PointNode";
			static constexpr char * DistanceDispatchGroup = "Geometry/PointNode";

			inline PointNode(const GVec3 &p, const BaseNode * parent, const BaseNodeGenerator * generator) : BaseNode(parent, generator), p_(p) {}
			inline virtual ~PointNode() {}
			inline const GVec3 &point() const { return p_; }

			inline virtual const char * nodeClass() const { return ClassName; }
			inline virtual const char * distanceDispatchGroup() const { return DistanceDispatchGroup; }
		};
		class SegmentNode : public BaseNode
		{
			GVec3 p0_, p1_;
		public:
			static constexpr char * ClassName = "Geometry/SegmentNode";
			static constexpr char * DistanceDispatchGroup = "Geometry/SegmentNode";

			inline SegmentNode(const GVec3 &p0, const GVec3 &p1, const BaseNode * parent, const BaseNodeGenerator * generator) : BaseNode(parent, generator),p0_(p0),p1_(p1) {}
			inline virtual ~SegmentNode() {}

			inline const GVec3 &point0() const { return p0_; }
			inline const GVec3 &point1() const { return p1_; }

			inline virtual const char * nodeClass() const { return ClassName; }
			inline virtual const char * distanceDispatchGroup() const { return DistanceDispatchGroup; }
		};

		double getNodeDistance(const PointNode * n1, const PointNode * n2);
		double getNodeDistance(const SegmentNode * n1, const PointNode * n2);
		double getNodeDistance(const PointNode * n1, const SegmentNode * n2);
		double getNodeDistance(const SegmentNode * n1, const SegmentNode * n2);


		double getNodeFastLowerDistance(const PointNode * n1, const PointNode * n2);
		double getNodeFastLowerDistance(const SegmentNode * n1, const PointNode * n2);
		double getNodeFastLowerDistance(const PointNode * n1, const SegmentNode * n2);
		double getNodeFastLowerDistance(const SegmentNode * n1, const SegmentNode * n2);
	}
}
