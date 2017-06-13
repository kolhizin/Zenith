#include "NodeTransform.h"
#include "MountainTransform.h"

using namespace zenith;
using namespace zenith::terragen;

template<class T>
void transformNodeToMGS_(const std::vector<const BaseNode *> &nodes, std::vector<bool> &fTransformed,
	util::math::GridAccessor2D<glm::dvec4> &weights,
	util::math::GridAccessor2D<glm::dvec3> &gradients,
	util::math::GridAccessor2D<double> &rhs,
	util::math::GridAccessor2D<double> &constraint)
{
	std::vector<const T *> tNodes;
	for (size_t i = 0; i < nodes.size(); i++)
	{
		if (fTransformed[i])
			continue;
		if (dynamic_cast<const T *>(nodes[i]))
		{
			tNodes.push_back(static_cast<const T *>(nodes[i]));
			fTransformed[i] = true;
		}
	}
	transformNodeToMGS(tNodes, weights, gradients, rhs, constraint);
}

void zenith::terragen::transformNodeToMGS(const std::vector<const BaseNode *> &nodes,
	util::math::GridAccessor2D<glm::dvec4> &weights,
	util::math::GridAccessor2D<glm::dvec3> &gradients,
	util::math::GridAccessor2D<double> &rhs,
	util::math::GridAccessor2D<double> &constraint)
{
	std::vector<bool> fTransformed(nodes.size(), false);

	util::math::fillGrid(weights, glm::dvec4(0.0, 0.0, 0.0, 0.0));

	transformNodeToMGS_<MountainRidgeNode>(nodes, fTransformed, weights, gradients, rhs, constraint);
}