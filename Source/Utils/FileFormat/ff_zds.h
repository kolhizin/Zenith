#pragma once
#include "ff_base.h"

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{

			/*
			Ideas:
			- no forced structure -- it may be implemented as higher level of abstraction
			- tag dictionary at the beginning of data to save space
			- should preserve order, but allow quick navigation to nodes

			Structure of data:
			1. Tag dictionary (16B header + variable size -- ZML_TAGDICT)
			2. Data heap (16B header + variable size -- ZML_DATAHEAP)
			2. Entries (either inplace data (up to 8 bytes) or offset in DATAHEAP)

			Possible entry implementation:
			1 byte -- chunk ZML_ENTRY
			1 byte -- chunk type (attributes and so on)
			2 byte -- tag id -- 65k different tags
			2 byte -- depth of node
			10 bytes for data or offset into data heap with size
			*/


		
			class ZChunk16B_ZDSHeader
			{
				ChunkType chunkType; /*ZDS_HEADER*/
				uint8_t reserved;
				uint16_t numTags;
				uint32_t numEntries;
				zf_size24_t tagHeapSize;
				zf_size40_t dataHeapSize;
				
				inline bool checkChunk_() const { return chunkType == ChunkType::ZDS_HEADER; }
				
			public:
				inline ZChunk16B_ZDSHeader() : chunkType(ChunkType::ZDS_HEADER)
				{
				}
				inline bool isValid() const { return checkChunk_(); }
				uint64_t getDataHeapSize() const { return dataHeapSize.get(); }
				uint64_t getTagHeapSize() const { return tagHeapSize.get(); }
				uint16_t getNumTags() const { return numTags; }
				uint32_t getNumEntries() const { return numEntries; }
				static inline ZChunk16B_ZDSHeader makeHeader(uint64_t dataHeapSize, uint64_t tagHeapSize, uint32_t numChunks, uint16_t numTags)
				{
					ZChunk16B_ZDSHeader res;
					res.dataHeapSize.set(dataHeapSize);
					res.tagHeapSize.set(tagHeapSize);
					res.numEntries = numChunks;
					res.numTags = numTags;
					return res;
				}
			};

			/*
			delta-depth concept is hard to work, better to use absolute depths
			*/
			/*
			0 - any data type
			1 - string in form of ANSI-STR
			2 - string in form of MBC-STR
			3 - string in form of UNICODE-STR
			4 - signed integer
			5 - unsigned integer
			6 - floating point
			7 - reserved
			*/

			enum class ZDSDataBaseType {UNDEF = 0, ANY = 0, ANSI_STR=1, MULTIBYTE_STR=2, UNICODE_STR=3,
				SIGNED_INT=4, UNSIGNED_INT=5, FLOATING_POINT=6, RESERVED = 7};


			class ZChunk16B_ZDSEntry
			{
				ChunkType chunkType; /*ZDS_ENTRY*/
				uint8_t flgAttribute : 1; //if attribute than in xml will be as attribute
				uint8_t szInplace : 4; //0 for out of place 1-10 for inplace sizes
				uint8_t dataType : 3;

				uint16_t tagId_;
				uint16_t depth_; 

				union
				{
					uint8_t data_[10];
					uint64_t data_ui64_;
					int64_t data_i64;
					double data_double_;
					char data_char_[10];
					struct
					{
						zf_size40_t offset_;
						zf_size40_t size_;
					}external;
				};
				
				inline bool checkChunk_() const { return chunkType == ChunkType::ZDS_ENTRY; }
			public:
				inline ZChunk16B_ZDSEntry() : chunkType(ChunkType::ZDS_ENTRY)
				{
				}
				inline void reset_depth(uint16_t d) { depth_ = d; }
				inline void reset_tag(uint16_t id) { tagId_ = id; }
				inline void reset_offset(uint64_t off)
				{
					if (szInplace == 0)
						external.offset_.set(off);
				}
				inline void reset_attribute(bool res) { flgAttribute = (res ? 1 : 0); }
				inline void reset_type(ZDSDataBaseType type) { dataType = static_cast<uint32_t>(type); }
				
				inline bool isValid() const { return checkChunk_(); }
				
				inline uint16_t depth() const { return depth_; }
				inline bool is_attribute() const { return flgAttribute; }
				inline ZDSDataBaseType type() const { return static_cast<ZDSDataBaseType>(dataType); }
				inline uint16_t tagId() const { return tagId_; }
				inline bool is_internal() const { return szInplace > 0; }
				inline bool is_external() const { return szInplace == 0; }
				inline uint64_t heap_offset() const { return external.offset_.get(); }
				inline const uint8_t * data(const uint8_t * heap) const
				{
					if (szInplace > 0)
						return data_;
					else
						return heap + external.offset_.get();
				}
				inline uint8_t * data(uint8_t * heap)
				{
					if (szInplace > 0)
						return data_;
					else
						return heap + external.offset_.get();
				}
				inline uint64_t size() const
				{
					if (szInplace > 0)
						return szInplace;
					else
						return external.size_.get();
				}
				inline void reset_inplace(uint64_t size)
				{
					if (size == 0)
						throw ZFileException("ZChunk16B_ZDSEntry: chunks with 0-size are not allowed.");
					if (size > 10)
						throw ZFileException("ZChunk16B_ZDSEntry: too large size for inplace data.");
					szInplace = size;
				}
				inline void reset_onheap(uint64_t offset, uint64_t size)
				{
					if (size == 0)
						throw ZFileException("ZChunk16B_ZDSEntry: chunks with 0-size are not allowed.");
					if (size >= 0x10000000000)
						throw ZFileException("ZChunk16B_ZDSEntry: too large size for external data.");
					szInplace = 0;
					external.offset_.set(offset);
					external.size_.set(size);
				}
				inline static ZChunk16B_ZDSEntry inplaceEntry(uint64_t size, uint16_t tagId, uint16_t depth, bool flgAttribute=false, ZDSDataBaseType type = ZDSDataBaseType::ANY)
				{
					ZChunk16B_ZDSEntry res;
					if(size == 0)
						throw ZFileException("ZChunk16B_ZDSEntry: chunks with 0-size are not allowed.");
					if (size > 10)
						throw ZFileException("ZChunk16B_ZDSEntry: too large size for inplace data.");
					res.szInplace = size;
					res.tagId_ = tagId;
					res.depth_ = depth;
					res.flgAttribute = (flgAttribute ? 1 : 0);
					res.dataType = static_cast<uint32_t>(type);
					return res;
				}
				inline static ZChunk16B_ZDSEntry onheapEntry(uint64_t offset, uint64_t size, uint16_t tagId, uint16_t depth, bool flgAttribute = false, ZDSDataBaseType type = ZDSDataBaseType::ANY)
				{
					ZChunk16B_ZDSEntry res;
					if (size == 0)
						throw ZFileException("ZChunk16B_ZDSEntry: chunks with 0-size are not allowed.");
					if (size >= 0x10000000000)
						throw ZFileException("ZChunk16B_ZDSEntry: too large size for external data.");
					res.szInplace = 0;
					res.tagId_ = tagId;
					res.depth_ = depth;
					res.flgAttribute = (flgAttribute ? 1 : 0);
					res.external.offset_.set(offset);
					res.external.size_.set(size);
					res.dataType = static_cast<uint32_t>(type);
					return res;
				}
			};

			struct zDataStorageDescription
			{
				uint8_t * dataHeap; //data heap
				uint8_t * tagHeap; //0-terminated char strings for tags names
				ZChunk16B_ZDSEntry * chunks;

				uint64_t dataHeapSize;
				uint64_t tagHeapSize;
				uint32_t numChunks;
				uint16_t numTags;

				inline uint64_t getChunksSize() const { return uint64_t(numChunks) * sizeof(ZChunk16B_ZDSEntry); }
				inline uint64_t getTagsSize() const { return zf_data_size(tagHeapSize); }
				inline uint64_t getDataSize() const { return (dataHeapSize > 0 ? zf_data_size(dataHeapSize) : 0); }
				inline uint64_t getHeaderSize() const { return sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ZDSHeader); }
				inline uint64_t getFullSize() const
				{
					return getHeaderSize() + getDataSize() + getTagsSize() + getChunksSize();
				}
			};

			
			inline zDataStorageDescription zds_from_mem(void * src, uint64_t size)
			{
				if (size < 32)
					throw ZFileException("zds_from_mem: too small size of src.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(src);
				if (!h->isValid())
					throw ZFileException("zds_from_mem: not valid z-file.");
				if (size != h->getSize() + sizeof(ZChunk16B_Header))
					throw ZFileException("zds_from_mem: size mismatch header vs provided buffer.");
				ZChunk16B_ZDSHeader * ih = reinterpret_cast<ZChunk16B_ZDSHeader *>(h + 1);
				if (!ih->isValid())
					throw ZFileException("zds_from_mem: not valid z-data-structure-file.");

				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ZDSHeader);
				uint8_t * ptr = reinterpret_cast<uint8_t *>(ih + 1);

				zDataStorageDescription descr;
				descr.dataHeapSize = ih->getDataHeapSize();
				descr.tagHeapSize = ih->getTagHeapSize();
				descr.numTags = ih->getNumTags();
				descr.numChunks = ih->getNumEntries();

				void * rawTagsDataPtr = nullptr;
				uint64_t rawTagsDataActSize = 0;
				uint64_t tagsAdv = zf_mem_to_dataptr(ptr, descr.tagHeapSize, rawTagsDataPtr, rawTagsDataActSize);
				if(rawTagsDataActSize != descr.tagHeapSize)
					throw ZFileException("zds_from_mem: tags data size mismatch with data chunk.");
				ptr += tagsAdv;
				size -= tagsAdv;

				void * heapDataPtr = nullptr;
				if (ih->getDataHeapSize() > 0)
				{
					uint64_t heapDataActSize = 0;
					uint64_t heapAdv = zf_mem_to_dataptr(ptr, ih->getDataHeapSize(), heapDataPtr, heapDataActSize);
					if (heapDataActSize != ih->getDataHeapSize())
						throw ZFileException("zds_from_mem: heap data size mismatch with data chunk.");
					ptr += heapAdv;
					size -= heapAdv;
				}

				if(size != descr.numChunks * sizeof(ZChunk16B_ZDSEntry))
					throw ZFileException("zds_from_mem: remaining size does not match number of tags.");

				descr.dataHeap = reinterpret_cast<uint8_t *>(heapDataPtr);
				descr.tagHeap = reinterpret_cast<uint8_t *>(rawTagsDataPtr);
				descr.chunks = reinterpret_cast<ZChunk16B_ZDSEntry *>(ptr);

				for (uint32_t i = 0; i < descr.numChunks; i++)
				{
					if(!descr.chunks[i].isValid())
						throw ZFileException("zds_from_mem: invalid zds-entry-chunk.");
					if(descr.chunks[i].tagId() >= descr.numTags)
						throw ZFileException("zds_from_mem: tag-id exceeds number of tags.");
				}

				return descr;
			}
			inline void zds_to_mem(zDataStorageDescription descr, void * dst, uint64_t size)
			{
				if (descr.getFullSize() > size)
					throw ZFileException("zds_to_mem: too small size of dst.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(dst);
				*h = ZChunk16B_Header();/*init with defaults*/
				h->varTag[0] = 'Z';
				h->varTag[1] = 'D';
				h->varTag[2] = 'S';
				h->setSize(descr.getFullSize() - sizeof(ZChunk16B_Header));
				ZChunk16B_ZDSHeader * ih = reinterpret_cast<ZChunk16B_ZDSHeader *>(h + 1);
				*ih = ZChunk16B_ZDSHeader::makeHeader(descr.dataHeapSize, descr.tagHeapSize, descr.numChunks, descr.numTags);

				uint8_t * ptr = reinterpret_cast<uint8_t *>(ih + 1);
				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ZDSHeader);

				{
					uint64_t adv = zf_data_to_mem(ptr, size, descr.tagHeap, descr.tagHeapSize);
					ptr += adv;
					size -= adv;
				}
				{
					if (descr.dataHeapSize > 0)
					{
						uint64_t adv = zf_data_to_mem(ptr, size, descr.dataHeap, descr.dataHeapSize);
						ptr += adv;
						size -= adv;
					}
				}
				memcpy_s(ptr, size, descr.chunks, uint64_t(descr.numChunks) * sizeof(ZChunk16B_ZDSEntry));
			}
		}
	}
}
