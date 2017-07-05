#pragma once
#include <stdexcept>

namespace zenith
{
	namespace util
	{
		template<size_t N>
		inline void zstrcpy(char(&dst)[N], const char * src)
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
		inline void zstrcpy(char *dst, const char * src)
		{
			size_t n = 0;
			while (*src)
			{
				*dst = *src;
				dst++;
				src++;
			}
			*dst = '\0';
		}
		inline uint32_t zstrlen(const char * src)
		{
			if (!src)
				return 0;
			uint32_t i = 0;
			while (*src++)
				i++;
			return i;				
		}
	}
}
