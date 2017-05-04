#pragma once
#include <cstdint>
namespace zenith
{
	namespace util
	{
		template<class BitMask, class Collection>
		BitMask collection2bitmask(const Collection &coll)
		{
			uint64_t res = 0;
			for (const auto &x : coll)
				res |= static_cast<uint64_t>(x);
			return static_cast<BitMask>(res);
		}

		template<class Collection, class BitMask>
		Collection bitmask2collection(BitMask bm)
		{
			uint64_t curVal = 1;
			Collection res;
			while (bm)
			{
				if (bm & 1)
					res.push_back(static_cast<typename Collection::value_type>(curVal));
				curVal <<= 1;
				bm >>= 1;
			}
			return res;
		}
	}
}