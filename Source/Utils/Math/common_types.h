#pragma once

namespace zenith
{
	namespace util
	{
		namespace math
		{
			template<class Val, class Prob = double>
			class ValueProbability
			{
			public:
				Val value;
				Prob prob;
			};

			template<class Elem>
			class Box2D
			{
			public:
				Elem xMin, xMax, yMin, yMax;
				inline Elem xSize() const { return xMax - xMin; }
				inline Elem ySize() const { return yMax - yMin; }
			};

			template<class Elem>
			class Box3D
			{
			public:
				Elem xMin, xMax, yMin, yMax, zMin, zMax;
				inline Elem xSize() const { return xMax - xMin; }
				inline Elem ySize() const { return yMax - yMin; }
				inline Elem zSize() const { return zMax - zMin; }
			};
		}
	}
}