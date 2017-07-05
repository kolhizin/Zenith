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
				EXTERNAL_REFERENCE
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

			class ZChunk64B_FontData
			{
				ChunkType chunkType;  //1b - 1
				FontChunkType fchunkType; //1b - 2
				uint8_t reserved[62]; 
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_DATA; }

			public:
				inline ZChunk64B_FontData() : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::UNDEF)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
				inline FontChunkType type() const { return fchunkType; }
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
				inline void setEncoding(const char * p) { zstrcpy(encodingName, p); }
				inline void setName(const char * p) { zstrcpy(fontName, p); }
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
			/*
			struct FontData_DataDesc_Atlas
			{
				char paramType[5];
				char atlasFormat[5];
				char reserved[52];

				inline bool isValid() const { return paramType[0] == 'A' && paramType[1] == 'T' && paramType[2] == 'L' && paramType[3] == 'A' && paramType[4] == 'S'; }
			};
			*/
			struct FontData_DataDesc_ExtRef
			{
				char paramType[6];
				char extRefFormat[6];
				char reserved[50];

				inline bool isValid() const { return paramType[0] == 'E' && paramType[1] == 'X' && paramType[2] == 'T' && paramType[3] == 'R' && paramType[4] == 'E' && paramType[5] == 'F'; }
			};

			class ZChunk64B_FontData_ExtRef
			{
				ChunkType chunkType;  //1b - 1
				FontChunkType fchunkType; //1b - 2
			public:			
				char extRefFormat[6];
				char extRefFilename[56];
			private:
				inline bool checkChunk_() const { return chunkType == ChunkType::FONT_DATA && fchunkType == FontChunkType::EXTERNAL_REFERENCE; }

			public:

				inline void setFormat(const char * p) { zstrcpy(extRefFormat, p); }
				inline void setFilename(const char * p) { zstrcpy(extRefFilename, p); }
				inline ZChunk64B_FontData_ExtRef() : chunkType(ChunkType::FONT_DATA), fchunkType(FontChunkType::EXTERNAL_REFERENCE)
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
				uint32_t reserved2_;
			};

			class zFontDescription
			{
				zFontGlyphDescription * glyphs_ = nullptr;
				uint32_t glyphStride_, glyphNum_;				
			public:
				char * extAtlasFileName = nullptr;
				char * extAtlasFormat = nullptr;
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
					res.extAtlasFormat = nullptr;
					res.intAtlasImage = zImgDescription::empty();
					res.fontType = FontType::UNDEF;
					res.fontSize = 0;
					return res;
				}
				//returns size in bytes for glyphs storage
				inline uint64_t getGlyphsSize() const
				{
					return sizeof(ZChunk64B_FontData_Glyph) * numGlyphs();
				}
				//returns size in bytes for atlas storage
				inline uint64_t getAtlasSize() const
				{
					if (intAtlasImage.width == 0 && !extAtlasFileName)
						return 0;

					if (intAtlasImage.width == 0 && extAtlasFileName != nullptr)
						return sizeof(ZChunk64B_FontData_ExtRef);

					return intAtlasImage.getFullSize();
				}
				//returns size in bytes for header storage
				inline uint64_t getHeaderSize() const
				{
					return sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_FontHeader) + sizeof(ZChunk64B_FontData_NameEncoding);
				}
				inline uint64_t getFullSize() const
				{
					return getHeaderSize() + getAtlasSize() + getGlyphsSize();
				}
			};

			inline zFontDescription zfont_from_mem(void * src, uint64_t size)
			{
				if (size < 32)
					throw ZFileException("zfont_from_mem: too small size of src.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(src);
				if (!h->isValid())
					throw ZFileException("zfont_from_mem: not valid z-file.");
				if (size != h->getSize() + sizeof(ZChunk16B_Header))
					throw ZFileException("zfont_from_mem: size mismatch header vs provided buffer.");
				ZChunk16B_FontHeader * fh = reinterpret_cast<ZChunk16B_FontHeader *>(h + 1);
				if (!fh->isValid())
					throw ZFileException("zfont_from_mem: not valid z-font-file.");

				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_FontHeader);
				uint8_t * ptr = reinterpret_cast<uint8_t *>(fh + 1);

				zFontDescription descr;
				descr.fontSize = fh->fontSize;
				descr.fontTraits = fh->fontTraits;
				descr.fontType = fh->fontType;
				descr.setGlyphs(nullptr, 0, sizeof(ZChunk64B_FontData_Glyph));
				//fh->numGlyphs
				uint32_t realGlyphNum = 0; bool glyphsEnded = false;
				bool atlasFound = false;
				bool extRefFound = false;
				while (size > 0)
				{
					if(size < 64)
						throw ZFileException("zfont_from_mem: invalid size for data chunk.");
					ZChunk16B_Header * fah = reinterpret_cast<ZChunk16B_Header *>(ptr);
					if (fah->isValid())
					{
						//img detected
						if(atlasFound)
							throw ZFileException("zfont_from_mem: multiple atlasses found.");
						uint32_t imgSize = fah->getSize() + sizeof(ZChunk16B_Header);
						if(imgSize > size)
							throw ZFileException("zfont_from_mem: image size exceeds residual file size.");
						descr.intAtlasImage = zimg_from_mem(ptr, imgSize);
						atlasFound = true;
						ptr += imgSize;
						size -= imgSize;
						continue;
					}
					ZChunk64B_FontData * fd = reinterpret_cast<ZChunk64B_FontData *>(ptr);
					ptr = reinterpret_cast<uint8_t *>(fd + 1);
					size -= sizeof(ZChunk64B_FontData);

					if(!fd->isValid())
						throw ZFileException("zfont_from_mem: invalid data chunk.");
					if (fd->type() == FontChunkType::GLYPH)
					{
						if(glyphsEnded)
							throw ZFileException("zfont_from_mem: glyphs should be continuous.");
						realGlyphNum++;
						if(realGlyphNum == 1)
							descr.setGlyphs(reinterpret_cast<zFontGlyphDescription *>(fd), fh->numGlyphs, sizeof(ZChunk64B_FontData_Glyph));
					}
					else
					{
						if (realGlyphNum > 0)
							glyphsEnded = true;
						
						if (fd->type() == FontChunkType::NAME_ENCODING)
						{
							ZChunk64B_FontData_NameEncoding * cp = reinterpret_cast<ZChunk64B_FontData_NameEncoding *>(fd);
							descr.setEncoding(cp->encoding());
							descr.setName(cp->name());
						}
						else if (fd->type() == FontChunkType::EXTERNAL_REFERENCE)
						{
							ZChunk64B_FontData_ExtRef * cp = reinterpret_cast<ZChunk64B_FontData_ExtRef *>(fd);
							descr.extAtlasFileName = cp->extRefFilename;
							descr.extAtlasFormat = cp->extRefFormat;
							extRefFound = true;
						}else throw ZFileException("zfont_from_mem: unexpected font data chunk.");
					}
				}
				if (descr.fontType == FontType::ATLAS_ALPHA || descr.fontType == FontType::ATLAS_SIGDIST)
				{
					if(atlasFound && extRefFound)
						throw ZFileException("zfont_from_mem: both internal atlas and external reference found.");
					if ((!atlasFound) && (!extRefFound))
						throw ZFileException("zfont_from_mem: neither internal atlas, nor external reference found.");
				}
				return descr;
			}
			inline void zfont_to_mem(zFontDescription descr, void * dst, uint64_t size)
			{
				if (descr.getFullSize() > size)
					throw ZFileException("zfont_to_mem: too small size of dst.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(dst);
				*h = ZChunk16B_Header();/*init with defaults*/
				h->varTag[0] = 'F';
				h->varTag[1] = 'N';
				h->varTag[2] = 'T';
				h->setSize(descr.getFullSize() - sizeof(ZChunk16B_Header));
				ZChunk16B_FontHeader * fh = reinterpret_cast<ZChunk16B_FontHeader *>(h + 1);
				*fh = ZChunk16B_FontHeader();
				fh->fontSize = descr.fontSize;
				fh->fontTraits = descr.fontTraits;
				fh->fontType = descr.fontType;
				fh->numExtraInfo = 0;
				fh->numKernInfo = 0;
				fh->numGlyphs = descr.numGlyphs();				
				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_FontHeader);

				ZChunk64B_FontData_NameEncoding * fdn = reinterpret_cast<ZChunk64B_FontData_NameEncoding *>(fh + 1);
				*fdn = ZChunk64B_FontData_NameEncoding();
				fdn->setEncoding(descr.encodingName);
				fdn->setName(descr.fontName);
				size -= sizeof(ZChunk64B_FontData_NameEncoding);
				void * cptr = fdn + 1;

				for (uint32_t i = 0; i < descr.numGlyphs(); i++)
				{
					ZChunk64B_FontData_Glyph * fg = reinterpret_cast<ZChunk64B_FontData_Glyph *>(cptr);
					*fg = ZChunk64B_FontData_Glyph();
					const auto &g = descr.getGlyph(i);
					fg->glyphId = g.glyphId;
					fg->w = g.w;
					fg->h = g.h;
					fg->x = g.x;
					fg->y = g.y;

					fg->xOffset = g.xOffset;
					fg->yOffset = g.yOffset;
					fg->xAdvance = g.xAdvance;

					cptr = fg + 1;
					size -= sizeof(ZChunk64B_FontData_Glyph);
				}

				if (descr.fontType == FontType::ATLAS_ALPHA || descr.fontType == FontType::ATLAS_SIGDIST)
				{
					if (descr.extAtlasFileName)
					{
						ZChunk64B_FontData_ExtRef * fdref = reinterpret_cast<ZChunk64B_FontData_ExtRef *>(cptr);
						size -= sizeof(ZChunk64B_FontData_NameEncoding);
						cptr = fdref + 1;
						*fdref = ZChunk64B_FontData_ExtRef();
						fdref->setFormat(descr.extAtlasFormat);
						fdref->setFilename(descr.extAtlasFileName);
					}
					else
					{
						uint32_t imgSize = descr.intAtlasImage.getFullSize();
						zimg_to_mem(descr.intAtlasImage, cptr, size);
						size -= imgSize;
						cptr = (reinterpret_cast<uint8_t *>(cptr) + imgSize);
					}
				}
			}

		}
	}
}