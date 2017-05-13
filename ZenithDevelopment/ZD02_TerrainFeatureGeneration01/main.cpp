#include <iostream>
#include <fstream>
#include <TerrainGeneration\MountainNode.h>
#include <TerrainGeneration\NodeFactory.h>

class MetaGenerator1
{
	std::mt19937 randomEngine_;
	std::uniform_real_distribution<> uDistr_;
public:
	inline zenith::util::nameid generate(const zenith::terragen::GeneratorArguments * arg)
	{
		if (!arg->previousNode)
			return zenith::util::nameid("top");
		if (strcmp(arg->previousNode->nodeClass(), "Mountain/Top") == 0)
			return zenith::util::nameid("ridge-top");
		double rnd = uDistr_(randomEngine_);
		if(rnd < 0.01)
			return zenith::util::nameid("ridge-top");
		if (rnd > 0.90)
		{
			std::cout << "fork\n";
			return zenith::util::nameid("ridge-fork");
		}
		return zenith::util::nameid("ridge-cont");
	}
	inline void setSeed(uint32_t seed)
	{
		randomEngine_.seed(seed);
	}

	inline static zenith::util::nameid generate(void * ptr, const zenith::terragen::GeneratorArguments * arg)
	{
		return reinterpret_cast<MetaGenerator1 *>(ptr)->generate(arg);
	}
	inline static void setSeed(void * ptr, uint32_t seed)
	{
		reinterpret_cast<MetaGenerator1 *>(ptr)->setSeed(seed);
	}
};

int main()
{
	zenith::terragen::MountainTopGenerator1 genTop;
	zenith::terragen::MountainTopRidgeGenerator1 genTopRidge;
	zenith::terragen::MountainContGenerator1 genContRidge(5);
	zenith::terragen::MountainForkGenerator1 genForkRidge(5);
	MetaGenerator1 metaGen;

	zenith::terragen::NodeFactory<1024 * 32> nodeFactory;

	nodeFactory.registerGenerator("top", &genTop);
	nodeFactory.registerGenerator("ridge-top", &genTopRidge);
	nodeFactory.registerGenerator("ridge-cont", &genContRidge);
	nodeFactory.registerGenerator("ridge-fork", &genForkRidge);
	nodeFactory.registerMetaGenerator(MetaGenerator1::generate, MetaGenerator1::setSeed, &metaGen);

	nodeFactory.setSeed(12);

	uint32_t numWaves = 20;
	for (uint32_t i = 0; i < numWaves; i++)
	{
		uint32_t numGen = nodeFactory.generateNewWave();
		std::cout << "For wave=" << i << " generated " << numGen << " nodes;\n";
	}

	std::ofstream ridges("ridges.txt");
	ridges << "plot_lines([";
	for (uint32_t i = 0; i < nodeFactory.numNodes(); i++)
	{
		const zenith::terragen::SegmentNode * sNode = dynamic_cast<const zenith::terragen::SegmentNode *>(nodeFactory.getNode(i));
		if (!sNode)
			continue;
		ridges << "[ [ " << sNode->point0().x << ", " << sNode->point0().y << " ], [ " << sNode->point1().x << ", " << sNode->point1().y << "] ],\n";
	}
	ridges << "])";
	ridges.close();

	std::cout << "\nTest 2";
	return 0;
}