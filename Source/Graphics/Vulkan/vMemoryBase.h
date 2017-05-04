#pragma once

#include "vulkan_general.h"
#include "vUtil.h"



namespace zenith
{
	namespace vulkan
	{
		constexpr size_t WholeSize = 0xFFFFFFFF;

		class vMemoryException : public VulkanException
		{
		public:
			vMemoryException() : VulkanException() {}
			vMemoryException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vMemoryException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vMemoryException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vMemoryException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vMemoryException() {}
		};

		class vMemoryException_OutOfMemory : public vMemoryException
		{
		public:
			vMemoryException_OutOfMemory() : vMemoryException() {}
			vMemoryException_OutOfMemory(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(p, type)
			{
			}
			vMemoryException_OutOfMemory(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(str, type)
			{
			}
			vMemoryException_OutOfMemory(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, p, type)
			{
			}
			vMemoryException_OutOfMemory(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, str, type)
			{
			}
			virtual ~vMemoryException_OutOfMemory() {}
		};
		class vMemoryException_NotOwn : public vMemoryException
		{
		public:
			vMemoryException_NotOwn() : vMemoryException() {}
			vMemoryException_NotOwn(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(p, type)
			{
			}
			vMemoryException_NotOwn(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(str, type)
			{
			}
			vMemoryException_NotOwn(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, p, type)
			{
			}
			vMemoryException_NotOwn(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, str, type)
			{
			}
			virtual ~vMemoryException_NotOwn() {}
		};
		class vMemoryException_NotAligned : public vMemoryException
		{
		public:
			vMemoryException_NotAligned() : vMemoryException() {}
			vMemoryException_NotAligned(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(p, type)
			{
			}
			vMemoryException_NotAligned(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(str, type)
			{
			}
			vMemoryException_NotAligned(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, p, type)
			{
			}
			vMemoryException_NotAligned(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, str, type)
			{
			}
			virtual ~vMemoryException_NotAligned() {}
		};
		class vMemoryException_AlreadyMapped : public vMemoryException
		{
		public:
			vMemoryException_AlreadyMapped() : vMemoryException() {}
			vMemoryException_AlreadyMapped(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(p, type)
			{
			}
			vMemoryException_AlreadyMapped(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(str, type)
			{
			}
			vMemoryException_AlreadyMapped(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, p, type)
			{
			}
			vMemoryException_AlreadyMapped(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : vMemoryException(res, str, type)
			{
			}
			virtual ~vMemoryException_AlreadyMapped() {}
		};

		class vMemoryChunkImpl_;
		class vMemoryPool_;
		class vMemoryBlockAuto;

		class vMemoryBlock
		{
			friend class vMemorySubPool_;
			friend class vMemoryPool_;
			friend class vMemoryManagerImpl_;
			friend class vMemoryBlockAuto;
			

			vMemoryChunkImpl_ * physicalChunk_;
			vMemoryPool_ * memoryPool_;
			util::memory::MemAllocBlock memBlk_;
			inline vMemoryBlock(vMemoryPool_ * memPool, vMemoryChunkImpl_ * p, const util::memory::MemAllocBlock &b) : memBlk_(b), physicalChunk_(p), memoryPool_(memPool) {};
		public:
			inline vMemoryBlock(const vMemoryBlock&oth) : physicalChunk_(oth.physicalChunk_), memBlk_(oth.memBlk_), memoryPool_(oth.memoryPool_) {}
			inline vMemoryBlock &operator=(const vMemoryBlock&oth)
			{
				physicalChunk_ = oth.physicalChunk_;
				memoryPool_ = oth.memoryPool_;
				memBlk_ = oth.memBlk_;
				return *this;
			}
			inline bool valid() const { return  (physicalChunk_ && memoryPool_ && memBlk_.size > 0); }
			inline size_t size() const { return memBlk_.size; }
			inline void * ptr() const { return memBlk_.ptr; }
			inline uint64_t offset() const { return reinterpret_cast<uint64_t>(memBlk_.ptr); }

			void * map(size_t offset = 0, size_t size = WholeSize);
			void unmap();
			VkDeviceMemory handle() const;
		};

		class vMemoryBlockAuto
		{
			friend class vMemorySubPool_;
			friend class vMemoryPool_;
			friend class vMemoryManagerImpl_;

			vMemoryChunkImpl_ * physicalChunk_;
			vMemoryPool_ * memoryPool_;
			util::memory::MemAllocBlock memBlk_;
			void free_();
			vMemoryBlockAuto(const vMemoryBlockAuto &) = delete;
			vMemoryBlockAuto &operator =(const vMemoryBlockAuto &) = delete;
		public:
			inline vMemoryBlockAuto() : physicalChunk_(nullptr), memoryPool_(nullptr) {}
			inline ~vMemoryBlockAuto() { free_(); }
			inline vMemoryBlockAuto(const vMemoryBlock& oth) : physicalChunk_(oth.physicalChunk_), memBlk_(oth.memBlk_), memoryPool_(oth.memoryPool_) {}
			inline vMemoryBlockAuto(vMemoryBlockAuto &&oth) : physicalChunk_(oth.physicalChunk_), memBlk_(oth.memBlk_), memoryPool_(oth.memoryPool_)
			{
				oth.memoryPool_ = nullptr;
				oth.physicalChunk_ = nullptr;
				oth.memBlk_ = util::memory::MemAllocBlock();
			}
			inline vMemoryBlockAuto &operator =(vMemoryBlockAuto &&oth)
			{
				free_();
				physicalChunk_ = oth.physicalChunk_;
				memBlk_ = oth.memBlk_;
				memoryPool_ = oth.memoryPool_;
				oth.memoryPool_ = nullptr;
				oth.physicalChunk_ = nullptr;
				oth.memBlk_ = util::memory::MemAllocBlock();
				return *this;
			}
			inline vMemoryBlockAuto &operator =(const vMemoryBlock &oth)
			{
				free_();
				physicalChunk_ = oth.physicalChunk_;
				memBlk_ = oth.memBlk_;
				memoryPool_ = oth.memoryPool_;
				return *this;
			}
			inline void deallocate() { free_(); }
			inline bool valid() const { return  (physicalChunk_ && memoryPool_ && memBlk_.size > 0); }
			inline size_t size() const { return memBlk_.size; }
			inline void * ptr() const { return memBlk_.ptr; }
			inline uint64_t offset() const { return reinterpret_cast<uint64_t>(memBlk_.ptr); }

			void * map(size_t offset = 0, size_t size = WholeSize);
			void unmap();

			VkDeviceMemory handle() const;
		};
	}
}