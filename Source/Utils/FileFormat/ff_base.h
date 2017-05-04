#pragma once

#include <cstdint>
#include "../log_config.h"

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			enum class ChunkType : uint8_t {UNDEF = 0, HEADER, IMAGE_HEADER, IMAGE_DATA_HEADER, DATA=255};

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

			struct Chunk16B
			{
				uint8_t chunkType;
				uint8_t chunkData[15];
			};

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
		}
	}
}
