#pragma once

#include "ff_base.h"


namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			/*
			file structure:
			FONT_HEADER
			FONT_INFO
			[0-1]DATA with filename of image file with glyph data
			[0+]GLYPH
			[0+]KERN
			[0+]EXTRA
			*/
			enum class FontType : uint8_t {
				UNDEF = 0,
				INT_ALPHA_FONT, INT_SDIST_FONT,
				EXT_ALPHA_FONT, EXT_SDIST_FONT
			};

			enum class FontTraits : uint8_t
			{
				UNDEF = 0, /*COMMON*/
				BOLD = 1,
				ITALIC = 2,
				BOLD_ITALIC = 3
			};

			inline bool isFontInternal(FontType f)
			{
				if (f == FontType::INT_ALPHA_FONT || f == FontType::INT_SDIST_FONT)
					return true;
			}
			inline bool isFontExternal(FontType f)
			{
				if (f == FontType::EXT_ALPHA_FONT || f == FontType::EXT_SDIST_FONT)
					return true;
			}
			inline bool isFontBold(FontTraits f)
			{
				if (f == FontTraits::BOLD || f == FontTraits::BOLD_ITALIC)
					return true;
			}
			inline bool isFontItalic(FontTraits f)
			{
				if (f == FontTraits::ITALIC || f == FontTraits::BOLD_ITALIC)
					return true;
			}
			inline FontTraits fontTraits(bool bold = false, bool italic = false)
			{
				if(bold)
				{
					if (italic)
						return FontTraits::BOLD_ITALIC;
					else
						return FontTraits::BOLD;
				}
				else
				{
					if (italic)
						return FontTraits::ITALIC;
					else
						return FontTraits::UNDEF;
				}
				return FontTraits::UNDEF;
			}

			class ZChunk16B_FontHeader
			{
				ChunkType chunkType; /*FONT_HEADER*/
			public:
				FontType fontType;
				FontTraits fontTraits;
				uint8_t fontSize;
				uint32_t numGlyphs;
				uint32_t numKernInfo;
				uint32_t numExtraInfo;
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_HEADER; }
				
			public:
				inline ZChunk16B_FontHeader() : chunkType(ChunkType::FONT_HEADER)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};

			class ZChunk16B_FontInfo
			{
				ChunkType chunkType; /*FONT_INFO*/
			public:
				char encoding[11];
				uint32_t externalFileNameLength;
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_INFO; }

			public:
				inline ZChunk16B_FontInfo() : chunkType(ChunkType::FONT_INFO)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};

			class ZChunk64B_FontGlyph
			{
				ChunkType chunkType; /*FONT_GLYPH*/
			public:
				uint8_t padding[3];/*padding*/
				double x;
				double y;
				double w;
				double h;
				uint32_t glyphId;
				int16_t xOffset;
				int16_t yOffset;
				int16_t xAdvance;
				
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_GLYPH; }

			public:
				inline ZChunk64B_FontGlyph() : chunkType(ChunkType::FONT_GLYPH)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};

		}
	}
}