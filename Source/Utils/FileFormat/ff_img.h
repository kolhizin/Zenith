#pragma once

#include "ff_base.h"


namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{			
			enum class ImageFormat : uint8_t { UNDEF = 0,
				R8, R16, R32, R32F,
				R8G8, R16G16, R32G32, R32G32F,
				R8G8B8, R16G16B16, R32G32B32, R32G32B32F,
				R8G8B8A8, R16G16B16A16, R32G32B32A32, R32G32B32A32F,
			};

			constexpr uint8_t getPixelSizeConstExpr(ImageFormat f)
			{
				return
					(f == zenith::util::zfile_format::ImageFormat::R8 ? 1 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R16 ? 2 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32 ? 4 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32F ? 4 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R8G8 ? 2 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R16G16 ? 4 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32 ? 8 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32F ? 8 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R8G8B8 ? 3 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R16G16B16 ? 6 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32B32 ? 12 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32B32F ? 12 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R8G8B8A8 ? 4 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R16G16B16A16 ? 8 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32B32A32 ? 16 : 0) +
					(f == zenith::util::zfile_format::ImageFormat::R32G32B32A32F ? 16 : 0);
			}

			//returns size of pixel in bytes
			inline uint8_t getPixelSize(ImageFormat f)
			{
				switch (f)
				{
				case zenith::util::zfile_format::ImageFormat::UNDEF: return 0;
				case zenith::util::zfile_format::ImageFormat::R8: return 1;
				case zenith::util::zfile_format::ImageFormat::R16: return 2;
				case zenith::util::zfile_format::ImageFormat::R32: return 4;
				case zenith::util::zfile_format::ImageFormat::R32F: return 4;
				case zenith::util::zfile_format::ImageFormat::R8G8: return 2;
				case zenith::util::zfile_format::ImageFormat::R16G16: return 4;
				case zenith::util::zfile_format::ImageFormat::R32G32: return 8;
				case zenith::util::zfile_format::ImageFormat::R32G32F: return 8;
				case zenith::util::zfile_format::ImageFormat::R8G8B8: return 3;
				case zenith::util::zfile_format::ImageFormat::R16G16B16: return 6;
				case zenith::util::zfile_format::ImageFormat::R32G32B32: return 12;
				case zenith::util::zfile_format::ImageFormat::R32G32B32F: return 12;
				case zenith::util::zfile_format::ImageFormat::R8G8B8A8: return 4;
				case zenith::util::zfile_format::ImageFormat::R16G16B16A16: return 8;
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32: return 16;
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32F: return 16;
				default: throw ZFileException("getPixelSize: unsupported format.");
				}
			}

			inline const char * ImageFormat2str(ImageFormat f)
			{
				switch (f)
				{
				case zenith::util::zfile_format::ImageFormat::UNDEF: return "UNDEF";
				case zenith::util::zfile_format::ImageFormat::R8: return "R8";
				case zenith::util::zfile_format::ImageFormat::R16: return "R16";
				case zenith::util::zfile_format::ImageFormat::R32: return "R32";
				case zenith::util::zfile_format::ImageFormat::R32F: return "R32F";
				case zenith::util::zfile_format::ImageFormat::R8G8: return "R8G8";
				case zenith::util::zfile_format::ImageFormat::R16G16: return "R16G16";
				case zenith::util::zfile_format::ImageFormat::R32G32: return "R32G32";
				case zenith::util::zfile_format::ImageFormat::R32G32F: return "R32G32F";
				case zenith::util::zfile_format::ImageFormat::R8G8B8: return "R8G8B8";
				case zenith::util::zfile_format::ImageFormat::R16G16B16: return "R16G16B16";
				case zenith::util::zfile_format::ImageFormat::R32G32B32: return "R32G32B32";
				case zenith::util::zfile_format::ImageFormat::R32G32B32F: return "R32G32B32F";
				case zenith::util::zfile_format::ImageFormat::R8G8B8A8: return "R8G8B8A8";
				case zenith::util::zfile_format::ImageFormat::R16G16B16A16: return "R16G16B16A16";
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32: return "R32G32B32A32";
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32F: return "R32G32B32A32F";
				default: return nullptr;
				}
			}
			inline ImageFormat str2ImageFormat(const char * str)
			{
				if (std::strcmp(str, "UNDEF") == 0)return zenith::util::zfile_format::ImageFormat::UNDEF;
				if (std::strcmp(str, "R8") == 0)return zenith::util::zfile_format::ImageFormat::R8;
				if (std::strcmp(str, "R16") == 0)return zenith::util::zfile_format::ImageFormat::R16;
				if (std::strcmp(str, "R32") == 0)return zenith::util::zfile_format::ImageFormat::R32;
				if (std::strcmp(str, "R32F") == 0)return zenith::util::zfile_format::ImageFormat::R32F;
				if (std::strcmp(str, "R8G8") == 0)return zenith::util::zfile_format::ImageFormat::R8G8;
				if (std::strcmp(str, "R16G16") == 0)return zenith::util::zfile_format::ImageFormat::R16G16;
				if (std::strcmp(str, "R32G32") == 0)return zenith::util::zfile_format::ImageFormat::R32G32;
				if (std::strcmp(str, "R32G32F") == 0)return zenith::util::zfile_format::ImageFormat::R32G32F;
				if (std::strcmp(str, "R8G8B8") == 0)return zenith::util::zfile_format::ImageFormat::R8G8B8;
				if (std::strcmp(str, "R16G16B16") == 0)return zenith::util::zfile_format::ImageFormat::R16G16B16;
				if (std::strcmp(str, "R32G32B32") == 0)return zenith::util::zfile_format::ImageFormat::R32G32B32;
				if (std::strcmp(str, "R32G32B32F") == 0)return zenith::util::zfile_format::ImageFormat::R32G32B32F;
				if (std::strcmp(str, "R8G8B8A8") == 0)return zenith::util::zfile_format::ImageFormat::R8G8B8A8;
				if (std::strcmp(str, "R16G16B16A16") == 0)return zenith::util::zfile_format::ImageFormat::R16G16B16A16;
				if (std::strcmp(str, "R32G32B32A32F") == 0)return zenith::util::zfile_format::ImageFormat::R32G32B32A32;
				if (std::strcmp(str, "R32G32B32A32") == 0)return zenith::util::zfile_format::ImageFormat::R32G32B32A32F;
				return zenith::util::zfile_format::ImageFormat::UNDEF;
			}

			enum class ImageType : uint8_t {
				UNDEF = 0,
				IMG1D = 1, IMG2D = 2, IMG3D = 3,
				IMG1D_ARRAY = 5, IMG2D_ARRAY = 6, IMG3D_ARRAY = 7
			};

			class ZChunk16B_ImageHeader
			{
				ChunkType chunkType; /*IMAGE_HEADER*/
			public:
				ImageFormat imageFormat;
				ImageType imageType;
				uint8_t mipLevels;
				uint16_t width, height, depth, arraySize;				
			private:
				uint8_t reserved[4]; /*should be chunkType all*/
				inline bool checkChunk_() const { return chunkType == ChunkType::IMAGE_HEADER; }
				inline bool checkReserved_() const
				{
					return (reserved[0] == static_cast<uint8_t>(ChunkType::IMAGE_HEADER))
						&& (reserved[1] == static_cast<uint8_t>(ChunkType::IMAGE_HEADER))
						&& (reserved[2] == static_cast<uint8_t>(ChunkType::IMAGE_HEADER))
						&& (reserved[3] == static_cast<uint8_t>(ChunkType::IMAGE_HEADER));
				}
			public:
				inline ZChunk16B_ImageHeader() : chunkType(ChunkType::IMAGE_HEADER)
				{
					reserved[0] = static_cast<uint8_t>(chunkType);
					reserved[1] = static_cast<uint8_t>(chunkType);
					reserved[2] = static_cast<uint8_t>(chunkType);
					reserved[3] = static_cast<uint8_t>(chunkType);
				}
				inline bool isValid() const { return checkChunk_() && checkReserved_(); }
			};

			class ZChunk64B_ImageDataHeader
			{
				ChunkType chunkType; /*IMAGE_DATA_HEADER*/ //1b - 1
			public:
				uint8_t mipLevel; //1b - 2
				uint16_t width, height, depth, arraySize; //4*2b -- 10
				zf_size48_t rowPitch; //6b -- 16
				zf_size64_t depthPitch, arrayPitch; //2*8b -- 32
				uint64_t dataSize; //8b -- 40
				uint8_t pixelSize; //1b -- 41	
			private:
				uint8_t reserved[13]; /*should be chunkType all*/
				inline bool checkChunk_() const { return chunkType == ChunkType::IMAGE_DATA_HEADER; }
				inline bool checkReserved_() const
				{
					return (reserved[0] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[1] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[2] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[3] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[4] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[5] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[6] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[7] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[8] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[9] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[10] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[11] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER))
						&& (reserved[12] == static_cast<uint8_t>(ChunkType::IMAGE_DATA_HEADER));
				}
			public:
				inline ZChunk64B_ImageDataHeader() : chunkType(ChunkType::IMAGE_DATA_HEADER)
				{
					reserved[0] = static_cast<uint8_t>(chunkType);
					reserved[1] = static_cast<uint8_t>(chunkType);
					reserved[2] = static_cast<uint8_t>(chunkType);
					reserved[3] = static_cast<uint8_t>(chunkType);
					reserved[4] = static_cast<uint8_t>(chunkType);
					reserved[5] = static_cast<uint8_t>(chunkType);
					reserved[6] = static_cast<uint8_t>(chunkType);
					reserved[7] = static_cast<uint8_t>(chunkType);
					reserved[8] = static_cast<uint8_t>(chunkType);
					reserved[9] = static_cast<uint8_t>(chunkType);
					reserved[10] = static_cast<uint8_t>(chunkType);
					reserved[11] = static_cast<uint8_t>(chunkType);
					reserved[12] = static_cast<uint8_t>(chunkType);
				}
				inline bool isValid() const { return checkChunk_() && checkReserved_(); }
				inline void updateSize()
				{
					dataSize = arrayPitch.get() * arraySize;
				}
				inline uint64_t getRowPitch() const { return rowPitch.get(); }
				inline uint64_t getDepthPitch() const { return depthPitch.get(); }
				inline uint64_t getArrayPitch() const { return arrayPitch.get(); }
				inline uint64_t getDataSize() const { return dataSize; }
			};

			struct zImgDataDescription
			{
			private:
				uint16_t width, height, depth, arraySize;
				zf_size48_t rowPitch; 
				zf_size64_t depthPitch, arrayPitch;
			public:
				inline uint64_t getRequiredDataSize() const
				{
					return arrayPitch.get() * arraySize;
				}
				inline static zImgDataDescription create1D(uint8_t pixelSize, uint16_t width)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = 1;
					descr.depth = 1;
					descr.arraySize = 1;
					
					descr.rowPitch.set(uint64_t(width) * pixelSize);
					descr.depthPitch.set(descr.rowPitch.get());
					descr.arrayPitch.set(descr.rowPitch.get());
					return descr;
				}
				inline static zImgDataDescription create1DArr(uint8_t pixelSize, uint16_t width, uint16_t arrSize, uint64_t arrayPitch)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = 1;
					descr.depth = 1;
					descr.arraySize = arrSize;

					descr.rowPitch.set(uint64_t(width) * pixelSize);
					descr.depthPitch.set(descr.rowPitch.get());
					descr.arrayPitch.set(arrayPitch);
					return descr;
				}
				inline static zImgDataDescription create2D(uint16_t width, uint16_t height, uint64_t rowPitch)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = height;
					descr.depth = 1;
					descr.arraySize = 1;

					descr.rowPitch.set(rowPitch);
					descr.depthPitch.set(descr.rowPitch.get() * height);
					descr.arrayPitch.set(descr.rowPitch.get() * height);
					return descr;
				}
				inline static zImgDataDescription create2DArr(uint16_t width, uint16_t height, uint16_t arrSize, uint64_t rowPitch, uint64_t arrayPitch)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = height;
					descr.depth = 1;
					descr.arraySize = arrSize;

					descr.rowPitch.set(rowPitch);
					descr.depthPitch.set(descr.rowPitch.get() * height);
					descr.arrayPitch.set(arrayPitch);
					return descr;
				}
				inline static zImgDataDescription create3D(uint16_t width, uint16_t height, uint16_t depth, uint64_t rowPitch, uint64_t depthPitch)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = height;
					descr.depth = height;
					descr.arraySize = 1;

					descr.rowPitch.set(rowPitch);
					descr.depthPitch.set(depthPitch);
					descr.arrayPitch.set(descr.depthPitch.get() * depth);
					return descr;
				}
				inline static zImgDataDescription create3DArr(uint16_t width, uint16_t height, uint16_t depth, uint16_t arrSize, uint64_t rowPitch, uint64_t depthPitch, uint64_t arrayPitch)
				{
					zImgDataDescription descr;

					descr.width = width;
					descr.height = height;
					descr.depth = height;
					descr.arraySize = arrSize;

					descr.rowPitch.set(rowPitch);
					descr.depthPitch.set(depthPitch);
					descr.arrayPitch.set(arrayPitch);
					return descr;
				}
				inline uint64_t getRowPitch() const { return rowPitch.get(); }
				inline uint64_t getDepthPitch() const { return depthPitch.get(); }
				inline uint64_t getArrayPitch() const { return arrayPitch.get(); }
			};
			struct zImgDescription
			{
				static const uint8_t MaxMipLevels = 32;
				ImageFormat imageFormat; //1b - 1
				ImageType imageType; //1b - 2
				uint16_t width, height, depth, arraySize; //4*2b - 10
				uint8_t mipLevels; //1b - 11
				uint8_t padding_[5]; //5b - 16
				zImgDataDescription * levelDescr[MaxMipLevels]; 
				void * levelData[MaxMipLevels]; 

				inline zImgDescription()
				{
					for (uint8_t i = 0; i < MaxMipLevels; i++)
					{
						levelDescr[i] = nullptr;
						levelData[i] = nullptr;
					}
				}
				inline uint64_t getDataSize() const
				{
					uint64_t res = 0;
					for (uint8_t i = 0; i < mipLevels; i++)
						res += zf_data_size(levelDescr[i]->getRequiredDataSize()) + sizeof(ZChunk64B_ImageDataHeader);
					return res;
				}
				inline uint64_t getHeaderSize() const
				{
					uint64_t res = sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ImageHeader);
					return res;
				}
				inline uint64_t getFullSize() const
				{
					return getDataSize() + getHeaderSize();
				}
			};

			inline zImgDescription zimg_from_mem(void * src, uint64_t size)
			{
				if (size < 32)
					throw ZFileException("zimg_from_mem: too small size of src.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(src);
				if(!h->isValid())
					throw ZFileException("zimg_from_mem: not valid z-file.");
				if(size != h->getSize() + sizeof(ZChunk16B_Header))
					throw ZFileException("zimg_from_mem: size mismatch header vs provided buffer.");
				ZChunk16B_ImageHeader * ih = reinterpret_cast<ZChunk16B_ImageHeader *>(h + 1);
				if(!ih->isValid())
					throw ZFileException("zimg_from_mem: not valid z-image-file.");

				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ImageHeader);
				uint8_t * ptr = reinterpret_cast<uint8_t *>(ih + 1);
				
				zImgDescription descr;
				descr.imageFormat = ih->imageFormat;
				descr.imageType = ih->imageType;
				descr.width = ih->width;
				descr.height = ih->height;
				descr.depth = ih->depth;
				descr.arraySize = ih->arraySize;
				descr.mipLevels = ih->mipLevels;

				for (uint8_t i = 0; i < descr.mipLevels; i++)
				{
					ZChunk64B_ImageDataHeader * idh = reinterpret_cast<ZChunk64B_ImageDataHeader *>(ptr);
					
					descr.levelDescr[i] = reinterpret_cast<zImgDataDescription *>(ptr + offsetof(ZChunk64B_ImageDataHeader, width));
					
					ptr += sizeof(ZChunk64B_ImageDataHeader);
					size -= sizeof(ZChunk64B_ImageDataHeader);

					uint64_t dSize = 0;
					uint64_t adv = zf_mem_to_dataptr(ptr, size, descr.levelData[i], dSize);
					uint64_t reqSize = descr.levelDescr[i]->getRequiredDataSize();
					if(reqSize != dSize)
						throw ZFileException("zimg_from_mem: data size mismatch with data chunk.");
					ptr += adv;
					size -= adv;
				}
				return descr;
			}
			inline void zimg_to_mem(zImgDescription descr, void * dst, uint64_t size)
			{
				if(descr.getFullSize() > size)
					throw ZFileException("zimg_from_mem: too small size of dst.");
				ZChunk16B_Header * h = reinterpret_cast<ZChunk16B_Header *>(dst);
				*h = ZChunk16B_Header();/*init with defaults*/
				h->varTag[0] = 'I';
				h->varTag[1] = 'M';
				h->varTag[2] = 'G';
				h->setSize(descr.getFullSize() - sizeof(ZChunk16B_Header));
				ZChunk16B_ImageHeader * ih = reinterpret_cast<ZChunk16B_ImageHeader *>(h + 1);
				*ih = ZChunk16B_ImageHeader();
				ih->imageType = descr.imageType;
				ih->imageFormat = descr.imageFormat;
				ih->width = descr.width;
				ih->height = descr.height;
				ih->depth = descr.depth;
				ih->arraySize = descr.arraySize;
				ih->mipLevels = descr.mipLevels;

				uint8_t * ptr = reinterpret_cast<uint8_t *>(ih + 1);
				size -= sizeof(ZChunk16B_Header) + sizeof(ZChunk16B_ImageHeader);
				for (uint8_t i = 0; i < descr.mipLevels; i++)
				{
					ZChunk64B_ImageDataHeader * idh = reinterpret_cast<ZChunk64B_ImageDataHeader *>(ptr);
					*idh = ZChunk64B_ImageDataHeader();

					memcpy_s(&idh->width, sizeof(zImgDataDescription), descr.levelDescr[i], sizeof(zImgDataDescription));

					idh->mipLevel = i;
					idh->pixelSize = getPixelSize(ih->imageFormat);
					idh->updateSize();

					ptr += sizeof(ZChunk64B_ImageDataHeader);
					size -= sizeof(ZChunk64B_ImageDataHeader);

					uint64_t adv = zf_data_to_mem(ptr, size, descr.levelData[i], idh->dataSize);
					ptr += adv;
					size -= adv;
				}
			}
		}
	}
}
