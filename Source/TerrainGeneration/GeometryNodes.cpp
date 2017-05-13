#include "GeometryNodes.h"

using namespace zenith::terragen;


double getPointSegmentDistance(const GVec2 &p, const GVec2 &s0, const GVec2 &s1)
{
	auto ds = s1 - s0;
	typename GVec2::value_type sl = glm::length(ds);
	auto dsn = ds / sl;
	auto dp = p - s0;

	double alpha = glm::dot(dp, dsn);
	double alpha_clamp = glm::max(0.0, glm::min(sl, alpha));
	auto d = s0 + alpha_clamp * dsn - p;
	return glm::length(d);
}

bool getLineIntersection(const GVec2 &p0, const GVec2 &dp, const GVec2 &q0, const GVec2 &dq, GVec2 &g)
{
	double pq = dp.x * dq.y - dp.y * dq.x;
	if (isZero(pq))
		return false;

	double pc = dp.x * p0.y - dp.y * p0.x;
	double qc = dq.x * q0.y - dq.y * q0.x;

	double gx = (dq.x*pc - dp.x*qc) / pq;
	double gy = (dq.y*pc - dp.y*qc) / pq;
	g = GVec2(gx, gy);
	return true;
}

bool getSegmentIntersection(const GVec2 &p0, const GVec2 &p1, const GVec2 &q0, const GVec2 &q1, GVec2 &g)
{
	bool lineInter = getLineIntersection(p0, p1 - p0, q0, q1 - q0, g);
	if (!lineInter)
		return false;

	if (g.x < glm::min(p0.x, p1.x))
		return false;
	if (g.x > glm::max(p0.x, p1.x))
		return false;
	if (g.y < glm::min(p0.y, p1.y))
		return false;
	if (g.y > glm::max(p0.y, p1.y))
		return false;

	if (g.x < glm::min(q0.x, q1.x))
		return false;
	if (g.x > glm::max(q0.x, q1.x))
		return false;
	if (g.y < glm::min(q0.y, q1.y))
		return false;
	if (g.y > glm::max(q0.y, q1.y))
		return false;

	return true;
}

double getSegmentSegmentDistance(const GVec2 &p0, const GVec2 &p1, const GVec2 &q0, const GVec2 &q1)
{
	GVec2 g;
	if (getSegmentIntersection(p0, p1, q0, q1, g))
		return 0.0;

	double d1 = getPointSegmentDistance(p0, q0, q1);
	double d2 = getPointSegmentDistance(p1, q0, q1);
	double d3 = getPointSegmentDistance(q0, p0, p1);
	double d4 = getPointSegmentDistance(q1, p0, p1);
	return glm::min(glm::min(d1, d2), glm::min(d3, d4));
}

double getFastLowPointSegmentDistance(const GVec2 &p, const GVec2 &s0, const GVec2 &s1)
{
	double d1 = glm::length(p - s0);
	double d2 = glm::length(p - s1);
	double d = glm::length(s1 - s0);
	return glm::min(d1, d2) - d;
}
double getFastLowSegmentSegmentDistance(const GVec2 &p0, const GVec2 &p1, const GVec2 &s0, const GVec2 &s1)
{
	double d1 = glm::length(p0 - s0);
	double d2 = glm::length(p0 - s1);
	double d3 = glm::length(p1 - s0);
	double d4 = glm::length(p1 - s1);
	double d = glm::length(s1 - s0) + glm::length(p1-p0);
	return glm::min(glm::min(d1, d2), glm::min(d3, d4)) - d;
}

double zenith::terragen::getNodeDistance(const PointNode * n1, const PointNode * n2)
{
	return glm::length(GVec2(n1->point()) - GVec2(n2->point()));
}

double zenith::terragen::getNodeDistance(const SegmentNode * n1, const PointNode * n2)
{
	return getPointSegmentDistance(GVec2(n2->point()), GVec2(n1->point0()), GVec2(n1->point1()));
}

double zenith::terragen::getNodeDistance(const PointNode * n1, const SegmentNode * n2)
{
	return getPointSegmentDistance(GVec2(n1->point()), GVec2(n2->point0()), GVec2(n2->point1()));
}

double zenith::terragen::getNodeDistance(const SegmentNode * n1, const SegmentNode * n2)
{
	return getSegmentSegmentDistance(GVec2(n1->point0()), GVec2(n1->point1()), GVec2(n2->point0()), GVec2(n2->point1()));
}

double zenith::terragen::getNodeFastLowerDistance(const PointNode * n1, const PointNode * n2)
{
	return glm::length(GVec2(n1->point()) - GVec2(n2->point()));
}

double zenith::terragen::getNodeFastLowerDistance(const SegmentNode * n1, const PointNode * n2)
{
	return getPointSegmentDistance(GVec2(n2->point()), GVec2(n1->point0()), GVec2(n1->point1()));
}

double zenith::terragen::getNodeFastLowerDistance(const PointNode * n1, const SegmentNode * n2)
{
	return getPointSegmentDistance(GVec2(n1->point()), GVec2(n2->point0()), GVec2(n2->point1()));return 0.0;
}

double zenith::terragen::getNodeFastLowerDistance(const SegmentNode * n1, const SegmentNode * n2)
{
	return getFastLowSegmentSegmentDistance(GVec2(n1->point0()), GVec2(n1->point1()), GVec2(n2->point0()), GVec2(n2->point1()));
}
