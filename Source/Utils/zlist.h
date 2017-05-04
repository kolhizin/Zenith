#pragma once
#include "alloc_config.h"
#include <list>

namespace zenith
{
	template<class T>
	using zlist = std::list<T, util::zstdAllocatorMainSTL<T>>;

	template<class T>
	using zlist_loc = std::list<T, util::zstdAllocatorLocalSTL<T>>;
}