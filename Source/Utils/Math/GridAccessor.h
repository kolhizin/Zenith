#pragma once

#include "Numeric.h"

namespace zenith
{
	namespace util
	{
		namespace math
		{

			template<class ElementType>
			class GridAccessor2D
			{
			public:
				typedef ElementType ElementType;
				typedef uint32_t SizeType;
			private:
				union
				{
					ElementType * data_;
					uint8_t * bytes_;
				};
				SizeType xSize_, ySize_, xStride_, yStride_;
				ElementType xMin_ = ElementType(0), xMax_ = ElementType(1), yMin_ = ElementType(0), yMax_ = ElementType(1);

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

					xMin_ = yMin_ = ElementType(0);
					xMax_ = yMax_ = ElementType(1);
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
					xMin_ = ga.xMin_;
					xMax_ = ga.xMax_;
					yMin_ = ga.yMin_;
					yMax_ = ga.yMax_;
				}
				inline GridAccessor2D(GridAccessor2D<ElementType> &&ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					xMin_ = ga.xMin_;
					xMax_ = ga.xMax_;
					yMin_ = ga.yMin_;
					yMax_ = ga.yMax_;
					ga.reset_();
				}
				inline const GridAccessor2D<ElementType> &operator =(const GridAccessor2D<ElementType> &ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					xMin_ = ga.xMin_;
					xMax_ = ga.xMax_;
					yMin_ = ga.yMin_;
					yMax_ = ga.yMax_;
					return *this;
				}
				inline const GridAccessor2D<ElementType> &operator =(GridAccessor2D<ElementType> &&ga)
				{
					reset_(ga.data_, ga.xSize_, ga.ySize_, ga.xStride_, ga.yStride_);
					xMin_ = ga.xMin_;
					xMax_ = ga.xMax_;
					yMin_ = ga.yMin_;
					yMax_ = ga.yMax_;
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

				inline void setDimensions(ElementType xMin = ElementType(0), ElementType xMax = ElementType(1), ElementType yMin = ElementType(0), ElementType yMax = ElementType(1))
				{
					xMin_ = xMin;
					yMin_ = yMin;
					xMax_ = xMax;
					yMax_ = yMax;
				}

				inline SizeType xSize() const { return xSize_; }
				inline SizeType xStride() const { return xStride_; }
				inline SizeType ySize() const { return ySize_; }
				inline SizeType yStride() const { return yStride_; }

				inline ElementType xMin() const { return xMin_; }
				inline ElementType xMax() const { return xMax_; }
				inline ElementType yMin() const { return yMin_; }
				inline ElementType yMax() const { return yMax_; }

				inline ElementType xStep() const { return (xMax_ - xMin_) / (xSize_ - 1); }
				inline ElementType yStep() const { return (yMax_ - yMin_) / (ySize_ - 1); }

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

				inline void swap(GridAccessor2D<ElementType> &ga)
				{
					std::swap(data_, ga.data_);
					std::swap(xSize_, ga.xSize_);
					std::swap(ySize_, ga.ySize_);
					std::swap(xStride_, ga.xStride_);
					std::swap(yStride_, ga.yStride_);
					std::swap(xMin_, ga.xMin_);
					std::swap(xMax_, ga.xMax_);
					std::swap(yMin_, ga.yMin_);
					std::swap(yMax_, ga.yMax_);
				}
			};

			template<class ElementType, class Function>
			void evaluateGrid(GridAccessor2D<ElementType> &grid, Function &&f)
			{
				ElementType xh = grid.xStep(), x0 = grid.xMin();
				ElementType yh = grid.yStep(), y0 = grid.yMin();
				GridAccessor2D<ElementType>::SizeType xL = grid.xSize();
				GridAccessor2D<ElementType>::SizeType yL = grid.ySize();
				ElementType x, y = 0;
				for (GridAccessor2D<ElementType>::SizeType yi = 0; yi < yL; yi++)
				{			
					x = 0;
					for (GridAccessor2D<ElementType>::SizeType xi = 0; xi < xL; xi++)
					{
						grid(xi, yi) = f(x, y, xh, yh);
						x += xh;
					}
					y += yh;
				}
			}

			template<class ElementType>
			void fillGrid(GridAccessor2D<ElementType> &grid, const ElementType &e)
			{
				GridAccessor2D<ElementType>::SizeType xL = grid.xSize();
				GridAccessor2D<ElementType>::SizeType yL = grid.ySize();
				for (GridAccessor2D<ElementType>::SizeType yi = 0; yi < yL; yi++)
					for (GridAccessor2D<ElementType>::SizeType xi = 0; xi < xL; xi++)
						grid(xi, yi) = e;
			}

		}
	}
}