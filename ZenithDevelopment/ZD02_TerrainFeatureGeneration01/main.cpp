#include <iostream>
#include <TerrainGeneration\MountainNode.h>
#include <TerrainGeneration\NodeFactory.h>
#include <TerrainGeneration\Params\Config.h>
#include <Utils\xml_tools.h>
#include <Utils\Math\categorical_distribution.h>

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
	zenith::terragen::TerraFactory<1024 * 128, 1024 * 32> terraFactory;

	pugi::xml_document xmlGenParams;
	xmlGenParams.load_file("TerraGenConfig.xml");
	

	auto xmlGeneratorSection = xmlGenParams.child("TerraGenConfig").child("Generators");
	for (const auto &ch : xmlGeneratorSection.children("Generator"))
	{
		zenith::util::ObjectMap<char, char> genParams;
		zenith::util::xml::xml2objmap(ch, genParams);
		zenith::util::nameid uid = terraFactory.constructGenerator(genParams);
		std::cout << uid.c_str() << "\n";
	}

	//zenith::terragen::MountainForkGenerator1 genForkRidge(5);
	//nodeFactory.registerGenerator("ridge-fork", &genForkRidge);

	terraFactory.setMainMetaGenerator("MntMetaGen1");
	terraFactory.setSeed(7);

	uint32_t numWaves = 10;
	for (uint32_t i = 0; i < numWaves; i++)
	{
		uint32_t numGen = terraFactory.generateNewWave();
		std::cout << "For wave=" << i << " generated " << numGen << " nodes;\n";
	}

	std::ofstream ridges("ridges.txt");
	ridges << "plot_lines([";
	for (uint32_t i = 0; i < terraFactory.numNodes(); i++)
	{
		const zenith::terragen::SegmentNode * sNode = dynamic_cast<const zenith::terragen::SegmentNode *>(terraFactory.getNode(i));
		if (!sNode)
			continue;
		ridges << "[ [ " << sNode->point0().x << ", " << sNode->point0().y << " ], [ " << sNode->point1().x << ", " << sNode->point1().y << "] ],\n";
	}
	ridges << "])";
	ridges.close();
	_sleep(500);
	return 0;
}