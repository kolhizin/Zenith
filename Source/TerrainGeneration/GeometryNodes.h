#pragma once
#include "NodeGenerator.h"
#include <glm\glm.hpp>

namespace zenith
{
	namespace terragen
	{
		typedef glm::dvec3 GVec3;
		typedef glm::dvec2 GVec2;

		typedef glm::dmat3x3 GMat3x3;

		inline GVec3 rotateDirXY(const GVec3 &v, double a)
		{
			/*
			x' = x * cos(a) - y * sin(a)
			y' = x * sin(a) + y * cos(a)
			z' = z
			*/
			double ca = glm::cos(a), sa = glm::sin(a);
			return GVec3(v.x * ca - v.y * sa, v.x * sa + v.y * ca, v.z);
		}
		inline GVec3 rotateDirZH(const GVec3 &v, double a, const GVec2 &dd = GVec2(1.0, 0.0))
		{
			/*
			h' = h * cos(a) - z * sin(a)
			z' = h * sin(a) + z * cos(a)

			x' = h' * x / h = x * cos(a) - z * x * sin(a) / h
			y' = h' * y / h = y * cos(a) - z * y * sin(a) / h
			z' = h * sin(a) + z * cos(a)
			*/
			double h = glm::length(GVec2(v));
			double ca = glm::cos(a), sa = glm::sin(a);
			if (isZero(h))
				return GVec3(-v.z * sa * dd, v.z * ca);

			double xy = ca - sa * v.z / h;
			return GVec3(v.x * xy, v.y * xy, h * sa + v.z * ca);
		}

		inline GVec3 rotateDir(const GVec3 &v, double xyA, double zA, const GVec2 &dd = GVec2(1.0, 0.0))
		{
			return rotateDirZH(rotateDirXY(v, xyA), zA, dd);
		}
		
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
