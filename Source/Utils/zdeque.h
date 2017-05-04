#pragma once
#include "alloc_config.h"
#include <deque>

namespace zenith
{
	template<class T>
	using zdeque = std::deque<T, util::zstdAllocatorMainSTL<T>>;

	template<class T>
	using zdeque_loc = std::deque<T, util::zstdAllocatorLocalSTL<T>>;
}