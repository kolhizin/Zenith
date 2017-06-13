#pragma once

#include "ff_base.h"
#include "ff_img.h"
#include "../str_utils.h"

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			/*
			file structure:
			FONT_HEADER
			FONT_DATA - ENCODING			
			FONT_DATA - [0+] GLYPH
			FONT_DATA - [0+] KERNING
			FONT_DATA - [0+] EXTRA - REFERENCES / DATA / ETC
			*/
			enum class FontType : uint8_t {
				UNDEF = 0,
				ATLAS_ALPHA,
				ATLAS_SIGDIST
			};

			enum class FontChunkType : uint8_t {
				UNDEF = 0,
				NAME_ENCODING,
				GLYPH,
				KERNING,
				DATA_DESCRIPTOR
			};

			struct FontTraits
			{
				uint8_t isBold : 1;
				uint8_t isItalic : 1;
				uint8_t reserved_ : 6;
			};
			class ZChunk16B_FontHeader
			{
				ChunkType chunkType; /*FONT_HEADER*/ //1b -- 1
			public:
				FontType fontType; //1b -- 2
				FontTraits fontTraits; //1b -- 3
				uint8_t fontSize; //1b -- 4
				uint32_t numGlyphs; //4b -- 8
				uint32_t numKernInfo; //4b -- 12
				uint32_t numExtraInfo; //4b -- 16
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_HEADER; }
				
			public:
				inline ZChunk16B_FontHeader() : chunkType(ChunkType::FONT_HEADER)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};
			
			class ZChunk64B_FontData_Glyph
			{
				ChunkType chunkType;  //1b - 1
				FontChunkType fchunkType; //1b - 2
			public:
				int16_t xOffset, yOffset, xAdvance; //3*2b -- 8
				double x, y, w, h; //normalized atlas location, 4*8b -- 40
				uint32_t glyphId; //4b -- 44
				uint8_t reserved[20]; 
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_DATA && fchunkType == FontChunkType::GLYPH; }

			public:
				inline ZChunk64B_FontData_Glyph() : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::GLYPH)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};

			class ZChunk64B_FontData_NameEncoding
			{
				ChunkType chunkType;  //1b - 1
				FontChunkType fchunkType; //1b - 2
			public:
				static const size_t EncodingLength = 29;
				static const size_t NameLength = 31;
				char encodingName[EncodingLength+1]; //30b - 32
				char fontName[NameLength+1];
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_DATA && fchunkType == FontChunkType::NAME_ENCODING; }

			public:
				inline ZChunk64B_FontData_NameEncoding() : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::NAME_ENCODING)
				{
				}
				inline ZChunk64B_FontData_NameEncoding(const char * encoding, const char * name) : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::NAME_ENCODING)
				{
					if (strlen(encoding) > EncodingLength)
						throw ZFileException("ZChunk64B_FontData_Encoding(): too long enconding name!");

					if (strlen(name) > NameLength)
						throw ZFileException("ZChunk64B_FontData_Encoding(): too long font name!");
					zstrcpy(encodingName, encoding);
					zstrcpy(fontName, name);
				}
				inline bool isValid() const { return checkChunk_(); }
				inline const char * encoding() const { return encodingName; }
				inline const char * name() const { return fontName; }
			};

			struct FontData_DataDesc_Atlas
			{
				char paramType[5];
				char atlasFormat[5];
				char reserved[52];

				inline bool isValid() const { return paramType[0] == 'A' && paramType[1] == 'T' && paramType[2] == 'L' && paramType[3] == 'A' && paramType[4] == 'S'; }
			};
			struct FontData_DataDesc_ExtRef
			{
				char paramType[6];
				char extRefFormat[6];
				char reserved[50];

				inline bool isValid() const { return paramType[0] == 'E' && paramType[1] == 'X' && paramType[2] == 'T' && paramType[3] == 'R' && paramType[4] == 'E' && paramType[5] == 'F'; }
			};

			class ZChunk64B_FontData_DataDesc
			{
				ChunkType chunkType;  //1b - 1
				FontChunkType fchunkType; //1b - 2
			public:				
				union
				{
					/*first 6 bytes specify type*/
					uint8_t paramBytes[62];
					char paramStr[62];
					FontData_DataDesc_Atlas paramAtlas;
					FontData_DataDesc_ExtRef paramExtRef;
				};
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_DATA && fchunkType == FontChunkType::DATA_DESCRIPTOR; }

			public:
				inline ZChunk64B_FontData_DataDesc() : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::DATA_DESCRIPTOR)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
			};

			

			class zFontGlyphDescription
			{
				uint16_t reserved_; //for padding
			public:
				int16_t xOffset, yOffset, xAdvance;
				double x, y, w, h;
				uint32_t glyphId;	
			};

			class zFontDescription
			{
				zFontGlyphDescription * glyphs_;
				uint32_t glyphStride_, glyphNum_;				
			public:
				const char * extAtlasFileName;
				zImgDescription intAtlasImage;

				char encodingName[32], fontName[32];

				FontType fontType;
				FontTraits fontTraits;
				uint8_t fontSize;

				inline void setGlyphs(zFontGlyphDescription * pGlyphs, uint32_t numGlyphs, uint32_t glyphStride = sizeof(zFontGlyphDescription))
				{
					glyphs_ = pGlyphs;
					glyphNum_ = numGlyphs;
					glyphStride_ = glyphStride;
				}
				inline void setGlyphs(const zFontDescription &fnt)
				{
					glyphs_ = fnt.glyphs_;
					glyphNum_ = fnt.glyphNum_;
					glyphStride_ = fnt.glyphStride_;
				}
				inline void setEncoding(const char * p) { zstrcpy(encodingName, p); }
				inline void setName(const char * p) { zstrcpy(fontName, p); }
				inline uint32_t numGlyphs() const {	return glyphNum_;}
				inline uint32_t glyphStride()const { return glyphStride_; }
				inline zFontGlyphDescription * glyphsPtr() { return glyphs_; }
				inline const zFontGlyphDescription * glyphsPtr() const { return glyphs_; }
				inline zFontGlyphDescription &getGlyph(uint32_t id)
				{
					if (id >= glyphNum_)
						throw ZFileException("getGlyph(): index out of bounds!");
					return *reinterpret_cast<zFontGlyphDescription *>(reinterpret_cast<uint8_t *>(glyphs_) + id * glyphStride_);
				}
				inline const zFontGlyphDescription &getGlyph(uint32_t id) const
				{
					if (id >= glyphNum_)
						throw ZFileException("getGlyph(): index out of bounds!");
					return *reinterpret_cast<const zFontGlyphDescription *>(reinterpret_cast<const uint8_t *>(glyphs_) + id * glyphStride_);
				}
				static inline zFontDescription empty()
				{
					zFontDescription res;
					res.glyphs_ = nullptr; res.glyphNum_ = 0; res.glyphStride_ = 0;

					res.extAtlasFileName = nullptr;
					res.intAtlasImage = zImgDescription::empty();
					res.fontType = FontType::UNDEF;
					res.fontSize = 0;
					return res;
				}
			};

		}
	}
}