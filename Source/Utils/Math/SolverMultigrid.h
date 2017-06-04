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
			inline size_t multigrid_required_size_setup(size_t xsize, size_t ysize)
			{
				/*1st grid -- gradient*/
				size_t gradientSize = xsize * ysize * sizeof(Elem) * 3;
				/*2nd grid -- rhs*/
				size_t rhsSize = xsize * ysize * sizeof(Elem);
				/*3rd grid -- constraint*/
				size_t constraintSize = xsize * ysize * sizeof(Elem);
				/*4th grid -- weight*/
				size_t weightSize = xsize * ysize * sizeof(Elem) * 2;
				return gradientSize + rhsSize + constraintSize + weightSize;
			}
			template<class Elem>
			inline size_t multigrid_required_size_run(size_t xsize, size_t ysize)
			{
				/*1st grid*/
				size_t gridSize = xsize * ysize * sizeof(Elem);
				return gridSize;
			}
			template<class Elem>
			inline size_t multigrid_required_size_run_full(size_t xsize, size_t ysize)
			{
				return 3 * multigrid_required_size_run<Elem>(xsize, ysize);
			}
			template<class Elem>
			inline size_t multigrid_required_size_setup(const size_t *xsizes, const size_t *ysizes, size_t num)
			{
				size_t res = 0;
				for (size_t i = 0; i < num; i++)
					res += multigrid_required_size_setup<Elem>(xsizes[i], ysizes[i]);
			}
			template<class Elem>
			inline size_t multigrid_required_size_setup_full(size_t xSize, size_t ySize)
			{
				size_t res = 0;
				while (xSize > 1 || ySize > 1)
				{
					res += multigrid_required_size_setup<Elem>(xSize, ySize);
					if (xSize == 1 && ySize == 1)
						break;
					if(xSize > 1)
						xSize = (xSize >> 1) + (xSize & 1);
					if(ySize > 1)
						ySize = (ySize >> 1) + (ySize & 1);
				}
				return res;
			}

			//returns actual numGrids in numGrids
			template<class WElem, class GElem, class Elem, class Dim>
			inline uint8_t * multigrid_setup(uint8_t * buff, size_t buffSize, size_t xSize, size_t ySize, Dim xMin, Dim xMax, Dim yMin, Dim yMax,
				GridAccessor2D<WElem, Dim> *gWeights, GridAccessor2D<GElem, Dim> *gGradients, GridAccessor2D<Elem, Dim> *gRHS, GridAccessor2D<Elem, Dim> *gConstraints,
				uint32_t &numGrids)
			{
				size_t mSize = (xSize > ySize ? xSize : ySize);
			
				size_t numSteps = 0;
				while (mSize > 0)
				{
					mSize >>= 1;
					numSteps++;
				}
				if (numGrids < numSteps || xSize == 0 || ySize == 0)
					throw NumericException(MultigridSolverError::ARGUMENT_ERROR);
				if (buffSize < multigrid_required_size_setup_full<Elem>(xSize, ySize))
					throw NumericException(MultigridSolverError::OUT_OF_MEMORY);

				size_t stepInd = 0;
				uint8_t * ptr = buff;
				size_t xSize0 = 1, ySize0 = 1;
				while (1)
				{
					gWeights[stepInd] = GridAccessor2D<WElem, Dim>(reinterpret_cast<WElem *>(ptr), xSize0, ySize0);
					ptr += sizeof(WElem) * xSize0 * ySize0;
					fillGrid(gWeights[stepInd], WElem(Elem(0), Elem(0)));

					gGradients[stepInd] = GridAccessor2D<GElem, Dim>(reinterpret_cast<GElem *>(ptr), xSize0, ySize0);
					ptr += sizeof(GElem) * xSize0 * ySize0;

					gRHS[stepInd] = GridAccessor2D<Elem, Dim>(reinterpret_cast<Elem *>(ptr), xSize0, ySize0);
					ptr += sizeof(Elem) * xSize0 * ySize0;
					fillGrid(gRHS[stepInd], Elem(0));

					gConstraints[stepInd] = GridAccessor2D<Elem, Dim>(reinterpret_cast<Elem *>(ptr), xSize0, ySize0);
					ptr += sizeof(Elem) * xSize0 * ySize0;

					gWeights[stepInd].setDimensions(xMin, xMax, yMin, yMax);
					gGradients[stepInd].setDimensions(xMin, xMax, yMin, yMax);
					gRHS[stepInd].setDimensions(xMin, xMax, yMin, yMax);
					gConstraints[stepInd].setDimensions(xMin, xMax, yMin, yMax);
					
					if (xSize == xSize0 && ySize == ySize0)
						break;
					xSize0 <<= 1;
					ySize0 <<= 1;
					if (xSize0 > xSize)
						xSize0 = xSize;
					if (ySize0 > ySize)
						ySize0 = ySize;
					stepInd++;
				}
				numGrids = stepInd + 1;
				return ptr;
			}

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

			/*
			//OLD
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
			*/
			template<class IndT, class ElemT>
			inline IndT multigrid_update_index_by_gradient_(ElemT g, IndT ind, IndT maxValue)
			{
				if (g > ElemT(0))
				{
					ind++;
					if (ind > maxValue)
						return maxValue;
					return ind;
				}
				else if(g < ElemT(0))
				{
					if (ind == 0)
						return 0;
					return ind - 1;
				}
				return ind;
			}

			//w.x -- weight of gradient, w.y -- weight of value-constraint, 1 - w.x - w.y -- Laplace/Poisson equation
			template<class WElem, class GElem, class Elem, class Dim>
			bool multigrid_jrelax(GridAccessor2D<Elem, Dim> &gDst, const GridAccessor2D<Elem, Dim> &gSrc,
				const GridAccessor2D<WElem, Dim> * gWeight /*can be nullptr, hence w.x=w.y=0*/,
				const GridAccessor2D<GElem, Dim> * gGradient /*can be nullptr if w.x always 0*/,
				const GridAccessor2D<Elem, Dim> *gConstraint /*can be nullptr if w.y always 0*/,
				const GridAccessor2D<Elem, Dim> *gRHS /*can be nullptr -- then RHS=0*/,
				Elem * error = nullptr)
			{
				bool hasWeight = (gWeight != nullptr);
				bool hasRHS = (gRHS != nullptr);

				if (gSrc.xSize() <= 1 || gSrc.ySize() <= 1)
				{
					for (GridAccessor2D<Elem, Dim>::SizeType yi = 0; yi < gSrc.ySize(); yi++)
						for (GridAccessor2D<Elem, Dim>::SizeType xi = 0; xi < gSrc.xSize(); xi++)
						{
							if (hasWeight && gWeight->get(xi, yi).x > Elem(0))
								gDst.get(xi, yi) = gConstraint->get(xi, yi);
							else
								gDst.get(xi, yi) = gSrc.get(xi, yi);
							if (error)
								*error += abs(gDst(xi, yi) - gSrc(xi, yi));
						}
					return true;
				}
				Elem xh = gDst.xStep();
				Elem yh = gDst.yStep();
				GridAccessor2D<Elem, Dim>::SizeType xSize = gDst.xSize();
				GridAccessor2D<Elem, Dim>::SizeType ySize = gDst.ySize();

				Elem xCoef = Elem(0.5)*yh*yh / (xh*xh + yh*yh);
				Elem yCoef = Elem(0.5)*xh*xh / (xh*xh + yh*yh);
				Elem fCoef = Elem(0.5)*xh*xh*yh*yh / (xh*xh + yh*yh);

				Elem maxUpdate = 0;
				Elem sumUpdate = 0, sumAbsUpdate = 0;
				
				
				for (GridAccessor2D<Elem, Dim>::SizeType yi = 0; yi < ySize; yi++)
					for (GridAccessor2D<Elem, Dim>::SizeType xi = 0; xi < xSize; xi++)
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

						//Laplace/Poisson part
						Elem ff = fx * xCoef + fy * yCoef;

						if (hasRHS)
							ff += gRHS->get(xi, yi)*fCoef;

						if (hasWeight)
						{
							const WElem &w = gWeight->get(xi, yi);
							Elem res = Elem(0);

							if (w.y > Elem(0))
								res += w.y * gConstraint->get(xi, yi);

							if (w.x > Elem(0))
							{
								const GElem &g = gGradient->get(xi, yi);
								auto gxi = multigrid_update_index_by_gradient_(g.x, xi, xSize - 1);
								auto gyi = multigrid_update_index_by_gradient_(g.y, yi, ySize - 1);
								auto gx = gSrc(gxi, yi);
								auto gy = gSrc(xi, gyi);
								auto gPrev = gx * g.x * g.x + gy * g.y * g.y;
								auto gStep = xh * g.x * g.x + yh * g.y * g.y;
								res += w.x * (gPrev + gStep * g.z);
							}

							gDst(xi, yi) = res + (Elem(1) - w.x - w.y) * ff;
						}
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

			template<class WElem, class GElem, class Elem, class Dim>
			bool multigrid_jrelax_n(GridAccessor2D<Elem, Dim> &gDst, GridAccessor2D<Elem, Dim> &gSrc,
				const GridAccessor2D<WElem, Dim> * gWeight /*can be nullptr, hence w.x=w.y=0*/,
				const GridAccessor2D<GElem, Dim> * gGradient /*can be nullptr if w.x always 0*/,
				const GridAccessor2D<Elem, Dim> *gConstraint /*can be nullptr if w.y always 0*/,
				const GridAccessor2D<Elem, Dim> *gRHS /*can be nullptr -- then RHS=0*/,
				uint32_t nStep, Elem * error = nullptr)
			{
				for (uint32_t i = 0; i < nStep; i++)
				{
					multigrid_jrelax(gDst, gSrc, gWeight, gGradient, gConstraint, gRHS, error);
					gSrc.swap(gDst);
				}
				return true;
			}

			/*at least one of following arguments should be filled: gRHS or gWeight, otherwise solution is trivial*/
			template<class WElem, class GElem, class Elem, class Dim>
			inline void multigrid_argcheck_(const GridAccessor2D<Elem, Dim> * gInit /*can be nullptr, hence initial value is zero*/,
				const GridAccessor2D<WElem, Dim> * gWeights /*can be nullptr, hence w.x=w.y=0*/,
				const GridAccessor2D<GElem, Dim> * gGradients /*can be nullptr if w.x always 0*/,
				const GridAccessor2D<Elem, Dim> * gConstraints /*can be nullptr if w.y always 0*/,
				const GridAccessor2D<Elem, Dim> * gRHS /*can be nullptr -- then RHS=0*/,
				uint32_t numGrids,
				uint32_t &minXSize, uint32_t &minYSize, uint32_t &maxXSize, uint32_t &maxYSize, uint32_t &initInd)
			{
				initInd = 0xFFFFFFFF;
				uint32_t smallDimIndX = 0, smallDimIndY = 0;

				minXSize = 0xFFFFFFFF; maxXSize = 0;
				minYSize = 0xFFFFFFFF; maxYSize = 0;

				bool hasWeight = (gWeights != nullptr);
				bool hasRHS = (gRHS != nullptr);

				if (!hasWeight && !hasRHS)
					throw NumericException(MultigridSolverError::ARGUMENT_ERROR);

				uint32_t prevXSize = 0, prevYSize = 0;

				/*check sizes*/
				for (uint32_t i = 0; i < numGrids; i++)
				{
					uint32_t xSize = 0, ySize = 0;
					if (hasWeight)
					{
						if(gConstraints &&
							((gWeights[i].xSize() != gConstraints[i].xSize())
								|| (gWeights[i].ySize() != gConstraints[i].ySize())
								))
							throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
						if (gGradients &&
							((gWeights[i].xSize() != gGradients[i].xSize())
								|| (gWeights[i].ySize() != gGradients[i].ySize())
								))
							throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
						if (gRHS &&
							((gWeights[i].xSize() != gRHS[i].xSize())
								|| (gWeights[i].ySize() != gRHS[i].ySize())
								))
							throw NumericException(MultigridSolverError::ARGUMENT_DIMENSION_MISMATCH);
						xSize = gWeights[i].xSize();
						ySize = gWeights[i].ySize();
					}else if (hasRHS)
					{
						xSize = gRHS[i].xSize();
						ySize = gRHS[i].ySize();
					}

					if (i > 0)
					{
						if (prevXSize * 2 != xSize || prevYSize * 2 != ySize)
							throw NumericException(MultigridSolverError::STEP_SIZE_NOT_SUPPORTED);
					}

					if (gInit && xSize == gInit->xSize() && ySize == gInit->ySize())
						initInd = i;
					
					if (xSize < minXSize)
					{
						smallDimIndX = i;
						minXSize = xSize;
					}
					if (xSize > maxXSize)
						maxXSize = xSize;

					if (ySize < minYSize)
					{
						smallDimIndY = i;
						minYSize = ySize;
					}
					if (ySize > maxYSize)
						maxYSize = ySize;

					prevXSize = xSize; prevYSize = ySize;
				}
				if(smallDimIndX != smallDimIndY)
					throw NumericException(MultigridSolverError::STEP_SIZE_NOT_SUPPORTED);
				if (!gInit)
					initInd = smallDimIndX;
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

			template<class WElem, class GElem, class Elem, class Dim>
			void multigrid_run(GridAccessor2D<Elem, Dim> &gDst, void * bufferPtr, const uint64_t bufferSize, const char * command,
				const GridAccessor2D<Elem, Dim> * gInit /*can be nullptr, hence initial value is zero, and index with smallest dimension is chosen as starting*/,
				const GridAccessor2D<WElem, Dim> * gWeights /*can be nullptr, hence w.x=w.y=0*/,
				const GridAccessor2D<GElem, Dim> * gGradients /*can be nullptr if w.x always 0*/,
				const GridAccessor2D<Elem, Dim> * gConstraints /*can be nullptr if w.y always 0*/,
				const GridAccessor2D<Elem, Dim> * gRHS /*can be nullptr -- then RHS=0*/,
				uint32_t numGrids)
			{
				uint32_t curIndex = 0;
				uint32_t minXSize, maxXSize, minYSize, maxYSize;

				bool hasInit = (gInit != nullptr);
				bool hasWeight = (gWeights != nullptr);
				bool hasRHS = (gRHS != nullptr);

				multigrid_argcheck_(gInit, gWeights, gGradients, gConstraints, gRHS, numGrids, minXSize, minYSize, maxXSize, maxYSize, curIndex);

				if (bufferSize < maxXSize * maxYSize * sizeof(Elem) * 3)
					throw NumericException(MultigridSolverError::OUT_OF_MEMORY);

				Elem * p1 = static_cast<Elem *>(bufferPtr);
				Elem * p2 = p1 + maxXSize * maxYSize;
				Elem * p3 = p2 + maxXSize * maxYSize;

				GridAccessor2D<Elem, Dim> gs, gd;

				if(hasInit)
					gs = *gInit;
				else
				{
					gs = GridAccessor2D<Elem, Dim>(p2, minXSize, minYSize);
					if (hasWeight)
						gs.setDimensions(gWeights[curIndex].getDimensions());
					else
						gs.setDimensions(gRHS[curIndex].getDimensions());

					for (GridAccessor2D<Elem, Dim>::SizeType yi = 0; yi < gs.ySize(); yi++)
						for (GridAccessor2D<Elem, Dim>::SizeType xi = 0; xi < gs.xSize(); xi++)
							gs.get(xi, yi) = Elem(0);
				}

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
						gd = GridAccessor2D<Elem, Dim>(pd, gs.xSize() * 2, gs.ySize() * 2);
						gd.setDimensions(gs.getDimensions());
						if (gd.xSize() > maxXSize || gd.ySize() > maxYSize)
							throw NumericException(MultigridSolverError::STEP_OUT_OF_SIZE_LIMIT);

						
						multigrid_prolong(gs, gd);
						gs = gd;

						curIndex++;
					}
					else if (step == MultigridSolverStep::DOWN_SIMPLE)
					{
						gd = GridAccessor2D<Elem>(pd, gs.xSize() / 2, gs.ySize() / 2);
						gd.setDimensions(gs.getDimensions());
						if (gd.xSize() < minXSize || gd.ySize() < minYSize)
							throw NumericException(MultigridSolverError::STEP_OUT_OF_SIZE_LIMIT);

						multigrid_restrict(gs, gd);
						gs = gd;

						curIndex--;
					}
					else if (step == MultigridSolverStep::JSMOOTH_SIMPLE)
					{
						gd = GridAccessor2D<Elem>(pd, gs.xSize(), gs.ySize());
						gd.setDimensions(gs.getDimensions());
						multigrid_jrelax_n(gd, gs,
							gWeights ? &gWeights[curIndex] : nullptr,
							gGradients ? &gGradients[curIndex] : nullptr,
							gConstraints ? &gConstraints[curIndex] : nullptr,
							gRHS ? &gRHS[curIndex] : nullptr,
							static_cast<uint32_t>(param));
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
