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
			
			zf_memregion2d_t zfont_extract_glyph(const zFontDescription &d, uint32_t glyphId);

			bool zfont_atlas_is_packable(const zf_memregion2d_t * glyphs, uint32_t num, uint32_t w, uint32_t h, uint32_t padding=0);
			void zfont_atlas_pack(zFontDescription &dst, zf_memregion2d_t * glyphs, uint32_t padding = 0);

			//allocates memory, zfont should be freed
			zFontDescription zfont_atlas_sigdist(const zFontDescription &src, uint32_t distExtension, uint32_t rW, uint32_t rH, uint32_t padding, float clampMin, float clampMax);
		}
	}
}
