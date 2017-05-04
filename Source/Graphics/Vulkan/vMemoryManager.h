#pragma once

#include "vMemoryBase.h"
#include <Utils\alloc_config.h>
#include <Utils\pimpl.h>
#include <list>
#include <mutex>

namespace zenith
{
	namespace vulkan
	{
		
		
		/*
		using MainAllocator = util::memory::MemAlloc_DynamicSegregator<
			util::memory::MemAlloc_Combine<util::memory::MemAllocVirtual_BitmapDynamic<AllocBase, MetaAllocator>, util::memory::MemAllocPool_DynamicVirtual>,
			util::memory::MemAlloc_Combine<util::memory::MemAllocVirtual_BitmapDynamic<AllocBase, MetaAllocator>, util::memory::MemAllocPool_DynamicVirtual>,
			AllocFnBase
		>;
		*/

		class vDeviceImpl_;
		class vDevice;
				
		class vMemoryChunkImpl_
		{
			using AllocatorLocalImpl_ =
				util::memory::MemAlloc_Fallback<
					util::memory::MemAlloc_Combine<util::memory::MemAllocVirtual_StaticSize<util::zstdAllocatorBase<util::memory::MemAlloc_BASE>, 1, 64, 1024>, util::memory::MemAllocPool_Stack<64 * 1024, 16>>,
					util::memory::MemAlloc_MAlloc<util::zstdAllocatorBase<util::memory::MemAlloc_BASE>>
				>;

			using AllocBase = util::memory::MemAlloc_FnExt_ErrThrow<util::memory::MemAlloc_BASE>;
			using AllocFnBase = util::memory::MemAlloc_FnExt_ErrThrow<util::memory::MemAllocFn_BASE>;

			using MainAllocator = util::memory::MemAlloc_Combine<util::memory::MemAllocVirtual_ListAlloc<AllocBase>, util::memory::MemAllocPool_DynamicVirtual>;

			util::memory::MemAllocInfo mai_;

			AllocatorLocalImpl_ locAlloc_;
			MainAllocator mainAlloc_;

			VkDevice device_;
			size_t totalSize_, usedSize_;
			uint32_t memType_;
			const VkAllocationCallbacks * extCallbacks_;
			VkDeviceMemory devMem_;

			void * mapPtr_;
			size_t mapSize_, mapOffset_;

			inline static util::memory::MemAllocBlock locAllocAlloc_(void * allocPtr, size_t size, util::memory::MemAllocHint hint)
			{
				return static_cast<AllocatorLocalImpl_ *>(allocPtr)->allocate(size, hint);
			}
			inline static void locAllocDealloc_(void * allocPtr, const util::memory::MemAllocBlock &blk)
			{
				static_cast<AllocatorLocalImpl_ *>(allocPtr)->deallocate(blk);
			}
			util::memory::MemAllocInfo * createMAI_(size_t fullSize, size_t alignment, size_t minAllocSize);

			vMemoryChunkImpl_(const vMemoryChunkImpl_ &) = delete;
			vMemoryChunkImpl_(vMemoryChunkImpl_ &&) = delete;
			vMemoryChunkImpl_ &operator =(const vMemoryChunkImpl_ &) = delete;
			vMemoryChunkImpl_ &operator =(vMemoryChunkImpl_ &&) = delete;
		public:
			vMemoryChunkImpl_(VkDevice dev, size_t size, uint32_t memType, size_t allocAlign, size_t allocMinSize, const VkAllocationCallbacks * extCB = nullptr);
			~vMemoryChunkImpl_();

			util::memory::MemAllocBlock allocate(size_t size);
			void deallocate(const util::memory::MemAllocBlock &mab);

			inline size_t unusedMemory() const { return totalSize_ - usedSize_; }
			inline size_t usedMemory() const { return usedSize_; }
			inline size_t totalMemory() const { return totalSize_; }
			inline double usageMemory() const { return double(usedSize_) / double(totalSize_); }
			inline bool isFree() const { return usedSize_ > 0; }
			inline bool isFull() const { return usedSize_ >= totalSize_; }
			inline VkDeviceMemory handle() const { return devMem_; }

			void * map(size_t offset = 0, size_t size = WholeSize);
			void unmap();
		};

		class vMemorySubPool_;
		
		class vMemorySubPoolChunk_
		{
			friend class vMemorySubPool_;

			vMemoryChunkImpl_ * memChunk_;
			util::memory::MemAllocBlock memBlk_;/*from memChunk_*/
			zenith::util::memory::MemAllocVirtual_BitmapDynamicExt<util::memory::MemAlloc_BASE> allocator_;/*do not throw when failed to alloc!*/
			size_t used_;
		public:
			vMemorySubPoolChunk_(vMemoryChunkImpl_ * chnk, const util::memory::MemAllocBlock &memBlk, const util::memory::MemAllocInfo * mai);
			~vMemorySubPoolChunk_() {}
			vMemorySubPoolChunk_(vMemorySubPoolChunk_ &&) = delete;
			vMemorySubPoolChunk_(const vMemorySubPoolChunk_ &) = delete;
			vMemorySubPoolChunk_ &operator =(const vMemorySubPoolChunk_ &) = delete;
			vMemorySubPoolChunk_ &operator =(vMemorySubPoolChunk_ &&) = delete;

			util::memory::MemAllocBlock allocate(size_t sz);
			void deallocate(const util::memory::MemAllocBlock &blk);
			inline bool owns(const util::memory::MemAllocBlock &blk) const;

			inline size_t unusedMemory() const { return memBlk_.size - used_; }
			inline size_t usedMemory() const { return used_; }
			inline size_t totalMemory() const { return memBlk_.size; }
			inline double usageMemory() const { return double(used_) / double(memBlk_.size); }
			inline bool isFree() const { return used_ > 0; }
			inline bool isFull() const { return used_ >= memBlk_.size; }
		};

		

		


		class vMemorySubPool_
		{
			util::memory::MemAllocInfo memAllocInfo_;
			std::list<vMemorySubPoolChunk_> subPoolChunks_;
			uint32_t subTypeMask_;

			void addChunk_(vMemoryChunkImpl_ * physicalChunk_, size_t chunkSize_);
			void removeChunk_(std::list<vMemorySubPoolChunk_>::iterator &it);
		public:
			~vMemorySubPool_();
			vMemorySubPool_(const vMemoryLogicalChunkConfig &lcc, void * locAllocPtr, util::memory::MemAllocInfo::PFN_Allocate pfnAlloc, util::memory::MemAllocInfo::PFN_Deallocate pfnDealloc);
			vMemorySubPool_(const vMemorySubPool_ &) = delete;
			vMemorySubPool_(vMemorySubPool_ &&) = delete;
			vMemorySubPool_ &operator =(const vMemorySubPool_ &) = delete;
			vMemorySubPool_ &operator =(vMemorySubPool_ &&) = delete;

			vMemoryBlock allocate(size_t sz);
			void deallocate(const vMemoryBlock &blk);

			bool try_allocate(size_t sz, vMemoryBlock &blk);
			bool try_deallocate(const vMemoryBlock &blk);
			bool owns(const vMemoryBlock &blk) const;

			inline bool suits(size_t size, uint32_t subType)
			{
				if (size > memAllocInfo_.allocMaxSize)
					return false;
				if ((subType & subTypeMask_) != subType)
					return false;
				return true;
			}

			void addChunkOverride(vMemoryChunkImpl_ * physicalChunk, size_t chunkSize);
			void addChunk(vMemoryChunkImpl_ * physicalChunk);
			void releaseUnusedChunks();
		};

		
		class vMemoryPool_
		{
			using AllocBase = util::memory::MemAlloc_FnExt_ErrThrow<util::memory::MemAlloc_BASE>;
			using AllocFnBase = util::memory::MemAlloc_FnExt_ErrThrow<util::memory::MemAllocFn_BASE>;

			using AllocatorLocalImpl_ =
				util::memory::MemAlloc_Fallback<
					util::memory::MemAlloc_Combine<util::memory::MemAllocVirtual_StaticSize<util::zstdAllocatorBase<util::memory::MemAlloc_BASE>, 1, 64, 1024>, util::memory::MemAllocPool_Stack<64 * 1024, 16>>,
					util::memory::MemAlloc_MAlloc<util::zstdAllocatorBase<util::memory::MemAlloc_BASE>>
				>;

			
			const VkAllocationCallbacks * extCallbacks_;
			
			AllocatorLocalImpl_ locAlloc_;

			inline static util::memory::MemAllocBlock locAllocAlloc_(void * allocPtr, size_t size, util::memory::MemAllocHint hint)
			{
				return static_cast<AllocatorLocalImpl_ *>(allocPtr)->allocate(size, hint);
			}
			inline static void locAllocDealloc_(void * allocPtr, const util::memory::MemAllocBlock &blk)
			{
				static_cast<AllocatorLocalImpl_ *>(allocPtr)->deallocate(blk);
			}

			vMemoryPoolConfig conf_;

			std::list<vMemoryChunkImpl_> memChunks_, userChunks_;
			std::list<vMemorySubPool_> pools_;
			mutable std::mutex poolMutex_;
			
			uint32_t memType_, memHeap_;
			VkMemoryPropertyFlags memTypeFlags_;
			VkMemoryHeapFlags memHeapFlags_;
			size_t memGranularity_;
			vDeviceImpl_ * pDevice_;

			vMemoryPool_(const vMemoryPool_ &) = delete;
			vMemoryPool_(vMemoryPool_ &&) = delete;
			vMemoryPool_ &operator =(const vMemoryPool_ &) = delete;
			vMemoryPool_ &operator =(vMemoryPool_ &&) = delete;

			void addPhysicalChunk_(size_t sz);
			void removePhysicalChunk_(std::list<vMemoryChunkImpl_>::iterator it);

			vMemoryBlock allocateUser_(size_t sz);
			void deallocateUser_(std::list<vMemoryChunkImpl_>::iterator it, const vMemoryBlock &mb);

			vMemoryBlock allocatePhysical_(vMemoryChunkImpl_ * chnk, size_t sz);
			vMemoryBlock allocatePhysical_(size_t sz);
			void deallocatePhysical_(const vMemoryBlock &mb);

			vMemoryBlock allocateLogical_(vMemorySubPool_ * sp, size_t sz);
			void deallocateLogical_(vMemorySubPool_ * sp, const vMemoryBlock &mb);

			vMemorySubPool_ * findSuitingPool_(size_t size, size_t alignment, uint32_t subType);
			vMemorySubPool_ * findOwningPool_(const vMemoryBlock &mb);

			
		public:
			vMemoryPool_(vDeviceImpl_ * device, const vMemoryPoolConfig &conf, uint32_t memType);
			~vMemoryPool_();

			vMemoryBlock allocate(size_t sz, size_t alignment, uint32_t subType, bool dedicatedMemoryObject = false);
			void deallocate(const vMemoryBlock &blk);
		};

		

		class vMemoryManagerImpl_
		{
			std::list<vMemoryPool_> pools_;
			std::array<std::vector<VkMemoryPropertyFlags>, static_cast<size_t>(vMemoryUsage::MAX)> usageMap_;
			vMemoryManagerConfig conf_;
			vDeviceImpl_ * dev_;
			VkPhysicalDeviceMemoryProperties memProps_;
			
			vMemoryPool_ * findSuitingPool_(uint32_t memoryTypeBits, VkMemoryPropertyFlags propertyFlags);

			vMemoryManagerImpl_(const vMemoryManagerImpl_ &) = delete;
			vMemoryManagerImpl_(vMemoryManagerImpl_ &&) = delete;
			vMemoryManagerImpl_ &operator =(const vMemoryManagerImpl_ &) = delete;
			vMemoryManagerImpl_ &operator =(vMemoryManagerImpl_ &&) = delete;
		public:
			vMemoryManagerImpl_(vDeviceImpl_ * dev, const vMemoryManagerConfig &config, const std::vector<vMemoryPoolConfig> &pools);
			
			vMemoryBlock allocate(size_t size, size_t alignment, uint32_t memoryTypeBits, vMemoryUsage usage, vMemoryLayout layout, bool dedicatedMemory = false);
			void deallocate(const vMemoryBlock &blk);

			inline const util::nameid &getUID() const { return conf_.uid; }
		};


		class vMemoryManager : public zenith::util::pimpl<vMemoryManagerImpl_>
		{
			friend class vDevice;
			using Impl = zenith::util::pimpl<vMemoryManagerImpl_>;
			using RawImpl = vMemoryManagerImpl_;

			RawImpl * raw() { return Impl::get(); }
		public:
			inline vMemoryManager() {};
			inline vMemoryManager(vDevice &dev, const vMemoryConfig &conf) { create(dev, conf); }
			/*
			inline vMemoryManager(vMemoryManager &&m) : Impl(std::forward<Impl>(m)) {};
			inline vMemoryManager &operator =(vMemoryManager &&m)
			{
				Impl::operator =(std::forward<Impl>(m));
				return *this;
			}

			vMemoryManager(const vMemoryManager &) = delete;
			vMemoryManager &operator =(const vMemoryManager &) = delete;
			*/
			void create(vDevice &dev, const vMemoryConfig &conf);
			void destroy() { Impl::destroy(); }

			inline vMemoryBlock allocate(size_t size, size_t alignment, uint32_t memoryTypeBits, vMemoryUsage usage, vMemoryLayout layout, bool dedicatedMemory = false){ return Impl::get()->allocate(size,alignment,memoryTypeBits, usage, layout, dedicatedMemory); }
			inline void deallocate(const vMemoryBlock &blk) { return Impl::get()->deallocate(blk); }
			const RawImpl * rawImpl() const { return Impl::get(); }

			const util::nameid &getImplUID() const { return Impl::get()->getUID(); }
		};
		
	}
}
