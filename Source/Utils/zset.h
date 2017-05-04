#pragma once
#include "alloc_config.h"
#include <set>

namespace zenith
{
	template<class T, class Compare = std::less<T>>
	using zset = std::set<T, Compare, util::zstdAllocatorMainSTL<T>>;

	template<class T, class Compare = std::less<T>>
	using zset_loc = std::set<T, Compare, util::zstdAllocatorLocalSTL<T>>;
}