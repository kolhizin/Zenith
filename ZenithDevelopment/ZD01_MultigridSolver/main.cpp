
#include <iostream>
#include "../../Source/Utils/IO/filesystem.h"
#include "../../Source/Utils/FileFormat/ff_img.h"
#include "../../Source/Utils/Math/GridAccessor.h"
#include "../../Source/Utils/Math/SolverMultigrid.h"

#include <vector>

zenith::util::io::FileSystem * fs;

using namespace zenith::util::math;


float evalDistCircle(float x, float y, float x0, float y0, float r0)
{
	float r = sqrtf((x - x0) * (x - x0) + (y - y0) * (y - y0));
	return abs(r - r0);
}
float evalDistLine(float x, float y, float A, float B, float C)
{
	return x * A + y * B + C;
}

std::pair<float, float> evalFunctionAndMask(float x, float y, float dx, float dy)
{
	float x1 = 0.25f, y1 = 0.35f, r1 = 0.45f, v1 = 0.15f;
	float x2 = 0.1f, y2 = 0.2f, r2 = 0.3f, v2 = 0.75f;
	float x3 = 0.8f, y3 = 0.7f, r3 = 0.1f, v3 = 0.4f;

	float coef = 0.7f;

	if (evalDistCircle(x, y, x1, y1, r1) < dx*coef)
		return std::make_pair(1.0f, v1);
	if (evalDistCircle(x, y, x2, y2, r2) < dx*coef)
		return std::make_pair(1.0f, v2);
	if (evalDistCircle(x, y, x3, y3, r3) < dx*coef)
		return std::make_pair(1.0f, v3);
	return std::make_pair(0.0f, 0.0f);
}
float evalMask(float x, float y, float dx, float dy)
{
	return evalFunctionAndMask(x, y, dx, dy).first;
}
float evalFunction(float x, float y, float dx, float dy)
{
	return evalFunctionAndMask(x, y, dx, dy).second;
}

float evalRHS(float x, float y, float dx, float dy)
{
	float x1 = 0.9f, y1 = 0.1f, r1 = 0.05f, v1 = 50.0f;
	float x2 = 0.9f, y2 = 0.1f, r2 = 0.01f, v2 = -1000.0f;

	float coef = 0.7f;

	if (evalDistCircle(x, y, x1, y1, r1) < dx)
		return v1;
	if (evalDistCircle(x, y, x2, y2, r2) < dx)
		return v2;
	return 0.0f;
}

void setupConstraints(GridAccessor2D<float> &gV, GridAccessor2D<float> &gM, float * pV, float * pM, uint32_t gSize)
{
	gV = GridAccessor2D<float>(pV, gSize, gSize);
	gM = GridAccessor2D<float>(pM, gSize, gSize);

	evaluateGrid(gM, evalMask);
	evaluateGrid(gV, evalFunction);
}
void setupRHS(GridAccessor2D<float> &gRHS, float * pRHS, uint32_t gSize)
{
	gRHS = GridAccessor2D<float>(pRHS, gSize, gSize);

	evaluateGrid(gRHS, evalRHS);
}
void setupGridSize(GridAccessor2D<float> &gSrc, GridAccessor2D<float> &gDst, float * pSrc, float * pDst, uint32_t gSize)
{

	gSrc = GridAccessor2D<float>(pSrc, gSize, gSize);
	gDst = GridAccessor2D<float>(pDst, gSize, gSize);
}


inline std::future<zenith::util::io::FileResult> writeFile(const char * fname, uint8_t * data, size_t size, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'w', priority);
	auto r = f.write(data, size);
	f.close();
	return r;
}

void saveGridZIMG(const GridAccessor2D<float> &gSrc, const char * fname)
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



void freeZIMG(zenith::util::zfile_format::zImgDescription &descr)
{
	for (size_t i = 0; i < descr.mipLevels; i++)
		delete descr.levelDescr[i];
}
int main()
{
	fs = new zenith::util::io::FileSystem(1);

	const uint64_t buffSize = 2048 * 2048 * 3;

	float * pV = new float[2048 * 2048], *pcV;
	float * pM = new float[2048 * 2048], *pcM;
	float * pRHS = new float[2048 * 2048], *pcRHS;
	float * pInit = new float[2048 * 2048];
	float * pBuff = new float[buffSize];

	std::vector<GridAccessor2D<float>> gV, gM, gRHS;
	gV.reserve(10);
	gM.reserve(10);
	gRHS.reserve(10);

	GridAccessor2D<float> gS0(pInit, 2, 2), gRes;
	fillGrid(gS0, 0.0f);

	uint32_t maxSize = 1024;
	pcV = pV; pcM = pM; pcRHS = pRHS;
	for (uint32_t curSize = gS0.xSize(); curSize <= maxSize; curSize *= 2)
	{
		gV.resize(gV.size() + 1);
		gM.resize(gM.size() + 1);
		gRHS.resize(gRHS.size() + 1);

		setupConstraints(gV.back(), gM.back(), pcV, pcM, curSize);
		pcV += curSize * curSize;
		pcM += curSize * curSize;

		setupRHS(gRHS.back(), pcRHS, curSize);
		pcRHS += curSize * curSize;
	}

	multigrid_run<float>(gRes, pBuff, buffSize, "uj100uj100uj100uj100uj100uj100uj100uj10uj10", gS0, &gM[0], &gV[0], &gRHS[0], static_cast<uint32_t>(gM.size()));

	std::cout << gRes.xSize() << "\n";

	saveGridZIMG(gRes, "initialGrid.zimg");

	delete fs;



	std::cout << "Finished!";
	return 0;
}