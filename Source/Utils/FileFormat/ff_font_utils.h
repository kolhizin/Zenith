#pragma once
#include "ff_font.h"
namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			zFontDescription zfont_from_xml(void * src, uint64_t size); /*should be freed by zfont_free*/
			zFontDescription zfont_from_zimg(const zImgDescription &d); /*moves image to font, image should not be freed, font should be freed*/
			void zfont_free(zFontDescription &d);
		}
	}
}
