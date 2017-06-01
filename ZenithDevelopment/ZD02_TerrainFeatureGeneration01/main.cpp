#include <iostream>
#include <TerrainGeneration\MountainNode.h>
#include <TerrainGeneration\NodeFactory.h>
#include <TerrainGeneration\Params\Config.h>
#include <TerrainGeneration\NodeTransform\NodeTransform.h>
#include <Utils\FileFormat\ff_img.h>
#include <Utils\IO\filesystem.h>
#include <Utils\xml_tools.h>
#include <Utils\Math\SolverMultigrid.h>

zenith::util::io::FileSystem * fs = nullptr;

template<class Elem, class Dim>
zenith::util::math::GridAccessor2D<Elem, Dim> newGrid(size_t xSize, size_t ySize, Dim xMin, Dim xMax, Dim yMin, Dim yMax, Elem def)
{
	Elem * data = new Elem[xSize * ySize];
	zenith::util::math::GridAccessor2D<Elem, Dim> res(data,
		static_cast<zenith::util::math::GridAccessor2D<Elem, Dim>::SizeType>(xSize), static_cast<zenith::util::math::GridAccessor2D<Elem, Dim>::SizeType>(ySize));
	for (size_t i = 0; i < xSize * ySize; i++)
		data[i] = def;
	res.setDimensions(xMin, xMax, yMin, yMax);
	return res;
}

template<class Elem, class ElemSrc, class Dim>
zenith::util::math::GridAccessor2D<Elem, Dim> newGrid(const zenith::util::math::GridAccessor2D<ElemSrc, Dim> &src)
{
	Elem * data = new Elem[src.xSize() * src.ySize()];
	zenith::util::math::GridAccessor2D<Elem, Dim> res(data, src.xSize(), src.ySize());
	for (zenith::util::math::GridAccessor2D<Elem, Dim>::SizeType xi = 0; xi < src.xSize(); xi++)
		for (zenith::util::math::GridAccessor2D<Elem, Dim>::SizeType yi = 0; yi < src.ySize(); yi++)
			res.get(xi, yi) = Elem(src.get(xi, yi));
	res.setDimensions(src.xMin(), src.xMax(), src.yMin(), src.yMax());
	return res;
}

template<class Elem, class Dim>
void deleteGrid(zenith::util::math::GridAccessor2D<Elem, Dim> &grid)
{
	delete grid.elements();
}

inline std::future<zenith::util::io::FileResult> writeFile(const char * fname, uint8_t * data, size_t size, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'w', priority);
	auto r = f.write(data, size);
	f.close();
	return r;
}


void saveGridZIMG(const zenith::util::math::GridAccessor2D<glm::vec3> &gSrc, const char * fname)
{
	zenith::util::zfile_format::zImgDescription descr;

	descr.imageType = zenith::util::zfile_format::ImageType::IMG2D;
	descr.imageFormat = zenith::util::zfile_format::ImageFormat::R32G32B32F;
	descr.mipLevels = 1;
	descr.arraySize = 1;
	descr.depth = 1;

	descr.width = gSrc.xSize();
	descr.height = gSrc.ySize();

	descr.levelDescr[0] = new zenith::util::zfile_format::zImgDataDescription;

	uint8_t pixSize = zenith::util::zfile_format::getPixelSize(descr.imageFormat);
	uint64_t rowPitch = pixSize * descr.width;
	uint64_t dataSize = rowPitch * descr.height;

	auto &dd = *(descr.levelDescr[0]);

	dd = zenith::util::zfile_format::zImgDataDescription::create2D(descr.width, descr.height, rowPitch);
	descr.levelData[0] = gSrc.data();


	uint64_t fSize = descr.getFullSize();
	uint8_t * fData = new uint8_t[fSize];
	zenith::util::zfile_format::zimg_to_mem(descr, fData, fSize);

	auto futW = writeFile(fname, fData, fSize);
	futW.get();

	delete[] fData;
	delete descr.levelDescr[0];
}

void saveGridZIMG(const zenith::util::math::GridAccessor2D<float> &gSrc, const char * fname)
{
	zenith::util::zfile_format::zImgDescription descr;

	descr.imageType = zenith::util::zfile_format::ImageType::IMG2D;
	descr.imageFormat = zenith::util::zfile_format::ImageFormat::R32F;
	descr.mipLevels = 1;
	descr.arraySize = 1;
	descr.depth = 1;

	descr.width = gSrc.xSize();
	descr.height = gSrc.ySize();

	descr.levelDescr[0] = new zenith::util::zfile_format::zImgDataDescription;

	uint8_t pixSize = zenith::util::zfile_format::getPixelSize(descr.imageFormat);
	uint64_t rowPitch = pixSize * descr.width;
	uint64_t dataSize = rowPitch * descr.height;

	auto &dd = *(descr.levelDescr[0]);

	dd = zenith::util::zfile_format::zImgDataDescription::create2D(descr.width, descr.height, rowPitch);
	descr.levelData[0] = gSrc.data();


	uint64_t fSize = descr.getFullSize();
	uint8_t * fData = new uint8_t[fSize];
	zenith::util::zfile_format::zimg_to_mem(descr, fData, fSize);

	auto futW = writeFile(fname, fData, fSize);
	futW.get();

	delete[] fData;
	delete descr.levelDescr[0];
}


int main()
{	
	fs = new zenith::util::io::FileSystem(1);
	zenith::terragen::TerraFactory<1024 * 128, 1024 * 32> * pTerraFactory = new zenith::terragen::TerraFactory<1024 * 128, 1024 * 32>;
	auto &terraFactory = *pTerraFactory;

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
	
	terraFactory.setMainMetaGenerator("MntMetaGen1");
	terraFactory.setSeed(7);

	uint32_t numWaves = 10;
	for (uint32_t i = 0; i < numWaves; i++)
	{
		uint32_t numGen = terraFactory.generateNewWave();
		std::cout << "For wave=" << i << " generated " << numGen << " nodes;\n";
	}

	double xMin = zenith::terragen::TerraGenInfty, xMax = -zenith::terragen::TerraGenInfty;
	double yMin = zenith::terragen::TerraGenInfty, yMax = -zenith::terragen::TerraGenInfty;
	std::ofstream ridges("ridges.txt");
	ridges << "plot_lines([";
	for (uint32_t i = 0; i < terraFactory.numNodes(); i++)
	{
		const zenith::terragen::SegmentNode * sNode = dynamic_cast<const zenith::terragen::SegmentNode *>(terraFactory.getNode(i));
		if (!sNode)
			continue;
		ridges << "[ [ " << sNode->point0().x << ", " << sNode->point0().y << ", " << sNode->point0().z << " ], [ " << sNode->point1().x << ", " << sNode->point1().y << ", " << sNode->point1().z << "] ],\n";

		if (sNode->point0().x > xMax)
			xMax = sNode->point0().x;
		if (sNode->point1().x > xMax)
			xMax = sNode->point1().x;

		if (sNode->point0().y > yMax)
			yMax = sNode->point0().y;
		if (sNode->point1().y > yMax)
			yMax = sNode->point1().y;


		if (sNode->point0().x < xMin)
			xMin = sNode->point0().x;
		if (sNode->point1().x < xMin)
			xMin = sNode->point1().x;

		if (sNode->point0().y < yMin)
			yMin = sNode->point0().y;
		if (sNode->point1().y < yMin)
			yMin = sNode->point1().y;
	}
	ridges << "])";
	ridges.close();


	const size_t GridSize = 1024, MaxGrids = 20;
	size_t multigridSetupSize = zenith::util::math::multigrid_required_size_setup_full<double>(GridSize, GridSize);
	size_t multigridRunSize = zenith::util::math::multigrid_required_size_run_full<double>(GridSize, GridSize);
	uint8_t * multigridSetupBuff = new uint8_t[multigridSetupSize];
	uint8_t * multigridRunBuff = new uint8_t[multigridRunSize];
	std::vector<zenith::util::math::GridAccessor2D<glm::dvec2>> gridsWeights(MaxGrids);
	std::vector<zenith::util::math::GridAccessor2D<glm::dvec3>> gridsGradients(MaxGrids);
	std::vector<zenith::util::math::GridAccessor2D<double>> gridsRHS(MaxGrids);
	std::vector<zenith::util::math::GridAccessor2D<double>> gridsConstraints(MaxGrids);
	zenith::util::math::GridAccessor2D<double> gridResult;
	uint32_t numGrids = MaxGrids;

	zenith::util::math::multigrid_setup(multigridSetupBuff, multigridSetupSize, GridSize, GridSize,
		xMin, xMax, yMin, yMax, &gridsWeights[0],
		&gridsGradients[0], &gridsRHS[0], &gridsConstraints[0], numGrids);
	
	zenith::terragen::transformNodeToMGSsetup(terraFactory.getNodes(),
		&gridsWeights[0], &gridsGradients[0], &gridsConstraints[0], &gridsRHS[0],
		numGrids, multigridSetupBuff, multigridSetupSize);

	for(size_t yi = 0; yi < gridsWeights[4].ySize(); yi++)
		for (size_t xi = 0; xi < gridsWeights[4].xSize(); xi++)
		{
			const auto &r = gridsGradients[4].get(xi, yi);
			if (gridsWeights[4].get(xi, yi).y > 0.0)
				std::cout << "strange";
//			std::cout << xi << " " << yi << " " << r.x << " " << r.y << " " << r.z << "\n";
		}


	zenith::util::math::multigrid_run<glm::dvec2, glm::dvec3, double, double>(gridResult,
		multigridRunBuff, multigridRunSize,
		//"j10uj100uj100uj100uj100uj100uj100uj100uj10uj10",
		"j10uj10uj10uj10uj10uj10uj10uj10uj10uj10",
		nullptr, &gridsWeights[0], &gridsGradients[0], &gridsConstraints[0], &gridsRHS[0],
		numGrids);

	double vMin = zenith::terragen::TerraGenInfty;
	double vMax = -zenith::terragen::TerraGenInfty;
	for(uint32_t yi = 0; yi < gridResult.ySize(); yi++)
		for (uint32_t xi = 0; xi < gridResult.xSize(); xi++)
		{
			double v = gridResult.get(xi, yi);
			if (v > vMax)
				vMax = v;
			if (v < vMin)
				vMin = v;
		}

	for (uint32_t yi = 0; yi < gridResult.ySize(); yi++)
		for (uint32_t xi = 0; xi < gridResult.xSize(); xi++)
		{
			double v = gridResult.get(xi, yi);
			gridResult.get(xi, yi) = (v - vMin) / (vMax - vMin);
		}

	/*
	auto gridRHS = newGrid(GridSize, GridSize, xMin, xMax, yMin, yMax, 0.0);
	auto gridConstraint = newGrid(GridSize, GridSize, xMin, xMax, yMin, yMax, 0.0);
	auto gridGradient = newGrid(GridSize, GridSize, xMin, xMax, yMin, yMax, glm::dvec3(0.0, 0.0, 0.0));
	auto gridWeights = newGrid(GridSize, GridSize, xMin, xMax, yMin, yMax, glm::dvec4(0.0, 0.0, 0.0, 0.0));
	
	zenith::terragen::transformNodeToMGS(terraFactory.getNodes(), gridWeights, 
		gridGradient, gridRHS, gridConstraint);
	
	size_t numFilled = 0;
	for(size_t yi = 0; yi < gridWeights.ySize(); yi++)
		for (size_t xi = 0; xi < gridWeights.xSize(); xi++)
		{
			const glm::dvec4 &w = gridWeights.get(xi, yi);
		}
	std::cout << "Num filled nodes: " << numFilled << "\n";
	auto gridRes = newGrid<glm::vec3>(gridWeights);
	*/
	auto gridRes = newGrid<float>(gridResult);
	saveGridZIMG(gridRes, "initialGrid.zimg");

	delete pTerraFactory;
	/*
	deleteGrid(gridRHS);
	deleteGrid(gridRes);
	deleteGrid(gridConstraint);
	deleteGrid(gridGradient);
	deleteGrid(gridWeights);
	*/
	std::cout << "\nFinished\n";
	_sleep(500);
	return 0;
}