#pragma once
#include <stdexcept>

namespace zenith
{
	namespace util
	{
		template<size_t N>
		void zstrcpy(char(&dst)[N], const char * src)
		{
			size_t n = 0;
			while (*src && n < N)
			{
				dst[n] = *src;
				n++;
				src++;
			}
			if(n >= N)
				throw std::out_of_range("zstrcpy failed!");
			dst[n] = '\0';
		}
	}
}
