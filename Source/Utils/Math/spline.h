#pragma once

namespace zenith
{
	namespace math
	{
		/*
		several functions for splines:
		1) Interpolate
		2) Rasterize
		*/

		template<class PointType, class FloatType>
		void interpolateSpline_CatmullRom(const PointType * controlPoints, const size_t numControlPoints, /*input: spline definition*/
			const FloatType * parameterPoints, const size_t numParameterPoints, /*input: parameter points*/
			PointType * splinePoints /*output: interpolated points (number of points -- in numParameterPoints)*/
		);
		
		template<class PointType, class FloatType>
		void rasterizeSpline_CatmullRom(const PointType * controlPoints, const size_t numControlPoints, /*input: spline definition*/
			const size_t numPoints, /*input: parameter points*/
			PointType * points /*output: interpolated points (number of points -- in numParameterPoints)*/
		);
	}
}