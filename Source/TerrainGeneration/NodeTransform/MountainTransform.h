#pragma once
#include "../MountainNode.h"
#include "NodeTransform.h"

namespace zenith
{
	namespace terragen
	{
		//MGS -- multigrid solver

		//transforms base node into gradient with .w component denoting influence, 0 - no influence, \infty - maximal influence
		glm::dvec4 transformNodeToMGS(const MountainRidgeNode * node, const glm::dvec2 &p,
			glm::dvec3 &gradient, double &rhs, double &constraint, double xh = 0.0, double yh = 0.0);

		void transformNodeToMGS(const std::vector<const MountainRidgeNode *> &nodes,
			util::math::GridAccessor2D<glm::dvec4> &weights,
			util::math::GridAccessor2D<glm::dvec3> &gradients,
			util::math::GridAccessor2D<double> &rhs,
			util::math::GridAccessor2D<double> &constraint);
	}
}