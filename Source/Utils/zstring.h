#pragma once
#include "alloc_config.h"
#include <string>

namespace zenith
{
	template<class charT, class traits = std::char_traits<charT>>
	using zstring_basic = std::basic_string<charT, traits, util::zstdAllocatorMainSTL<charT>>;

	template<class charT, class traits = std::char_traits<charT>>
	using zstring_loc_basic = std::basic_string<charT, traits, util::zstdAllocatorLocalSTL<charT>>;

	using zstring = zstring_basic<char>;
	using zwstring = zstring_basic<wchar_t>;

	using zstring_loc = zstring_loc_basic<char>;
	using zwstring_loc = zstring_loc_basic<wchar_t>;
}
