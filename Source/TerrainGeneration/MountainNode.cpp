#include "MountainNode.h"
#include "NodeDispatcher.h"
#include <glm\gtc\constants.hpp>

using namespace zenith::terragen;


MountainTopGenerator1::MountainTopGenerator1(const MountainTopGenerator1_Params & p)
	: hDistr_(p.heightLambda), numTries_(p.numTries), minDstNode_(p.minNodeDistance), minDstTop_(p.minMountainTopDistance),
	xParam_(p.boundingBox.x0, p.boundingBox.x1), yParam_(p.boundingBox.y0, p.boundingBox.y1)
{
	//p.numTops;
}

MountainTopRidgeGenerator1::MountainTopRidgeGenerator1(const MountainTopRidgeGenerator1_Params & p)
	: aDistr_(p.alpha.mu, p.alpha.sigma), bDistr_(p.beta.mu, p.beta.sigma), b0_(p.betaBox.x0), b1_(p.betaBox.x1),
	lenDistr_(p.len.mu, p.len.sigma), len0_(p.lenBox.x0), len1_(p.lenBox.x1), numTries_(p.numTries), minDst_(p.minNodeDistance)
{
	//p.numRidges	
}


MountainContGenerator1::MountainContGenerator1(const MountainContGenerator1_Params & p)
	: aDistr_(p.alpha.mu, p.alpha.sigma), a0_(p.alphaBox.x0), a1_(p.alphaBox.x1),
	mixDistr_(p.mix.mu, p.mix.sigma), mix0_(p.mixBox.x0), mix1_(p.mixBox.x1),
	lenDistr_(p.len.mu, p.len.sigma), len0_(p.lenBox.x0), len1_(p.lenBox.x1), numTries_(p.numTries), minDst_(p.minNodeDistance)
{
}

/*
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

	return numGenerated_;
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

uint32_t zenith::terragen::MountainTopRidgeGenerator1::generate(const GeneratorArguments * arg)
{
	setSeed_(arg->seedNumber);

	uint32_t numG = pDistr_(randomEngine_);
	if (numG > MaxNodes)
		numG = MaxNodes;

	const BaseNode * parent = arg->previousNode;
	if (!parent)
		throw TerraGenGeneratorException("MountainTopRidgeGenerator1::generate(): parent node expected to be non-null!");

	if (dynamic_cast<const MountainTopNode *>(parent))
		p0_ = static_cast<const MountainTopNode *>(parent)->point();
	else if (dynamic_cast<const MountainRidgeNode *>(parent))
		p0_ = static_cast<const MountainRidgeNode *>(parent)->point1();
	else throw TerraGenGeneratorException("MountainTopRidgeGenerator1::generate(): parent node expected to be one of following types: MountainTop, MountainRidge!");

	uint32_t realNum = 0;
	double step = glm::two_pi<double>() / double(numG);
	double sigma = 0.3 * step;
	

	for (uint32_t i = 0; i < numG; i++)
	{
		double a = double(i) * step + nDistr_(randomEngine_, std::normal_distribution<>::param_type(0, sigma));
		double dx = glm::cos(a);
		double dy = glm::sin(a);
		double dh = glm::min<double>(-0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(-3.0, 1.0)));
		double dl = lnDistr_(randomEngine_);

		p1_[realNum] = p0_ + GVec3(dx*dl, dy*dl, dh*dl);

		MountainRidgeNode m(p0_, p1_[realNum], parent, this);

		double minDistance = TerraGenInfty;
		for (uint32_t j = 0; j < arg->allNodesNum; j++)
		{
			const BaseNode * ptr = arg->allNodesPtr[j];
			if (ptr == parent)
				continue;
			double dst = getNodeDistance(&m, ptr, 50.0);
			if (dst < minDistance)
				minDistance = dst;
		}
		if (minDistance < 0.5)
			continue;
		realNum++;
	}

	setState_(realNum, arg->previousNode);
	return numGenerated_;
}

size_t zenith::terragen::MountainTopRidgeGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainRidgeNode(p0_, p1_[nodeId], curParent_, this);
	return szReq;
}

uint32_t zenith::terragen::MountainContGenerator1::generate(const GeneratorArguments * arg)
{
	uint32_t numG = 1;

	const BaseNode * parent = arg->previousNode;
	if (!parent)
		throw TerraGenGeneratorException("MountainContGenerator1::generate(): parent node expected to be non-null!");

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
	}else throw TerraGenGeneratorException("MountainContGenerator1::generate(): parent node expected to be one of following types: MountainRidge!");

	GVec2 mainDir = p0_ - prevPoints[0];
	double mainDirL = glm::length(mainDir);
	mainDir /= GVec2::value_type(mainDirL);
	double bestDist = 0;
	double sigmaL = mainDirL * 0.3;
	double sigmaD = 0.2;
	double mainDirDH = std::min<double>(0.0, (p0_.z - prevPoints[0].z)) * 0.8;

	for (uint32_t i = 0; i < numTries_; i++)
	{
		double dx = nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDir.x, sigmaD));
		double dy = nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDir.y, sigmaD));
		double dl = glm::max<double>(0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirL, sigmaL)));
		double dh = glm::min<double>(-0.2, nDistr_(randomEngine_, std::normal_distribution<>::param_type(mainDirDH, 1.0)));
		
		GVec3 p1 = p0_ + GVec3(dx * dl, dy * dl, dh);
		MountainRidgeNode m(p0_, p1, parent, this);

		double minDistance = TerraGenInfty;
		for (uint32_t j = 0; j < arg->allNodesNum; j++)
		{
			const BaseNode * ptr = arg->allNodesPtr[j];
			if (ptr == parent)
				continue;
			double dst = getNodeDistance(&m, ptr, 50.0);
			if (dst < minDistance)
				minDistance = dst;
		}
		if (minDistance < 0.5 * glm::max(1.0,dl))
			continue;
		if (minDistance > bestDist)
		{
			bestDist = minDistance;
			p1_ = p1;
		}
	}
	if(isZero(bestDist))
		setState_(0, arg->previousNode);
	else
		setState_(1, arg->previousNode);
	return numGenerated_;
}

size_t zenith::terragen::MountainContGenerator1::get(uint32_t nodeId, void * buffPtr, size_t buffSize) const
{
	checkId_(nodeId);
	size_t szReq = checkSize_<MountainRidgeNode>(buffSize);
	if (szReq == 0)
		return 0;
	new (buffPtr) MountainRidgeNode(p0_, p1_, curParent_, this);
	return szReq;
}

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

