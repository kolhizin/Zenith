#pragma once

#include "../NodeGenerator.h"
#include <glm\glm.hpp>
#include <Utils\Math\GridAccessor.h>

namespace zenith
{
	namespace terragen
	{
		class TerraGenTransformException : public TerraGenException
		{
		public:
			TerraGenTransformException() : TerraGenException() {}
			TerraGenTransformException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(p, type)
			{
			}
			TerraGenTransformException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : TerraGenException(str, type)
			{
			}
			virtual ~TerraGenTransformException() {}
		};


		template<class T>
		T normalizeWeight(const T &w)
		{
			T res = w;
			double n = w.x + w.y + w.z;
			if (n < 1e-7)
			{
				res.x = res.y = res.z = 0.0;
				return res;
			}
			res.x /= n;
			res.y /= n;
			res.z /= n;
			return res;
		}

		template<class T>
		T weightedMix(const T &t1, const T &t2, double w1, double w2)
		{
			if (w1 + w2 < 1e-7)
				return T();
			return (t1 * w1 + t2 * w2) / (w1 + w2);
		}
		

		void transformNodeToMGS(const std::vector<const BaseNode *> &nodes,
			util::math::GridAccessor2D<glm::dvec4> &weights,
			util::math::GridAccessor2D<glm::dvec3> &gradients,
			util::math::GridAccessor2D<double> &rhs,
			util::math::GridAccessor2D<double> &constraint);

		template<class WElem, class GElem, class Elem, class Dim> 
		inline void transformNodeToMGS_1level(const std::vector<const BaseNode *> &nodes,
			util::math::GridAccessor2D<WElem, Dim> &gWeights,
			util::math::GridAccessor2D<GElem, Dim> &gGradients,
			util::math::GridAccessor2D<Elem, Dim> &gConstraints,
			util::math::GridAccessor2D<Elem, Dim> &gRHS,
			uint8_t * wrkBuff = nullptr, uint32_t wrkBuffSize = 0)
		{
			auto xSize = gWeights.xSize();
			auto ySize = gWeights.ySize();
			util::math::Box2D<double> gridDim = gWeights.getDimensions();

			size_t reqSize0 = xSize * ySize;
			size_t reqSize = reqSize0 * (4 + 3 + 1 + 1) * sizeof(double);
			std::unique_ptr<uint8_t[]> ownBuffer;
			if (wrkBuff)
			{
				if (wrkBuffSize < reqSize)
					throw TerraGenTransformException("transformNodeToMGS(): supplied buffer is not big enough for operation!");
			}
			else
			{
				wrkBuffSize = reqSize;
				ownBuffer.reset(new uint8_t[reqSize]);
				wrkBuff = ownBuffer.get();
			}

			double * dPtr = reinterpret_cast<double *>(wrkBuff);

			util::math::GridAccessor2D<glm::dvec4, double> wWeights(reinterpret_cast<glm::dvec4 *>(dPtr), xSize, ySize);
			dPtr += xSize * ySize * 4;

			util::math::GridAccessor2D<glm::dvec3, double> wGradients(reinterpret_cast<glm::dvec3 *>(dPtr), xSize, ySize);
			dPtr += xSize * ySize * 3;

			util::math::GridAccessor2D<double, double> wConstraints(dPtr, xSize, ySize);
			dPtr += xSize * ySize;

			util::math::GridAccessor2D<double, double> wRHS(dPtr, xSize, ySize);
			dPtr += xSize * ySize;

			wWeights.setDimensions(gridDim);
			wGradients.setDimensions(gridDim);
			wConstraints.setDimensions(gridDim);
			wRHS.setDimensions(gridDim);

			transformNodeToMGS(nodes, wWeights, wGradients, wRHS, wConstraints);

			for(uint32_t yi = 0; yi < wWeights.ySize(); yi++)
				for (uint32_t xi = 0; xi < wWeights.xSize(); xi++)
				{
					auto &gWeight = gWeights.get(xi, yi);
					const auto &wWeight = wWeights.get(xi, yi);

					if (wWeight.w > 0.0)
					{
						auto tWeight = normalizeWeight(wWeight);
						gWeight = WElem(Elem(tWeight.x), Elem(tWeight.z));
						
						gGradients.get(xi, yi) = (tWeight.x > Elem(0) ?
							GElem(wGradients.get(xi, yi)) : GElem(Elem(0), Elem(0), Elem(0)));

						gRHS.get(xi, yi) = (tWeight.y > Elem(0) ? wRHS.get(xi, yi) : Elem(0));

						gConstraints.get(xi, yi) = (tWeight.z > Elem(0) ? wConstraints.get(xi, yi) : Elem(0));
					}
					else
					{
						gWeight = WElem(Elem(0), Elem(0));

						gGradients.get(xi, yi) = GElem(Elem(0), Elem(0), Elem(0));
						gConstraints.get(xi, yi) = Elem(0);
						gRHS.get(xi, yi) = Elem(0);
					}
				}
		}

		template<class WElem, class GElem, class Elem, class Dim>
		inline void transformNodeToMGSsetup(const std::vector<const BaseNode *> &nodes,
			util::math::GridAccessor2D<WElem, Dim> *gWeights,
			util::math::GridAccessor2D<GElem, Dim> *gGradients,
			util::math::GridAccessor2D<Elem, Dim> *gConstraints,
			util::math::GridAccessor2D<Elem, Dim> *gRHS,
			uint32_t numGrids, uint8_t * wrkBuff = nullptr, uint32_t wrkBuffSize = 0)
		{
			auto xSize = gWeights[0].xSize();
			auto ySize = gWeights[0].ySize();
			size_t reqSize0 = xSize * ySize;
			size_t reqSize = reqSize0 * (4 + 3 + 1 + 1) * sizeof(double);
			std::unique_ptr<uint8_t[]> ownBuffer;
			if (wrkBuff)
			{
				if (wrkBuffSize < reqSize)
					throw TerraGenTransformException("transformNodeToMGS(): supplied buffer is not big enough for operation!");
			}
			else
			{
				wrkBuffSize = reqSize;
				ownBuffer.reset(new uint8_t[reqSize]);
				wrkBuff = ownBuffer.get();
			}

			for (uint32_t i = 0; i < numGrids; i++)
				transformNodeToMGS_1level(nodes, gWeights[i], gGradients[i], gConstraints[i], gRHS[i], wrkBuff, wrkBuffSize);
		}
	}
}