#include "MountainTransform.h"

using namespace zenith;
using namespace zenith::terragen;


void MtnNode2BB_(const MountainRidgeNode * node,
	glm::dvec2 &aabb_min, glm::dvec2 &aabb_max,
	glm::dvec2 &obb_p0, glm::dvec2 &obb_p1, glm::dvec2 &obb_p2, glm::dvec2 &obb_p3)
{
	const double nodeInfluenceDist = 15.0;
	GVec2 p1 = node->point0();
	GVec2 p2 = node->point1();

	GVec2 d1 = glm::normalize(p2 - p1);
	GVec2 d2 = GVec2(-d1.y, d1.x);

	obb_p0 = p1 + nodeInfluenceDist * (-0.7071*d1 -0.7071*d2);
	obb_p1 = p1 + nodeInfluenceDist * (-0.7071*d1 +0.7071*d2);
	obb_p2 = p2 + nodeInfluenceDist * (0.7071*d1 -0.7071*d2);
	obb_p3 = p2 + nodeInfluenceDist * (0.7071*d1 +0.7071*d2);

	aabb_min = glm::dvec2(glm::min(glm::min(obb_p0.x, obb_p1.x), glm::min(obb_p2.x, obb_p3.x)),
		glm::min(glm::min(obb_p0.y, obb_p1.y), glm::min(obb_p2.y, obb_p3.y)));
	aabb_max = glm::dvec2(glm::max(glm::max(obb_p0.x, obb_p1.x), glm::max(obb_p2.x, obb_p3.x)),
		glm::max(glm::max(obb_p0.y, obb_p1.y), glm::max(obb_p2.y, obb_p3.y)));
}

glm::dvec4 zenith::terragen::transformNodeToMGS(const MountainRidgeNode * node, const glm::dvec2 &p,
	 glm::dvec3 &gradient, double &rhs, double &constraint, double xh, double yh)
{
	const double nodeWidth = 1.0, nodeGradWidth=15.0, nodeInfluenceDist=15.0;
	glm::dvec4 res(0.0,0.0,0.0,0.0);

	const GVec3 &p1 = node->point0();
	const GVec3 &p2 = node->point1();

	//if (glm::min(glm::length(p - GVec2(p1)), glm::length(p - GVec2(p2))) > glm::length(GVec2(p1) - GVec2(p2)))
	//	return res;

	glm::dvec2 d = glm::normalize(GVec2(p2) - GVec2(p1));
	glm::dvec2 n = glm::dvec2(-d.y, d.x);
	glm::dvec2 v = p - glm::dvec2(p1);

	double len = glm::length(GVec2(p2)-GVec2(p1));
	double alpha = glm::dot(d, v);


	double hParallel = 0.7 * (xh * d.x * d.x + yh * d.y * d.y);
	double hOrtho = 0.7 * (xh * n.x * n.x + yh * n.y * n.y);

	double orthoDist = glm::abs(glm::dot(n,v)) - hOrtho;
	double alphaClamp = glm::clamp(alpha, 0.0, len);
	double parallelDist = glm::abs(alpha - alphaClamp) - hParallel;
	
	if (parallelDist < 0.0)
		parallelDist = 0.0;
	if (orthoDist < 0.0)
		orthoDist = 0.0;

	double dist = glm::length(glm::dvec2(parallelDist, orthoDist));

	glm::dvec2 pClosest = glm::dvec2(p1) + alphaClamp * d;
	
	gradient = glm::dvec3(glm::normalize(pClosest - p), 10.0);
	res.x = 0.0;
	if (dist < nodeGradWidth)
		res.x = (nodeGradWidth - dist)/ nodeGradWidth;
	
	if (dist > nodeWidth)
	{
		constraint = 0;
		res.z = 0;
	}
	else
	{
		constraint = p1.z * (1 - alpha / len) + p2.z * alpha/len;
		res.z = (nodeWidth - dist)/ nodeWidth;
	}

	res.y = 1.0 - res.x - res.z;
	if (res.y < 0.0)
		res.y = 0.0;

	res.w = 1.0 / (0.1 + dist);

	return normalizeWeight(res);
}


void zenith::terragen::transformNodeToMGS(const std::vector<const MountainRidgeNode *> &nodes,
	util::math::GridAccessor2D<glm::dvec4> &weights,
	util::math::GridAccessor2D<glm::dvec3> &gradients,
	util::math::GridAccessor2D<double> &rhs,
	util::math::GridAccessor2D<double> &constraint)
{
	uint32_t nx = weights.xSize(), ny = weights.ySize();
	size_t nn = nodes.size();
	double xStep = weights.xStep();
	double yStep = weights.yStep();

	for (size_t i = 0; i < nn; i++)
	{
		glm::dvec2 obb_p0, obb_p1, obb_p2, obb_p3, aabb_min, aabb_max;
		MtnNode2BB_(nodes[i], aabb_min, aabb_max, obb_p0, obb_p1, obb_p2, obb_p3);

		uint32_t x1 = static_cast<uint32_t>(glm::floor(glm::max(0.0, (aabb_min.x - xStep - weights.xMin())) / weights.xStep()));
		uint32_t x2 = static_cast<uint32_t>(glm::ceil((aabb_max.x + xStep - weights.xMin()) / weights.xStep()));
		uint32_t y1 = static_cast<uint32_t>(glm::floor(glm::max(0.0, (aabb_min.y - yStep - weights.yMin())) / weights.yStep()));
		uint32_t y2 = static_cast<uint32_t>(glm::ceil((aabb_max.y + yStep - weights.yMin()) / weights.yStep()));

		for (uint32_t yi = y1; yi <= glm::min(y2, ny-1); yi++)
			for (uint32_t xi = x1; xi <= glm::min(x2, nx-1); xi++)
			{
				glm::dvec2 curPoint(weights.getX(xi), weights.getY(yi));
				glm::dvec4 pWeights = weights.get(xi, yi), cWeights;
				glm::dvec3 pGradient = gradients.get(xi, yi), cGradient;
				double pConstraint = constraint.get(xi, yi), cConstraint, pRHS = rhs.get(xi, yi), cRHS;

				cWeights = transformNodeToMGS(nodes[i], curPoint, cGradient, cRHS, cConstraint, xStep, yStep);

				pGradient = weightedMix(pGradient, cGradient, pWeights.x * pWeights.w, cWeights.x * cWeights.w);
				pRHS = weightedMix(pRHS, cRHS, pWeights.y * pWeights.w, cWeights.y * cWeights.w);
				pConstraint = weightedMix(pConstraint, cConstraint, pWeights.z * pWeights.w, cWeights.z * cWeights.w);

				glm::dvec3 tmpW = weightedMix(glm::dvec3(pWeights), glm::dvec3(cWeights), pWeights.w, cWeights.w);

				pWeights = glm::dvec4(tmpW, pWeights.w + cWeights.w);

				gradients.get(xi, yi) = pGradient;
				rhs.get(xi, yi) = pRHS;
				constraint.get(xi, yi) = pConstraint;
				weights.get(xi, yi) = pWeights;
			}
	}
}
