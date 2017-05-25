#pragma once
#include "ff_img.h"
namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			zImgDescription zimg_from_png(void * src, uint64_t size);
			zImgDescription zimg_from_img(void * src, uint64_t size, const char * fmt);
			zImgDescription zimg_clone(const zImgDescription &src);
			zImgDescription zimg_change_format(const zImgDescription &src, ImageFormat dstFormat);
			void zimg_free(zImgDescription &d);
		}
	}
}