#pragma once

#include <cstdint>
#include "../log_config.h"

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			enum class ChunkType : uint8_t {UNDEF = 0,
					HEADER=1,
						IMAGE_HEADER, FONT_HEADER,
					RESERVED=128,
						IMAGE_DATA_HEADER, FONT_GLYPH_INFO,
					DATA08=253, DATA16=254, DATA48=255};

			class ZFileException : public zenith::util::LoggedException
			{
			public:
				ZFileException() : zenith::util::LoggedException() {}
				ZFileException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(p, type)
				{
				}
				ZFileException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : zenith::util::LoggedException(str, type)
				{
				}
				virtual ~ZFileException() {}
			};
			class zf_size16_t
			{
				uint8_t data_[2];
			public:
				static inline uint64_t max()
				{
					return (static_cast<uint64_t>(1) << 16) - 1;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) +
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size16_t::set: value exceeds limits of this type.");
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};
			class zf_size24_t
			{
				uint8_t data_[3];
			public:
				static inline uint64_t max()
				{
					return (static_cast<uint64_t>(1) << 24) - 1;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[2]) * (static_cast<uint64_t>(1) << 16) +
						static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) +
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size24_t::set: value exceeds limits of this type.");
					data_[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};
			class zf_size32_t
			{
				uint8_t data_[4];
			public:
				static inline uint64_t max()
				{
					return (static_cast<uint64_t>(1) << 32) - 1;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[3]) * (static_cast<uint64_t>(1) << 24) +
						static_cast<uint64_t>(data_[2]) * (static_cast<uint64_t>(1) << 16) +
						static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) +
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size32_t::set: value exceeds limits of this type.");
					data_[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
					data_[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};
			class zf_size40_t
			{
				uint8_t data_[5];
			public:
				static inline uint64_t max()
				{
					return (static_cast<uint64_t>(1) << 40) - 1;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[4]) * (static_cast<uint64_t>(1) << 32) + 
						static_cast<uint64_t>(data_[3]) * (static_cast<uint64_t>(1) << 24) + 
						static_cast<uint64_t>(data_[2]) * (static_cast<uint64_t>(1) << 16) + 
						static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) + 
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size40_t::set: value exceeds limits of this type.");
					data_[4] = static_cast<uint8_t>((v >> 32) & 0xFF);
					data_[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
					data_[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};
			class zf_size48_t
			{
				uint8_t data_[6];
			public:
				static inline uint64_t max()
				{
					return (static_cast<uint64_t>(1) << 48) - 1;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[5]) * (static_cast<uint64_t>(1) << 40) +
						static_cast<uint64_t>(data_[4]) * (static_cast<uint64_t>(1) << 32) +
						static_cast<uint64_t>(data_[3]) * (static_cast<uint64_t>(1) << 24) +
						static_cast<uint64_t>(data_[2]) * (static_cast<uint64_t>(1) << 16) +
						static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) +
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size48_t::set: value exceeds limits of this type.");
					data_[5] = static_cast<uint8_t>((v >> 40) & 0xFF);
					data_[4] = static_cast<uint8_t>((v >> 32) & 0xFF);
					data_[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
					data_[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};
			class zf_size64_t
			{
				uint8_t data_[8];
			public:
				static inline uint64_t max()
				{
					return 0xFFFFFFFFFFFFFFFF;
				}
				inline uint64_t get() const
				{
					return static_cast<uint64_t>(data_[7]) * (static_cast<uint64_t>(1) << 56) +
						static_cast<uint64_t>(data_[6]) * (static_cast<uint64_t>(1) << 48) +
						static_cast<uint64_t>(data_[5]) * (static_cast<uint64_t>(1) << 40) +
						static_cast<uint64_t>(data_[4]) * (static_cast<uint64_t>(1) << 32) +
						static_cast<uint64_t>(data_[3]) * (static_cast<uint64_t>(1) << 24) +
						static_cast<uint64_t>(data_[2]) * (static_cast<uint64_t>(1) << 16) +
						static_cast<uint64_t>(data_[1]) * (static_cast<uint64_t>(1) << 8) +
						static_cast<uint64_t>(data_[0]);
				}
				inline void set(uint64_t v)
				{
					if (v > max())
						throw ZFileException("zf_size48_t::set: value exceeds limits of this type.");
					data_[7] = static_cast<uint8_t>((v >> 56) & 0xFF);
					data_[6] = static_cast<uint8_t>((v >> 48) & 0xFF);
					data_[5] = static_cast<uint8_t>((v >> 40) & 0xFF);
					data_[4] = static_cast<uint8_t>((v >> 32) & 0xFF);
					data_[3] = static_cast<uint8_t>((v >> 24) & 0xFF);
					data_[2] = static_cast<uint8_t>((v >> 16) & 0xFF);
					data_[1] = static_cast<uint8_t>((v >> 8) & 0xFF);
					data_[0] = static_cast<uint8_t>(v & 0xFF);
				}
			};

			template<class T>
			T zf_align16(T v)
			{
				if (v & 0xF)
					return ((v >> 4) + 1) << 4;
				return v;
			}

			struct Chunk16B
			{
				ChunkType chunkType;
				uint8_t chunkData[15];
			};
			struct Chunk64B
			{
				ChunkType chunkType;
				uint8_t chunkData[63];
			};
			struct ChunkD08H
			{
				ChunkType chunkType;
				uint8_t chunkSize;//including these 2 bytes

				static const uint64_t ExtraSize = 2;

				inline ChunkD08H(uint8_t sz) : chunkSize(sz), chunkType(ChunkType::DATA08) {}
				inline uint64_t dataSize() const { return chunkSize - 2; }
				inline uint64_t dataOffset() const { return ExtraSize; }
				inline void * dataPtr() const { return reinterpret_cast<void *>(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(this) + dataOffset())); }
				
				inline uint64_t fullSize() const { return zf_align16(uint16_t(chunkSize)); }

				inline bool isValid() const { return chunkType == ChunkType::DATA08; }
			};
			struct ChunkD16H
			{
				ChunkType chunkType;
				uint8_t dataOffsetRel;
				zf_size16_t chunkSize;//including these 4 bytes

				static const uint64_t ExtraSize = 4;

				inline ChunkD16H(uint16_t sz, uint8_t off = 0) : chunkType(ChunkType::DATA16), dataOffsetRel(off) { chunkSize.set(sz); }
				inline uint64_t dataSize() const { return chunkSize.get() - dataOffsetRel - ExtraSize; }
				inline uint64_t dataOffset() const { return ExtraSize + dataOffsetRel; }
				inline void * dataPtr() const { return reinterpret_cast<void *>(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(this) + dataOffset())); }
				inline uint64_t fullSize() const { return zf_align16(chunkSize.get()); }

				inline bool isValid() const { return chunkType == ChunkType::DATA16; }
			};
			struct ChunkD48H
			{
				ChunkType chunkType;
				uint8_t dataOffsetRel;
				zf_size48_t chunkSize;//including these 8 bytes				

				static const uint64_t ExtraSize = 8;

				inline ChunkD48H(uint64_t sz, uint8_t off = 0) : chunkType(ChunkType::DATA48), dataOffsetRel(off) { chunkSize.set(sz); }
				inline uint64_t dataSize() const { return chunkSize.get() - dataOffsetRel - ExtraSize; }
				inline uint64_t dataOffset() const { return ExtraSize + dataOffsetRel; }
				inline void * dataPtr() const { return reinterpret_cast<void *>(const_cast<uint8_t *>(reinterpret_cast<const uint8_t *>(this) + dataOffset())); }
				inline uint64_t fullSize() const { return zf_align16(chunkSize.get()); }

				inline bool isValid() const { return chunkType == ChunkType::DATA48; }
			};

			
			inline uint64_t zChunkSize(uint8_t chunkByte)
			{
				if (chunkByte == 0)
					return 0;
				if (chunkByte < 128)
					return 16;
				if (chunkByte < 253)
					return 64;
				return 0xFFFFFFFFFFFFFFFF;
			}
			inline uint64_t zChunkSize(ChunkType chunkType)
			{
				return zChunkSize(static_cast<uint8_t>(chunkType));
			}
			inline uint64_t zChunkSize(uint8_t *chunkBytes)
			{
				uint64_t r = zChunkSize(chunkBytes);
				if (r == 0xFFFFFFFFFFFFFFFF)
				{
					if (*chunkBytes == 253)
						return reinterpret_cast<ChunkD08H *>(chunkBytes)->chunkSize;
					if (*chunkBytes == 254)
						return reinterpret_cast<ChunkD16H *>(chunkBytes)->chunkSize.get();
					if (*chunkBytes == 255)
						return reinterpret_cast<ChunkD48H *>(chunkBytes)->chunkSize.get();
					return r;
				}
				else return r;
			}

			class ZChunk16B_Header
			{
				ChunkType chunkType; /*HEADER*/
				char fixedTag[3]; /*ZFF*/
			public:
				char varTag[3];/*defines format*/
			private:
				uint8_t magic[4];
				zf_size40_t size; /*File size after this header*/

				inline bool checkChunk_() const { return chunkType == ChunkType::HEADER; }
				inline bool checkFixed_() const { return (fixedTag[0] == 'Z') && (fixedTag[1] == 'F') && (fixedTag[2] == 'F'); }
				inline bool checkMagic_() const { return (magic[0] == 0x0D) && (magic[1] == 0x0A) && (magic[2] == 0x1A) && (magic[3] == 0x0A); }
			public:
				inline ZChunk16B_Header()
				{
					chunkType = ChunkType::HEADER;
					fixedTag[0] = 'Z';
					fixedTag[1] = 'F';
					fixedTag[2] = 'F';
					varTag[0] = '?';
					varTag[1] = '?';
					varTag[2] = '?';
					magic[0] = 0x0D;
					magic[1] = 0x0A;
					magic[2] = 0x1A;
					magic[3] = 0x0A;
					size.set(0);
				}
				inline bool isValid() const
				{
					return checkChunk_() && checkFixed_() && checkMagic_();
				}
				inline void setSize(uint64_t sz)
				{
					size.set(sz);
				}
				inline uint64_t getSize() const /*max 1TB*/
				{
					return size.get();
				}
			};

			inline uint64_t zf_data_size(uint64_t dataSize, uint8_t dataAlign = 0)
			{
				if (dataAlign > 16)
					throw ZFileException("zf_data_size(): dataAlign of more than 16 is not supported by file format!");
				bool okAlign = (dataAlign <= 2) || (dataAlign == 4) || (dataAlign == 8) || (dataAlign == 16);
				if (!okAlign)
					throw ZFileException("zf_data_size(): dataAlign is not power of two!");
				const uint64_t extraSize8 = ChunkD08H::ExtraSize;
				const uint64_t extraSize16 = ChunkD16H::ExtraSize;
				const uint64_t extraSize48 = ChunkD48H::ExtraSize;
				if (dataAlign <= 1 && zf_align16(dataSize + extraSize8) <= 255)
					return zf_align16(dataSize + extraSize8);
				uint64_t offset16 = 0, offset48 = 0;
				if (dataAlign == 16)
				{
					offset16 = 12;
					offset48 = 8;
				}
				else if (dataAlign == 8)
				{
					offset16 = 4;
				}
				uint64_t realSize16 = extraSize16 + offset16 + dataSize;
				uint64_t realSize48 = extraSize48 + offset48 + dataSize;
				uint64_t fullSize16 = zf_align16(realSize16);
				uint64_t fullSize48 = zf_align16(realSize48);
				if (fullSize16 <= 0xFFFF)
					return fullSize16;				
				if (fullSize48 <= 0xFFFFFFFFFFFF)
					return fullSize48;
				throw ZFileException("zf_data_size(): too large size of data!");
			}

			inline uint64_t zf_data_to_mem(void * dstPtr, uint64_t dstSize, void * dataPtr, uint64_t dataSize, uint8_t dataAlign = 0)
			{
				if (dataAlign > 16)
					throw ZFileException("zf_data_to_mem(): dataAlign of more than 16 is not supported by file format!");
				bool okAlign = (dataAlign <= 2) || (dataAlign == 4) || (dataAlign == 8) || (dataAlign == 16);
				if(!okAlign)
					throw ZFileException("zf_data_to_mem(): dataAlign is not power of two!");
				const uint64_t extraSize8 = ChunkD08H::ExtraSize;
				const uint64_t extraSize16 = ChunkD16H::ExtraSize;
				const uint64_t extraSize48 = ChunkD48H::ExtraSize;
				if (dataAlign <= 1 && zf_align16(dataSize + extraSize8) <= 255)
				{					
					uint64_t realSize = dataSize + extraSize8;
					uint64_t fullSize = zf_align16(realSize);

					if(dstSize < fullSize)
						throw ZFileException("zf_data_to_mem(): dstSize is not enough to write required data!");
					ChunkD08H * p = reinterpret_cast<ChunkD08H *>(dstPtr);
					*p = ChunkD08H(static_cast<uint8_t>(realSize));

					auto r = memcpy_s(p->dataPtr(), dstSize - extraSize8, dataPtr, dataSize);
					if(r != 0)
						throw ZFileException("zf_data_to_mem(): failed to memcpy_s!");					
					return fullSize;
				}
				uint64_t offset16 = 0, offset48 = 0;
				if (dataAlign == 16)
				{
					offset16 = 12;
					offset48 = 8;
				}
				else if (dataAlign == 8)
				{
					offset16 = 4;
				}
				uint64_t realSize16 = extraSize16 + offset16 + dataSize;
				uint64_t realSize48 = extraSize48 + offset48 + dataSize;
				uint64_t fullSize16 = zf_align16(realSize16);
				uint64_t fullSize48 = zf_align16(realSize48);
				if (fullSize16 <= 0xFFFF)
				{
					if (dstSize < fullSize16)
						throw ZFileException("zf_data_to_mem(): dstSize is not enough to write required data!");
					ChunkD16H * p = reinterpret_cast<ChunkD16H *>(dstPtr);
					*p = ChunkD16H(static_cast<uint16_t>(realSize16), static_cast<uint8_t>(offset16));

					auto r = memcpy_s(p->dataPtr(), dstSize - extraSize16 - offset16, dataPtr, dataSize);
					if (r != 0)
						throw ZFileException("zf_data_to_mem(): failed to memcpy_s!");
					return fullSize16;
				}
				if (fullSize48 <= 0xFFFFFFFFFFFF)
				{
					if (dstSize < fullSize48)
						throw ZFileException("zf_data_to_mem(): dstSize is not enough to write required data!");
					ChunkD48H * p = reinterpret_cast<ChunkD48H *>(dstPtr);
					*p = ChunkD48H(realSize48, static_cast<uint8_t>(offset48));

					auto r = memcpy_s(p->dataPtr(), dstSize - extraSize48 - offset48, dataPtr, dataSize);
					if (r != 0)
						throw ZFileException("zf_data_to_mem(): failed to memcpy_s!");
					return fullSize48;
				}
				throw ZFileException("zf_data_to_mem(): too large size of data!");
			}
			inline uint64_t zf_mem_to_data(void * srcPtr, uint64_t srcSize, void * dataPtr, uint64_t dstSize)
			{
				uint8_t * p = static_cast<uint8_t *>(srcPtr);
				if (static_cast<ChunkType>(*p) == ChunkType::DATA08)
				{
					ChunkD08H * cp = static_cast<ChunkD08H *>(srcPtr);
					uint64_t dataSize = cp->dataSize();
					if(dataSize > dstSize)
						throw ZFileException("zf_mem_to_data(): data size exceeds buffer size!");
					auto r = memcpy_s(dataPtr, dstSize, cp->dataPtr(), srcSize - cp->dataOffset());
					if (r != 0)
						throw ZFileException("zf_mem_to_data(): failed to memcpy_s!");
					return cp->fullSize();
				}
				if (static_cast<ChunkType>(*p) == ChunkType::DATA16)
				{
					ChunkD16H * cp = static_cast<ChunkD16H *>(srcPtr);
					uint64_t dataSize = cp->dataSize();
					if (dataSize > dstSize)
						throw ZFileException("zf_mem_to_data(): data size exceeds buffer size!");
					auto r = memcpy_s(dataPtr, dstSize, cp->dataPtr(), srcSize - cp->dataOffset());
					if (r != 0)
						throw ZFileException("zf_mem_to_data(): failed to memcpy_s!");
					return cp->fullSize();
				}
				if (static_cast<ChunkType>(*p) == ChunkType::DATA48)
				{
					ChunkD48H * cp = static_cast<ChunkD48H *>(srcPtr);
					uint64_t dataSize = cp->dataSize();
					if (dataSize > dstSize)
						throw ZFileException("zf_mem_to_data(): data size exceeds buffer size!");
					auto r = memcpy_s(dataPtr, dstSize, cp->dataPtr(), srcSize - cp->dataOffset());
					if (r != 0)
						throw ZFileException("zf_mem_to_data(): failed to memcpy_s!");
					return cp->fullSize();
				}
				throw ZFileException("zf_mem_to_data(): incompatible chunk-type!");
			}
			inline uint64_t zf_mem_to_dataptr(void * srcPtr, uint64_t srcSize, void *&dataPtr, uint64_t &dataSize)
			{
				uint8_t * p = static_cast<uint8_t *>(srcPtr);
				if (static_cast<ChunkType>(*p) == ChunkType::DATA08)
				{
					ChunkD08H * cp = static_cast<ChunkD08H *>(srcPtr);
					dataPtr = cp->dataPtr();
					dataSize = cp->dataSize();
					return cp->fullSize();
				}
				if (static_cast<ChunkType>(*p) == ChunkType::DATA16)
				{
					ChunkD16H * cp = static_cast<ChunkD16H *>(srcPtr);
					dataPtr = cp->dataPtr();
					dataSize = cp->dataSize();
					return cp->fullSize();
				}
				if (static_cast<ChunkType>(*p) == ChunkType::DATA48)
				{
					ChunkD48H * cp = static_cast<ChunkD48H *>(srcPtr);
					dataPtr = cp->dataPtr();
					dataSize = cp->dataSize();
					return cp->fullSize();
				}
				throw ZFileException("zf_mem_to_dataptr(): incompatible chunk-type!");
			}
		}
	}
}
