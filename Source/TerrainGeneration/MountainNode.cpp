#include "MountainNode.h"
#include "NodeDispatcher.h"

using namespace zenith::terragen;

uint32_t zenith::terragen::MountainTopGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);

	uint32_t numG = pDistr_(randomEngine_);
	if (numG > MaxNodes)
		numG = MaxNodes;

	setState_(numG, arg->previousNode);
	
	for (uint32_t i = 0; i < numG; i++)
	{
		double x = nDistr_(randomEngine_);
		double y = nDistr_(randomEngine_);
		double h = nDistr_(randomEngine_, std::normal_distribution<>::param_type(100.0, 30.0));
		pts_[i] = GVec3(x, y, h);
	}

	return numG;
}

size_t zenith::terragen::MountainTopGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainTopNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainTopNode(pts_[nodeId], curParent_, this);
	return szReq;
}
