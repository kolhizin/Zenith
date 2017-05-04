#pragma once
#include "alloc_config.h"
#include <map>


namespace zenith
{
	template<class Key, class T, class Compare = std::less<T>>
	using zmultimap = std::multimap<T, Compare, util::zstdAllocatorMainSTL<std::pair<const Key, T>>>;

	template<class Key, class T, class Compare = std::less<T>>
	using zmultimap_loc = std::multimap<T, Compare, util::zstdAllocatorLocalSTL<std::pair<const Key, T>>>;

	template<class Key, class T, class Compare = std::less<T>>
	using zmap = std::multimap<T, Compare, util::zstdAllocatorMainSTL<std::pair<const Key, T>>>;

	template<class Key, class T, class Compare = std::less<T>>
	using zmap_loc = std::multimap<T, Compare, util::zstdAllocatorLocalSTL<std::pair<const Key, T>>>;
}