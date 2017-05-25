#include "MountainNode.h"
#include "NodeDispatcher.h"
#include <glm\gtc\constants.hpp>

using namespace zenith::terragen;



MountainTopGenerator1::MountainTopGenerator1(const MountainTopGenerator1_Params & p)
	: hDistr_(1.0 / p.heightLambda), szDistr_(1.0 / p.sizeLambda),
	numTries_(p.numTries), minDstNode_(p.minNodeDistance), minDstTop_(p.minMountainTopDistance),
	xParam_(p.boundingBox.x0, p.boundingBox.x1), yParam_(p.boundingBox.y0, p.boundingBox.y1),
	numDistr_(p.numTops.begin(), p.numTops.end(), [](const auto &x) {return x.value;}, [](const auto &x) {return x.prob;})
{
}

MountainTopRidgeGenerator1::MountainTopRidgeGenerator1(const MountainTopRidgeGenerator1_Params & p)
	: aDistr_(p.alpha.mu, p.alpha.sigma), bDistr_(p.beta.mu, p.beta.sigma), b0_(p.betaBox.x0), b1_(p.betaBox.x1),
	lenDistr_(p.len.mu, p.len.sigma), len0_(p.lenBox.x0), len1_(p.lenBox.x1), numTries_(p.numTries), minDst_(p.minNodeDistance),
	numDistr_(p.numRidges.begin(), p.numRidges.end(), [](const auto &x) {return x.value;}, [](const auto &x) {return x.prob;})
{
}


MountainContGenerator1::MountainContGenerator1(const MountainContGenerator1_Params & p)
	: aDistr_(p.alpha.mu, p.alpha.sigma), a0_(p.alphaBox.x0), a1_(p.alphaBox.x1),
	bDistr_(p.beta.mu, p.beta.sigma), b0_(p.betaBox.x0), b1_(p.betaBox.x1),
	mixDistr_(p.mix.mu, p.mix.sigma), mix0_(p.mixBox.x0), mix1_(p.mixBox.x1),
	lenDistr_(p.len.mu, p.len.sigma), len0_(p.lenBox.x0), len1_(p.lenBox.x1),
	numTries_(p.numTries), minDst_(p.minNodeDistance)
{
}


MountainTopGenerator1::InternalStateVar_ zenith::terragen::MountainTopGenerator1::genState_(const GeneratorArguments * arg)
{
	InternalStateVar_ res;

	uint32_t numG = numDistr_(randomEngine_);
	if (numG > MaxNodes)
		numG = MaxNodes;

	res.numTops_ = numG;

	for (uint32_t i = 0; i < numG; i++)
	{
		double x = xyDistr_(randomEngine_, xParam_);
		double y = xyDistr_(randomEngine_, yParam_);
		double h = hDistr_(randomEngine_);
		double s = szDistr_(randomEngine_);
		res.pts_[i] = GVec3(x, y, h);
		res.szs_[i] = s;
	}

	return res;
}

double zenith::terragen::MountainTopGenerator1::evaluateState_(const InternalStateVar_ & state, const GeneratorArguments * arg)
{
	double minDistance = TerraGenInfty, minTopDistance = TerraGenInfty;
	
	for (uint32_t i = 0; i < state.numTops_; i++)
	{
		MountainTopNode m(state.pts_[i], state.szs_[i], arg->previousNode, this);
		for (uint32_t j = i+1; j < state.numTops_; j++)
		{
			double td = glm::length(GVec2(state.pts_[i]) - GVec2(state.pts_[j]));
			if (td < minTopDistance)
				minTopDistance = td;
		}

		for (uint32_t j = 0; j < arg->allNodesNum; j++)
		{
			const BaseNode * ptr = arg->allNodesPtr[j];
			if (ptr == arg->previousNode)
				continue;
			double dst = getNodeDistance(&m, ptr, 50.0);
			if (dynamic_cast<const MountainTopNode *>(ptr))
			{
				if (dst < minTopDistance)
					minTopDistance = dst;
			}
			else
			{
				if (dst < minDistance)
					minDistance = dst;
			}
		}
	}
	if (minDistance < minDstNode_)
		return -1.0;
	if (minTopDistance < minDstTop_)
		return -1.0;

	return minDistance;
}

uint32_t zenith::terragen::MountainTopGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);

	InternalStateVar_ bestState;
	double bestVal = -1.0;

	for (uint32_t i = 0; i < numTries_; i++)
	{
		InternalStateVar_ curState = genState_(arg);
		double val = evaluateState_(curState, arg);
		if (val > bestVal)
		{
			bestVal = val;
			bestState = curState;
		}
	}

	if (bestVal > 0.0)
	{
		setState_(bestState.numTops_, arg->previousNode);
		stateVar_ = bestState;
	}
	else setState_(0, arg->previousNode);

	return numGenerated_;
}

size_t zenith::terragen::MountainTopGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainTopNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainTopNode(stateVar_.pts_[nodeId], stateVar_.szs_[nodeId], curParent_, this);
	return szReq;
}



void zenith::terragen::MountainTopRidgeGenerator1::setFixedState_(const GeneratorArguments * arg)
{
	const BaseNode * parent = arg->previousNode;
	if (!parent)
		throw TerraGenGeneratorException("MountainTopRidgeGenerator1::generate(): parent node expected to be non-null!");

	stateFix_.isParentRidge_ = false;
	if (dynamic_cast<const MountainTopNode *>(parent))
	{
		stateFix_.p0_ = static_cast<const MountainTopNode *>(parent)->point();
		stateFix_.sz_ = static_cast<const MountainTopNode *>(parent)->sizeParam();
		stateFix_.topNode_ = static_cast<const MountainTopNode *>(parent)->topNode();

		stateFix_.prevLen_ = -1.0;
	}
	else if (dynamic_cast<const MountainRidgeNode *>(parent))
	{
		stateFix_.isParentRidge_ = true;
		stateFix_.p0_ = static_cast<const MountainRidgeNode *>(parent)->point1();
		stateFix_.sz_ = static_cast<const MountainRidgeNode *>(parent)->sizeParam();
		stateFix_.topNode_ = static_cast<const MountainRidgeNode *>(parent)->topNode();

		stateFix_.prevLen_ = glm::length(GVec2(static_cast<const MountainRidgeNode *>(parent)->point1() - static_cast<const MountainRidgeNode *>(parent)->point0()));
	}
	else throw TerraGenGeneratorException("MountainTopRidgeGenerator1::generate(): parent node expected to be one of following types: MountainTop, MountainRidge!");

}

MountainTopRidgeGenerator1::InternalStateVar_ zenith::terragen::MountainTopRidgeGenerator1::genState_(const GeneratorArguments * arg)
{
	InternalStateVar_ res;

	res.numRidges_ = numDistr_(randomEngine_);
	if (res.numRidges_ > MaxNodes)
		res.numRidges_ = MaxNodes;

	double step = glm::two_pi<double>() / double(res.numRidges_);

	for (uint32_t i = 0; i < res.numRidges_; i++)
	{
		double a = double(i) * step + step * aDistr_(randomEngine_);
		double b = bRnd_();

		double dx = glm::cos(a) * glm::cos(b);
		double dy = glm::sin(a) * glm::cos(b);
		double dh = glm::sin(b);

		double dl = stateFix_.sz_ * lenRnd_();

		res.p1_[i] = stateFix_.p0_ + GVec3(dx*dl, dy*dl, dh*dl);
	}
	return res;
}

double zenith::terragen::MountainTopRidgeGenerator1::evaluateState_(const InternalStateVar_ & state, const GeneratorArguments * arg)
{
	double minDistance = TerraGenInfty;
	for (uint32_t i = 0; i < state.numRidges_; i++)
	{
		GVec2 curRP = glm::normalize(GVec2(state.p1_[i] - stateFix_.p0_));

		for (uint32_t j = i + 1; j < state.numRidges_; j++)
		{
			GVec2 testRP = glm::normalize(GVec2(state.p1_[j] - stateFix_.p0_));
			double dotp = glm::dot(curRP, testRP);
			if (dotp > 0.8)
				return -1.0;
		}
		MountainRidgeNode m(stateFix_.p0_, state.p1_[i], stateFix_.sz_, state.p1_[i] - stateFix_.p0_, stateFix_.topNode_, arg->previousNode, this);
		double nl = glm::length(GVec2(state.p1_[i] - stateFix_.p0_));
		if (stateFix_.prevLen_ > 0.0)
			nl = glm::min(nl, stateFix_.prevLen_);
		for (uint32_t j = 0; j < arg->allNodesNum; j++)
		{
			const BaseNode * ptr = arg->allNodesPtr[j];
			if (ptr == arg->previousNode)
				continue;
			double dst = getNodeDistance(&m, ptr, 50.0) / nl;
			if (dst < minDistance)
				minDistance = dst;
		}
		if (minDistance < minDst_)
			return -1.0;
	}
	return minDistance;
}


uint32_t zenith::terragen::MountainTopRidgeGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);

	setFixedState_(arg);

	InternalStateVar_ bestState;
	double bestVal = -1.0;
	
	for (uint32_t i = 0; i < numTries_; i++)
	{
		InternalStateVar_ curState = genState_(arg);
		double val = evaluateState_(curState, arg);
		if (val > bestVal)
		{
			bestVal = val;
			bestState = curState;
		}
	}

	if (bestVal > 0.0)
	{
		setState_(bestState.numRidges_, arg->previousNode);
		stateVar_ = bestState;
	}
	else setState_(0, arg->previousNode);
	
	return numGenerated_;
}

size_t zenith::terragen::MountainTopRidgeGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainRidgeNode(stateFix_.p0_, stateVar_.p1_[nodeId], stateFix_.sz_, stateVar_.p1_[nodeId]- stateFix_.p0_, stateFix_.topNode_, curParent_, this);
	return szReq;
}


void zenith::terragen::MountainContGenerator1::setFixedState_(const GeneratorArguments * arg)
{
	stateFix_.numPrevPts_ = 1;

	if (dynamic_cast<const MountainRidgeNode *>(arg->previousNode))
	{
		const MountainRidgeNode * pNode = static_cast<const MountainRidgeNode *>(arg->previousNode);
		stateFix_.prevPts_[0] = pNode->point0();
		stateFix_.p0_ = pNode->point1();
		stateFix_.od_ = pNode->overallDirection();
		stateFix_.sz_ = pNode->sizeParam();
		stateFix_.prevLen_ = glm::length(GVec2(pNode->point1()- pNode->point0()));

		while (stateFix_.numPrevPts_ < InternalStateFix_::MaxNumPrevPts && dynamic_cast<const MountainRidgeNode *>(pNode->getParent()))
		{
			pNode = static_cast<const MountainRidgeNode *>(pNode->getParent());
			stateFix_.prevPts_[stateFix_.numPrevPts_] = pNode->point0();
			stateFix_.numPrevPts_++;
		}
	}
	else throw TerraGenGeneratorException("MountainContGenerator1::generate(): parent node expected to be one of following types: MountainRidge!");
}

MountainContGenerator1::InternalStateVar_ zenith::terragen::MountainContGenerator1::genState_(const GeneratorArguments * arg)
{
	InternalStateVar_ res;

	GVec3 prevDir = stateFix_.p0_ - stateFix_.prevPts_[0];
	GVec3 dir = glm::normalize(mixDirs_(stateFix_.od_, prevDir));

	double xyA = aRnd_();
	double zhA = bRnd_();
	GVec3 newDir = rotateDir(dir, xyA, zhA, stateFix_.od_);

	double l = stateFix_.sz_ * lenRnd_();

	res.p1_ = stateFix_.p0_ + l * newDir;

	return res;
}

double zenith::terragen::MountainContGenerator1::evaluateState_(const InternalStateVar_ & state, const GeneratorArguments * arg)
{
	MountainRidgeNode m(stateFix_.p0_, state.p1_, stateFix_.sz_, stateFix_.od_, stateFix_.topNode_, arg->previousNode, this);
	double nl = glm::length(GVec2(state.p1_ - stateFix_.p0_));
	if (stateFix_.prevLen_ > 0.0)
		nl = glm::min(nl, stateFix_.prevLen_);
	double minDistance = TerraGenInfty;
	for (uint32_t j = 0; j < arg->allNodesNum; j++)
	{
		const BaseNode * ptr = arg->allNodesPtr[j];
		if (ptr == arg->previousNode)
			continue;
		double dst = getNodeDistance(&m, ptr, 50.0) / nl;
		if (dst < minDistance)
			minDistance = dst;
	}
	if (minDistance < minDst_)
		return -1.0;
	return minDistance;
}


uint32_t zenith::terragen::MountainContGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);
	setFixedState_(arg);

	InternalStateVar_ bestState;
	double bestVal = -1.0;

	for (uint32_t i = 0; i < numTries_; i++)
	{
		InternalStateVar_ curState = genState_(arg);
		double val = evaluateState_(curState, arg);
		if (val > bestVal)
		{
			bestVal = val;
			bestState = curState;
		}
	}

	if (bestVal > 0.0)
	{
		setState_(1, arg->previousNode);
		stateVar_ = bestState;
	}
	else setState_(0, arg->previousNode);

	return numGenerated_;
}

size_t zenith::terragen::MountainContGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainRidgeNode(stateFix_.p0_, stateVar_.p1_, stateFix_.sz_, stateFix_.od_, stateFix_.topNode_, curParent_, this);
	return szReq;
}
/*
uint32_t zenith::terragen::MountainForkGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);

	uint32_t numG = 2;

	const BaseNode * parent = arg->previousNode;
	if (!parent)
		throw TerraGenGeneratorException("MountainForkGenerator1::generate(): parent node expected to be non-null!");

	GVec3 prevPoints[8];
	uint8_t numPrevPoints = 1;

	if (dynamic_cast<const MountainRidgeNode *>(parent))
	{
		prevPoints[0] = static_cast<const MountainRidgeNode *>(parent)->point0();
		p0_ = static_cast<const MountainRidgeNode *>(parent)->point1();
		const MountainRidgeNode * pNode = static_cast<const MountainRidgeNode *>(parent);
		while (numPrevPoints < 8 && dynamic_cast<const MountainRidgeNode *>(pNode->getParent()))
		{
			pNode = static_cast<const MountainRidgeNode *>(pNode->getParent());
			prevPoints[numPrevPoints] = pNode->point0();
			numPrevPoints++;
		}
	}
	else throw TerraGenGeneratorException("MountainForkGenerator1::generate(): parent node expected to be one of following types: MountainRidge!");

	GVec2 mainDir = p0_ - prevPoints[0];
	double mainDirL = glm::length(mainDir);
	mainDir /= GVec2::value_type(mainDirL);
	double bestDist = 0;
	double sigmaL = mainDirL * 0.3;
	double sigmaD = 0.2;
	double mainDirDH = std::min<double>(0.0, (p0_.z - prevPoints[0].z)) * 0.8;

	for (uint32_t i = 0; i < numTries_; i++)
	{
		double a0 = nDistr_(randomEngine_, std::normal_distribution<>::param_type(glm::pi<double>() * 0.3, glm::pi<double>() * 0.1));
		a0 = glm::clamp(a0, glm::pi<double>() * 0.05, glm::pi<double>() * 0.4);
		double a1 = nDistr_(randomEngine_, std::normal_distribution<>::param_type(a0, a0 * 0.2));
		double a2 = nDistr_(randomEngine_, std::normal_distribution<>::param_type(-a0, a0 * 0.2));
		a1 = glm::clamp(a1, glm::pi<double>() * 0.01, glm::pi<double>() * 0.5);
		a2 = glm::clamp(a2, -glm::pi<double>() * 0.5, -glm::pi<double>() * 0.01);

		glm::dmat2x2 r1(glm::cos(a1), -glm::sin(a1), glm::sin(a1), glm::cos(a1));
		glm::dmat2x2 r2(glm::cos(a2), -glm::sin(a2), glm::sin(a2), glm::cos(a2));

		GVec2 d1 = r1 * mainDir;
		GVec2 d2 = r2 * mainDir;

		double dl1 = glm::max<double>(0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirL, sigmaL)));
		double dl2 = glm::max<double>(0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirL, sigmaL)));
		double dlm = glm::max(dl1, dl2);
		double dh1 = glm::min<double>(-0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirDH, 1.0)));
		double dh2 = glm::min<double>(-0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirDH, 1.0)));

		GVec3 p1 = p0_ + GVec3(d1.x * dl1, d1.y * dl1, dh1);
		GVec3 p2 = p0_ + GVec3(d2.x * dl2, d2.y * dl2, dh2);
		MountainRidgeNode m1(p0_, p1, parent, this);
		MountainRidgeNode m2(p0_, p2, parent, this);

		double minDistance = TerraGenInfty;
		for (uint32_t j = 0; j < arg->allNodesNum; j++)
		{
			const BaseNode * ptr = arg->allNodesPtr[j];
			if (ptr == parent)
				continue;
			double dst = getNodeDistance(&m1, ptr, 50.0);
			if (dst < minDistance)
				minDistance = dst;
			dst = getNodeDistance(&m2, ptr, 50.0);
			if (dst < minDistance)
				minDistance = dst;
		}
		if (minDistance < 0.5 * glm::max(1.0, dlm))
			continue;
		if (minDistance > bestDist)
		{
			bestDist = minDistance;
			p1_[0] = p1;
			p1_[1] = p2;
		}
	}
	if (isZero(bestDist))
		setState_(0, arg->previousNode);
	else
		setState_(2, arg->previousNode);
	return numGenerated_;
}

size_t zenith::terragen::MountainForkGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainRidgeNode(p0_, p1_[nodeId], curParent_, this);
	return szReq;
}
*/

void zenith::terragen::MountainForkGenerator1::setFixedState_(const GeneratorArguments * arg)
{
	stateFix_.numPrevPts_ = 1;

	if (dynamic_cast<const MountainRidgeNode *>(arg->previousNode))
	{
		const MountainRidgeNode * pNode = static_cast<const MountainRidgeNode *>(arg->previousNode);
		stateFix_.prevPts_[0] = pNode->point0();
		stateFix_.p0_ = pNode->point1();
		stateFix_.od_ = pNode->overallDirection();
		stateFix_.sz_ = pNode->sizeParam();
		stateFix_.prevLen_ = glm::length(GVec2(pNode->point1() - pNode->point0()));

		while (stateFix_.numPrevPts_ < InternalStateFix_::MaxNumPrevPts && dynamic_cast<const MountainRidgeNode *>(pNode->getParent()))
		{
			pNode = static_cast<const MountainRidgeNode *>(pNode->getParent());
			stateFix_.prevPts_[stateFix_.numPrevPts_] = pNode->point0();
			stateFix_.numPrevPts_++;
		}
	}
	else throw TerraGenGeneratorException("MountainForkGenerator1::generate(): parent node expected to be one of following types: MountainRidge!");

}

MountainForkGenerator1::InternalStateVar_ zenith::terragen::MountainForkGenerator1::genState_(const GeneratorArguments * arg)
{
	InternalStateVar_ res;

	GVec3 prevDir = stateFix_.p0_ - stateFix_.prevPts_[0];
	GVec3 dir = glm::normalize(mixDirs_(stateFix_.od_, prevDir));

	double xyA0 = a0Rnd_();
	double zhA0 = bRnd_();
	GVec3 newDir0 = rotateDir(dir, xyA0, zhA0, stateFix_.od_);

	double xyA1 = (xyA0 > 0.0 ? xyA0 - glm::half_pi<double>() : xyA0 + glm::half_pi<double>()) + a1Rnd_();
	double zhA1 = bRnd_();
	GVec3 newDir1 = rotateDir(dir, xyA1, zhA1, stateFix_.od_);

	double l0 = stateFix_.sz_ * lenRnd_();
	double l1 = stateFix_.sz_ * lenRnd_() * 0.5;

	res.p1_ = stateFix_.p0_ + l0 * newDir0;
	res.p2_ = stateFix_.p0_ + l1 * newDir1;

	return res;
}

double zenith::terragen::MountainForkGenerator1::evaluateState_(const InternalStateVar_ & state, const GeneratorArguments * arg)
{
	MountainRidgeNode m1(stateFix_.p0_, state.p1_, stateFix_.sz_, stateFix_.od_, stateFix_.topNode_, arg->previousNode, this);
	MountainRidgeNode m2(stateFix_.p0_, state.p2_, stateFix_.sz_ * 0.5, state.p2_ - stateFix_.p0_, stateFix_.topNode_, arg->previousNode, this);

	double nl1 = glm::length(GVec2(state.p1_ - stateFix_.p0_));
	if (stateFix_.prevLen_ > 0.0)
		nl1 = glm::min(nl1, stateFix_.prevLen_);
	double nl2 = glm::length(GVec2(state.p2_ - stateFix_.p0_));
	if (stateFix_.prevLen_ > 0.0)
		nl2 = glm::min(nl2, stateFix_.prevLen_);
	double minDistance = TerraGenInfty;
	for (uint32_t j = 0; j < arg->allNodesNum; j++)
	{
		const BaseNode * ptr = arg->allNodesPtr[j];
		if (ptr == arg->previousNode)
			continue;
		double dst1 = getNodeDistance(&m1, ptr, 50.0) / nl1;
		if (dst1 < minDistance)
			minDistance = dst1;
		double dst2 = getNodeDistance(&m2, ptr, 50.0) / nl2;
		if (dst2 < minDistance)
			minDistance = dst2;
	}
	if (minDistance < minDst_)
		return -1.0;
	return minDistance;
}

zenith::terragen::MountainForkGenerator1::MountainForkGenerator1(const MountainForkGenerator1_Params & p)
	:a0Distr_(p.alpha0.mu, p.alpha0.sigma), a00_(p.alpha0Box.x0), a01_(p.alpha0Box.x1),
	a1Distr_(p.alpha1.mu, p.alpha1.sigma), a10_(p.alpha1Box.x0), a11_(p.alpha1Box.x1),
	bDistr_(p.beta.mu, p.beta.sigma), b0_(p.betaBox.x0), b1_(p.betaBox.x1),
	mixDistr_(p.mix.mu, p.mix.sigma), mix0_(p.mixBox.x0), mix1_(p.mixBox.x1),
	lenDistr_(p.len.mu, p.len.sigma), len0_(p.lenBox.x0), len1_(p.lenBox.x1),
	numTries_(p.numTries), minDst_(p.minNodeDistance)
{
}

uint32_t zenith::terragen::MountainForkGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);
	setFixedState_(arg);

	InternalStateVar_ bestState;
	double bestVal = -1.0;

	for (uint32_t i = 0; i < numTries_; i++)
	{
		InternalStateVar_ curState = genState_(arg);
		double val = evaluateState_(curState, arg);
		if (val > bestVal)
		{
			bestVal = val;
			bestState = curState;
		}
	}

	if (bestVal > 0.0)
	{
		setState_(2, arg->previousNode);
		stateVar_ = bestState;
	}
	else setState_(0, arg->previousNode);

	return numGenerated_;
}

size_t zenith::terragen::MountainForkGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	if(nodeId == 0)
		new (buffPtr) MountainRidgeNode(stateFix_.p0_, stateVar_.p1_, stateFix_.sz_, stateFix_.od_, stateFix_.topNode_, curParent_, this);
	else
		new (buffPtr) MountainRidgeNode(stateFix_.p0_, stateVar_.p2_, stateFix_.sz_ * 0.5, stateVar_.p2_ - stateFix_.p0_, stateFix_.topNode_, curParent_, this);
	return szReq;
}
