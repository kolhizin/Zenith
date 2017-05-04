#pragma once
#include "alloc_config.h"
#include <vector>

namespace zenith
{
	template<class T>
	using zvector = std::vector<T, util::zstdAllocatorMainSTL<T>>;

	template<class T>
	using zvector_loc = std::vector<T, util::zstdAllocatorLocalSTL<T>>;
}