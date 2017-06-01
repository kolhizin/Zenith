#include "MountainTransform.h"

using namespace zenith;
using namespace zenith::terragen;


void MtnNode2BB_(const MountainRidgeNode * node,
	glm::dvec2 &aabb_min, glm::dvec2 &aabb_max,
	glm::dvec2 &obb_p0, glm::dvec2 &obb_p1, glm::dvec2 &obb_p2, glm::dvec2 &obb_p3)
{
	const double nodeInfluenceDist = 5.0;
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
	const double nodeWidth = 1.0, nodeGradWidth=5.0, nodeInfluenceDist=5.0;
	glm::dvec4 res(0.0,0.0,0.0,0.0);

	const GVec3 &p1 = node->point0();
	const GVec3 &p2 = node->point1();

	//if (glm::min(glm::length(p - GVec2(p1)), glm::length(p - GVec2(p2))) > glm::length(GVec2(p1) - GVec2(p2)))
	//	return res;

	glm::dvec2 d1 = glm::dvec2(p2 - p1);
	glm::dvec2 d2 = p - glm::dvec2(p1);

	double len = glm::length(d1);
	double alpha = glm::dot(d1, d2);

	glm::dvec2 pOrtho = glm::dvec2(p1) + d1 * alpha / len;
	glm::dvec2 pClosest = glm::dvec2(p1) + d1 * glm::clamp(alpha, 0.0, 1.0) / len;

	glm::dvec2 dParallel = glm::normalize(d1);
	glm::dvec2 dOrtho = glm::dvec2(-dParallel.y, dParallel.x);

	double hParallel = xh * dParallel.x * dParallel.x + yh * dParallel.y * dParallel.y;
	double hOrtho = xh * dOrtho.x * dOrtho.x + yh * dOrtho.y * dOrtho.y;

	double orthoDist = glm::length(pOrtho - p) - hOrtho;
	if (orthoDist < 0.0)
		orthoDist = 0.0;

	double parallelDist = glm::abs(alpha - glm::clamp(alpha, 0.0, 1.0)) * len - hParallel;
	if (parallelDist < 0.0)
		parallelDist = 0.0;

	double dist = glm::length(pClosest - p);

	if (dist > nodeInfluenceDist)
		return res;
	
	gradient = glm::dvec3(glm::normalize(pClosest - p), 1.0);
	res.x = 0.0;
	if (dist < nodeGradWidth)
		res.x = nodeGradWidth - dist;
	
	if (parallelDist > 0.0 || orthoDist > nodeWidth)
	{
		constraint = 0;
		res.z = 0;
	}
	else
	{
		constraint = p1.z * (1 - alpha) + p2.z * alpha;
		res.z = nodeWidth - orthoDist;
	}

	res.y = 0.0;

	if (res.z > 0.0)
	{
		res.z = 1.0;
		res.x = 0.0;
	}
	
	res.w = 1.0 / (0.1 + orthoDist + parallelDist);

	//if (orthoDist < 0.0 || parallelDist < 0.0)
	//	std::cout << "DistError";
	//if (res.x < 0.0 || res.y < 0.0 || res.z < 0.0)
	//	std::cout << "WeightError";

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
		MtnNode2BB_(nodes[i], obb_p0, obb_p1, obb_p2, obb_p3, aabb_min, aabb_max);

		uint32_t x1 = static_cast<uint32_t>(glm::floor(glm::max(0.0,(aabb_min.x - weights.xMin())) / weights.xStep()));
		uint32_t x2 = static_cast<uint32_t>(glm::ceil((aabb_max.x - weights.xMin()) / weights.xStep()));
		uint32_t y1 = static_cast<uint32_t>(glm::floor(glm::max(0.0, (aabb_min.y - weights.yMin())) / weights.yStep()));
		uint32_t y2 = static_cast<uint32_t>(glm::ceil((aabb_max.y - weights.yMin()) / weights.yStep()));

		for (uint32_t yi = y1; yi <= glm::min(y2, ny-1); yi++)
			for (uint32_t xi = x1; xi <= glm::min(x2, nx-1); xi++)
			{
				glm::dvec2 curPoint(weights.getX(xi), weights.getY(yi));
				glm::dvec4 pWeights = weights.get(xi, yi), cWeights;
				glm::dvec3 pGradient = gradients.get(xi, yi), cGradient;
				double pConstraint = constraint.get(xi, yi), cConstraint, pRHS = rhs.get(xi, yi), cRHS;

				cWeights = transformNodeToMGS(nodes[i], curPoint, cGradient, cRHS, cConstraint, xStep, yStep);

				if (cWeights.x > 0.0)
					std::cout << "Very strange;\n";

				pGradient = weightedMix(pGradient, cGradient, pWeights.x * pWeights.w, cWeights.x * cWeights.w);
				pRHS = weightedMix(pRHS, cRHS, pWeights.y * pWeights.w, cWeights.y * cWeights.w);
				pConstraint = weightedMix(pConstraint, cConstraint, pWeights.z * pWeights.w, cWeights.z * cWeights.w);

				glm::dvec3 tmpW = weightedMix(glm::dvec3(pWeights), glm::dvec3(cWeights), pWeights.w, cWeights.w);

				/*
				if (tmpW.x > 1.0 || tmpW.y > 1.0 || tmpW.z > 1.0 || tmpW.x < 0.0 || tmpW.y < 0.0 || tmpW.z < 0.0)
					std::cout << "WeightValueError";
				if (tmpW.x + tmpW.y + tmpW.z > 1.0 + 1e-7)
					std::cout << "WeightNormError";
				if ((pWeights.w + cWeights.w) > 1e-7 && tmpW.x + tmpW.y + tmpW.z < 1.0 - 1e-7)
					std::cout << "WeightNormError";
				*/
				pWeights = glm::dvec4(tmpW, pWeights.w + cWeights.w);

				gradients.get(xi, yi) = pGradient;
				rhs.get(xi, yi) = pRHS;
				constraint.get(xi, yi) = pConstraint;
				weights.get(xi, yi) = pWeights;
			}
	}


	/* //grid-centric solution -- 29 sec without early out, 21 sec with early out
	for (uint32_t yi = 0; yi < ny; yi++)
		for (uint32_t xi = 0; xi < nx; xi++)
		{
			glm::dvec2 curPoint(weights.getX(xi), weights.getY(yi));
			glm::dvec4 pWeights(0.0,0.0,0.0,0.0), cWeights;
			glm::dvec3 pGradient(0.0,0.0,0.0), cGradient;
			double pConstraint = 0.0, cConstraint, pRHS = 0.0, cRHS;
			for (size_t i = 0; i < nn; i++)
			{
				cWeights = transformNodeToMGS(nodes[i], curPoint, cGradient, cRHS, cConstraint);

				pGradient = weightedMix(pGradient, cGradient, pWeights.x * pWeights.w, cWeights.x * cWeights.w);
				pRHS = weightedMix(pRHS, cRHS, pWeights.y * pWeights.w, cWeights.y * cWeights.w);
				pConstraint = weightedMix(pConstraint, cConstraint, pWeights.z * pWeights.w, cWeights.z * cWeights.w);

				glm::dvec3 tmpW = weightedMix(glm::dvec3(pWeights), glm::dvec3(cWeights), pWeights.w, cWeights.w);
				pWeights = glm::dvec4(tmpW, pWeights.w + cWeights.w);
			}
			glm::dvec4 gWeights = weights.get(xi, yi);
			cGradient = weightedMix(pGradient, gradients.get(xi, yi), pWeights.x * pWeights.w, gWeights.x * gWeights.w);
			cRHS = weightedMix(pRHS, rhs.get(xi, yi), pWeights.y * pWeights.w, gWeights.y * gWeights.w);
			cConstraint = weightedMix(pConstraint, constraint.get(xi, yi), pWeights.z * pWeights.w, gWeights.z * gWeights.w);
			
			glm::dvec3 tmpW = weightedMix(glm::dvec3(pWeights), glm::dvec3(gWeights), pWeights.w, gWeights.w);
			cWeights = glm::dvec4(tmpW, pWeights.w + gWeights.w);
			
			gradients.get(xi, yi) = cGradient;
			rhs.get(xi, yi) = cRHS;
			constraint.get(xi, yi) = cConstraint;
			weights.get(xi, yi) = cWeights;
		}		
	*/
}
