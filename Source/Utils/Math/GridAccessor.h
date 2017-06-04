#pragma once

#include "Numeric.h"
#include "common_types.h"

namespace zenith
{
	namespace util
	{
		namespace math
		{

			template<class ElementType, class GridDimensionType = double>
			class GridAccessor2D
			{
			public:
				typedef ElementType ElementType;
				typedef GridDimensionType GridDimensionType;
				typedef Box2D<GridDimensionType> DimensionBoxType;
				typedef uint32_t SizeType;
			private:
				union
				{
					ElementType * data_;
					uint8_t * bytes_;
				};
				SizeType xSize_, ySize_, xStride_, yStride_;
				DimensionBoxType dimBox_ = { GridDimensionType(0), GridDimensionType(1),
					GridDimensionType(0), GridDimensionType(1) };

				inline void reset_(ElementType * data = nullptr, SizeType xSize = 0, SizeType ySize = 0, SizeType xStride = sizeof(ElementType), SizeType yStride = 0)
				{
					data_ = data;
					xSize_ = xSize;
					ySize_ = ySize;
					xStride_ = (data ? xStride : 0);
					if (yStride > 0)
						yStride_ = yStride;
					else
						yStride_ = (data && (xStride > 0) && (xSize > 0) ? xStride * xSize : 0);

					
					dimBox_.xMin = dimBox_.yMin = GridDimensionType(0);
					dimBox_.xMax = dimBox_.yMax = GridDimensionType(1);
				}
				inline ElementType * addr_(SizeType x, SizeType y) { return reinterpret_cast<ElementType *>(bytes_ + y * yStride_ + x * xStride_); }
				inline const ElementType * addr_(SizeType x, SizeType y) const { return reinterpret_cast<const ElementType *>(bytes_ + y * yStride_ + x * xStride_); }

				inline ElementType * get_(SizeType x, SizeType y) { return (validPoint(x, y) ? addr_(x, y) : nullptr); }
				inline const ElementType * get_(SizeType x, SizeType y) const { return (validPoint(x, y) ? addr_(x, y) : nullptr); }
			public:
				inline GridAccessor2D(ElementType * data = nullptr, SizeType xSize = 0, SizeType ySize = 0, SizeType xStride = sizeof(ElementType), SizeType yStride = 0)
				{
					reset_(data, xSize, ySize, xStride, yStride);
				}
				inline GridAccessor2D(const GridAccessor2D<ElementType> &ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					dimBox_ = ga.dimBox_;
				}
				inline GridAccessor2D(GridAccessor2D<ElementType> &&ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					dimBox_ = ga.dimBox_;
					ga.reset_();
				}
				inline const GridAccessor2D<ElementType> &operator =(const GridAccessor2D<ElementType> &ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					dimBox_ = ga.dimBox_;
					return *this;
				}
				inline const GridAccessor2D<ElementType> &operator =(GridAccessor2D<ElementType> &&ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					dimBox_ = ga.dimBox_;
					ga.reset_();
					return *this;
				}

				inline void setSize(SizeType xSize, SizeType ySize)
				{
					xSize_ = xSize;
					ySize_ = ySize;
					if (yStride_ == 0)
						yStride_ = xSize_ * xStride_;
				}
				inline void setXStride(SizeType xStride)
				{
					xStride_ = xStride;
				}
				inline void setYStride(SizeType yStride)
				{
					yStride_ = yStride;
				}	

				inline void setDimensions(GridDimensionType xMin = GridDimensionType(0), GridDimensionType xMax = GridDimensionType(1),
					GridDimensionType yMin = GridDimensionType(0), GridDimensionType yMax = GridDimensionType(1))
				{					
					dimBox_.xMin = xMin;
					dimBox_.yMin = yMin;
					dimBox_.xMax = xMax;
					dimBox_.yMax = yMax;
				}

				inline void setDimensions(Box2D<GridDimensionType> gridDim)
				{
					dimBox_ = gridDim;
				}

				inline SizeType xSize() const { return xSize_; }
				inline SizeType xStride() const { return xStride_; }
				inline SizeType ySize() const { return ySize_; }
				inline SizeType yStride() const { return yStride_; }

				inline const DimensionBoxType &getDimensions() const { return dimBox_; }
				inline GridDimensionType xMin() const { return dimBox_.xMin; }
				inline GridDimensionType xMax() const { return dimBox_.xMax; }
				inline GridDimensionType yMin() const { return dimBox_.yMin; }
				inline GridDimensionType yMax() const { return dimBox_.yMax; }

				inline GridDimensionType xStep() const { return (dimBox_.xMax - dimBox_.xMin) / (xSize_ - 1); }
				inline GridDimensionType yStep() const { return (dimBox_.yMax - dimBox_.yMin) / (ySize_ - 1); }

				inline GridDimensionType getX(SizeType x) const { return dimBox_.xMin + (xSize_ > 1 ? xStep() * x : dimBox_.xSize() * GridDimensionType(0.5)); }
				inline GridDimensionType getY(SizeType y) const { return dimBox_.yMin + (ySize_ > 1 ? yStep() * y : dimBox_.ySize() * GridDimensionType(0.5)); }

				inline bool validLinear() const { return xStride_ > 0; }
				inline bool validGrid() const { return (yStride_ > 0) && (xStride_ > 0); }
				inline bool validPoint(SizeType x, SizeType y) const { return (x < xSize_) && (y < ySize_); }

				inline ElementType &operator ()(SizeType x, SizeType y) { return *addr_(x, y); }
				inline const ElementType &operator ()(SizeType x, SizeType y) const { return *addr_(x, y); }

				inline ElementType &get(SizeType x, SizeType y) { return *get_(x, y); }
				inline const ElementType &get(SizeType x, SizeType y) const { return *get_(x, y); }

				template<class T>
				inline SizeType xClamp(T x) const { if (x < 0)return 0; if (x >= xSize_) return xSize_ - 1;return x; }
				template<class T>
				inline SizeType yClamp(T y) const { if (y < 0)return 0; if (y >= xSize_) return ySize_ - 1;return y; }


				inline void * data() { return bytes_; }
				inline void * data() const { return bytes_; }
				
				inline uint8_t * bytes() { return bytes_; }
				inline const uint8_t * bytes() const { return bytes_; }

				inline ElementType * elements() { return data_; }
				inline const ElementType * elements() const { return data_; }

				inline void swap(GridAccessor2D<ElementType, GridDimensionType> &ga)
				{
					std::swap(data_, ga.data_);
					std::swap(xSize_, ga.xSize_);
					std::swap(ySize_, ga.ySize_);
					std::swap(xStride_, ga.xStride_);
					std::swap(yStride_, ga.yStride_);
					std::swap(dimBox_, ga.dimBox_);
				}
			};

			template<class ElementType, class GridDimType, class Function>
			void evaluateGrid(GridAccessor2D<ElementType, GridDimType> &grid, Function &&f)
			{
				ElementType xh = grid.xStep(), x0 = grid.xMin();
				ElementType yh = grid.yStep(), y0 = grid.yMin();
				GridAccessor2D<ElementType, GridDimType>::SizeType xL = grid.xSize();
				GridAccessor2D<ElementType, GridDimType>::SizeType yL = grid.ySize();
				ElementType x, y = 0;
				for (GridAccessor2D<ElementType, GridDimType>::SizeType yi = 0; yi < yL; yi++)
				{			
					x = 0;
					for (GridAccessor2D<ElementType, GridDimType>::SizeType xi = 0; xi < xL; xi++)
					{
						grid(xi, yi) = f(x, y, xh, yh);
						x += xh;
					}
					y += yh;
				}
			}

			template<class ElementType, class GridDimType>
			void fillGrid(GridAccessor2D<ElementType, GridDimType> &grid, const ElementType &e)
			{
				GridAccessor2D<ElementType, GridDimType>::SizeType xL = grid.xSize();
				GridAccessor2D<ElementType, GridDimType>::SizeType yL = grid.ySize();
				for (GridAccessor2D<ElementType, GridDimType>::SizeType yi = 0; yi < yL; yi++)
					for (GridAccessor2D<ElementType, GridDimType>::SizeType xi = 0; xi < xL; xi++)
						grid(xi, yi) = e;
			}

		}
	}
}