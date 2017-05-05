#pragma once
#include "GridAccessor.h"


namespace zenith
{
	namespace util
	{
		namespace math
		{

			enum class MultigridSolverError {UNKNOWN=0, ARGUMENT_ERROR, ARGUMENT_DIMENSION_MISMATCH, STEP_SIZE_NOT_SUPPORTED, UNSUPPORTED_COMMAND, OUT_OF_MEMORY, STEP_OUT_OF_SIZE_LIMIT};
			enum class MultigridSolverStep {NONE=0, UP_SIMPLE, DOWN_SIMPLE, JSMOOTH_SIMPLE};

			template<class Elem>
			bool multigrid_prolong(const GridAccessor2D<Elem> &gCoarse, GridAccessor2D<Elem> &gFine)
			{
				//assert sizes:
				if (gCoarse.xSize() * 2 != gFine.xSize() || gCoarse.ySize() * 2 != gFine.ySize())
					return false;

				for (GridAccessor2D<Elem>::SizeType yi = 0; yi < gCoarse.ySize(); yi++)
					for (GridAccessor2D<Elem>::SizeType xi = 0; xi < gCoarse.xSize(); xi++)
					{
						GridAccessor2D<Elem>::SizeType fy = yi * 2, fx = xi * 2;
						GridAccessor2D<Elem>::SizeType xn = gCoarse.xClamp(xi + 1);
						GridAccessor2D<Elem>::SizeType yn = gCoarse.yClamp(yi + 1);

						gFine(fx, fy) = gCoarse(xi, yi);
						gFine(fx + 1, fy) = Elem(0.5) * (gCoarse(xi, yi) + gCoarse(xn, yi));
						gFine(fx, fy + 1) = Elem(0.5) * (gCoarse(xi, yi) + gCoarse(xi, yn));
						gFine(fx + 1, fy + 1) = Elem(0.25) * (gCoarse(xi, yi) + gCoarse(xi, yn) + gCoarse(xn, yi) + gCoarse(xn, yn));
					}
				return true;
			}
			template<class Elem>
			bool multigrid_restrict(const GridAccessor2D<Elem> &gFine, GridAccessor2D<Elem> &gCoarse)
			{
				//assert sizes:
				if (gCoarse.xSize() * 2 != gFine.xSize() || gCoarse.ySize() * 2 != gFine.ySize())
					return false;

				for (GridAccessor2D<Elem>::SizeType yi = 0; yi < gCoarse.ySize(); yi++)
					for (GridAccessor2D<Elem>::SizeType xi = 0; xi < gCoarse.xSize(); xi++)
					{
						GridAccessor2D<Elem>::SizeType fy = yi * 2, fx = xi * 2;
						GridAccessor2D<Elem>::SizeType px = gFine.xClamp(int64_t(fx) - 1);
						GridAccessor2D<Elem>::SizeType nx = gFine.xClamp(fx + 1);
						GridAccessor2D<Elem>::SizeType py = gFine.yClamp(int64_t(fy) - 1);
						GridAccessor2D<Elem>::SizeType ny = gFine.yClamp(fy + 1);

						gCoarse(xi, yi) = Elem(0.0625) * (Elem(4) * gFine(fx, fy)
							+ Elem(2) * (gFine(fx, py) + gFine(fx, ny) + gFine(px, fy) + gFine(nx, fy))
							+ (gFine(px, py) + gFine(px, ny) + gFine(nx, py) + gFine(nx, ny)));
					}
				return true;
			}

			//need to implement gradient constraint
			template<class Elem>
			bool multigrid_jrelax(GridAccessor2D<Elem> &gDst, const GridAccessor2D<Elem> &gSrc, const GridAccessor2D<Elem> *gVal, const GridAccessor2D<Elem> *gMask, const GridAccessor2D<Elem> *gRHS, Elem * error = nullptr)
			{
				Elem xh = gDst.xStep();
				Elem yh = gDst.yStep();

				Elem xCoef = Elem(0.5)*yh*yh / (xh*xh + yh*yh);
				Elem yCoef = Elem(0.5)*xh*xh / (xh*xh + yh*yh);
				Elem fCoef = Elem(0.5)*xh*xh*yh*yh / (xh*xh + yh*yh);

				Elem maxUpdate = 0;
				Elem sumUpdate = 0, sumAbsUpdate = 0;

				bool hasMask = (gMask != nullptr);
				bool hasRHS = (gRHS != nullptr);

				for (GridAccessor2D<Elem>::SizeType yi = 0; yi < gDst.ySize(); yi++)
					for (GridAccessor2D<Elem>::SizeType xi = 0; xi < gDst.xSize(); xi++)
					{
						Elem fx = 0.0, fy = 0.0;

						if (xi == 0)
							fx += gSrc(xi, yi);
						else
							fx += gSrc(xi - 1, yi);

						if (xi + 1 == gDst.xSize())
							fx += gSrc(xi, yi);
						else
							fx += gSrc(xi + 1, yi);

						if (yi == 0)
							fy += gSrc(xi, yi);
						else
							fy += gSrc(xi, yi - 1);

						if (yi + 1 == gDst.ySize())
							fy += gSrc(xi, yi);
						else
							fy += gSrc(xi, yi + 1);

						Elem ff = fx * xCoef + fy * yCoef;

						if (hasRHS)
							ff += gRHS->get(xi, yi)*fCoef;

						Elem cf = (gVal ? gVal->get(xi, yi) : Elem(0));

						if (hasMask)
							gDst(xi, yi) = ff * (1.0f - gMask->get(xi, yi)) + cf * (gMask->get(xi, yi));
						else
							gDst(xi, yi) = ff;

						Elem update = gDst(xi, yi) - gSrc(xi, yi);
						sumUpdate += update;
						update = abs(update);
						sumAbsUpdate += update;
						if (update > maxUpdate)
							maxUpdate = update;
					}
				if (error)
					*error = maxUpdate;
				return true;
			}

			template<class Elem>
			bool multigrid_jrelax_n(GridAccessor2D<Elem> &gDst, GridAccessor2D<Elem> &gSrc, const GridAccessor2D<Elem> *gVal, const GridAccessor2D<Elem> *gMask, const GridAccessor2D<Elem> *gRHS, uint32_t nStep, Elem * error = nullptr)
			{
				for (uint32_t i = 0; i < nStep; i++)
				{
					multigrid_jrelax(gDst, gSrc, gVal, gMask, gRHS, error);
					gSrc.swap(gDst);
				}
				return true;
			}

			template<class Elem>
			inline void multigrid_argcheck_(const GridAccessor2D<Elem> &gInit, const GridAccessor2D<Elem> *gMasks, const GridAccessor2D<Elem> *gValues, const GridAccessor2D<Elem> *gRHS, uint32_t numGrids,
				uint32_t &minXSize, uint32_t &minYSize, uint32_t &maxXSize, uint32_t &maxYSize, uint32_t &initInd)
			{
				initInd = 0xFFFFFFFF;

				minXSize = 0xFFFFFFFF; maxXSize = 0;
				minYSize = 0xFFFFFFFF; maxYSize = 0;

				bool hasMask = (gMasks != nullptr);
				bool hasRHS = (gRHS != nullptr);

				if (!hasMask && !hasRHS)
					throw NumericException(MultigridSolverError::ARGUMENT_ERROR);

				uint32_t prevXSize = 0, prevYSize = 0;

				/*check sizes*/
				for (uint32_t i = 0; i < numGrids; i++)
				{
					uint32_t xSize = 0, ySize = 0;
					if (hasMask && (gMasks[i].xSize() != gValues[i].xSize()))
						throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
					if (hasMask && (gMasks[i].ySize() != gValues[i].ySize()))
						throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
					if (hasMask)
					{
						xSize = gMasks[i].xSize();
						ySize = gMasks[i].ySize();
					}

					if (hasMask && hasRHS)
					{
						if ((xSize != gRHS[i].xSize()) || (ySize != gRHS[i].ySize()))
							throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
					}
					else if (hasRHS)
					{
						xSize = gRHS[i].xSize();
						ySize = gRHS[i].ySize();
					}

					if (i > 0)
					{
						if (prevXSize * 2 != xSize || prevYSize * 2 != ySize)
							throw NumericException(MultigridSolverError::STEP_SIZE_NOT_SUPPORTED);
					}

					if (xSize == gInit.xSize() && ySize == gInit.ySize())
						initInd = i;
					
					if (xSize < minXSize)
						minXSize = xSize;
					if (xSize > maxXSize)
						maxXSize = xSize;

					if (ySize < minYSize)
						minYSize = ySize;
					if (ySize > maxYSize)
						maxYSize = ySize;

					prevXSize = xSize; prevYSize = ySize;
				}
			}

			inline MultigridSolverStep multigrid_decode_(const char * src, const char * &dst, uint64_t &out_param)
			{
				if (src[0] == '\0')
					return MultigridSolverStep::NONE;
				if (src[0] == 'u')
				{
					dst = src + 1;
					out_param = 0;
					return MultigridSolverStep::UP_SIMPLE;
				}
				if (src[0] == 'd')
				{
					dst = src + 1;
					out_param = 0;
					return MultigridSolverStep::DOWN_SIMPLE;
				}
				if (src[0] == 'j')
				{
					const char * p = src + 1;
					uint64_t num = 0;
					while (*p >= '0' && *p <= '9')
					{
						num = num * 10 + uint64_t(*p - '0');
						p++;
					}
					dst = p;
					out_param = num;
					return MultigridSolverStep::JSMOOTH_SIMPLE;
				}
				throw NumericException(static_cast<uint64_t>(MultigridSolverError::UNSUPPORTED_COMMAND));
			}

			template<class Elem>
			void multigrid_run(GridAccessor2D<Elem> &gDst, Elem * bufferPtr, const uint64_t bufferSize, const char * command,
				const GridAccessor2D<Elem> &gInit,	const GridAccessor2D<Elem> *gMasks,	const GridAccessor2D<Elem> *gValues, const GridAccessor2D<Elem> *gRHS, uint32_t numGrids)
			{
				uint32_t curIndex = 0;
				uint32_t minXSize, maxXSize, minYSize, maxYSize;

				bool hasMask = (gMasks != nullptr);
				bool hasRHS = (gRHS != nullptr);

				multigrid_argcheck_(gInit, gMasks, gValues, gRHS, numGrids, minXSize, minYSize, maxXSize, maxYSize, curIndex);

				if (bufferSize < maxXSize * maxYSize * 3)
					throw NumericException(MultigridSolverError::OUT_OF_MEMORY);

				Elem * p1 = bufferPtr;
				Elem * p2 = p1 + maxXSize * maxYSize;
				Elem * p3 = p2 + maxXSize * maxYSize;

				GridAccessor2D<Elem> gs, gd;

				gs = gInit;

				uint64_t param = 0;
				const char * curCommand = command, *tmpCommand = command;
				MultigridSolverStep step = multigrid_decode_(curCommand, tmpCommand, param);				
				curCommand = tmpCommand;
				while (step != MultigridSolverStep::NONE)
				{
					Elem * pd = nullptr;
					if (gs.elements() == p1)
						pd = p2;
					else pd = p1;
					if (step == MultigridSolverStep::UP_SIMPLE)
					{						
						gd = GridAccessor2D<Elem>(pd, gs.xSize() * 2, gs.ySize() * 2);
						if (gd.xSize() > maxXSize || gd.ySize() > maxYSize)
							throw NumericException(MultigridSolverError::STEP_OUT_OF_SIZE_LIMIT);

						
						multigrid_prolong(gs, gd);
						gs = gd;

						curIndex++;
					}
					else if (step == MultigridSolverStep::DOWN_SIMPLE)
					{
						gd = GridAccessor2D<Elem>(pd, gs.xSize() / 2, gs.ySize() / 2);
						if (gd.xSize() < minXSize || gd.ySize() < minYSize)
							throw NumericException(MultigridSolverError::STEP_OUT_OF_SIZE_LIMIT);

						multigrid_restrict(gs, gd);
						gs = gd;

						curIndex--;
					}
					else if (step == MultigridSolverStep::JSMOOTH_SIMPLE)
					{
						gd = GridAccessor2D<Elem>(pd, gs.xSize(), gs.ySize());
						multigrid_jrelax_n(gd, gs, (hasMask ? &gValues[curIndex] : nullptr), (hasMask ? &gMasks[curIndex] : nullptr), (hasRHS ? &gRHS[curIndex] : nullptr), static_cast<uint32_t>(param));
						if ((param & 1) == 1)
							gs = gd;
					}

					step = multigrid_decode_(curCommand, tmpCommand, param);
					curCommand = tmpCommand;
				}

				gDst = gs;
			}
		}
	}
}
