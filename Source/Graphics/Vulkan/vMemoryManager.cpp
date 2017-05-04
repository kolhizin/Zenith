#include "vDevice.h"
#include "vMemoryManager.h"

zenith::util::memory::MemAllocInfo * zenith::vulkan::vMemoryChunkImpl_::createMAI_(size_t fullSize, size_t alignment, size_t minAllocSize)
{
	mai_.allocPtr = &locAlloc_;
	mai_.pfnAllocate = &zenith::vulkan::vMemoryChunkImpl_::locAllocAlloc_;
	mai_.pfnDeallocate = &zenith::vulkan::vMemoryChunkImpl_::locAllocDealloc_;

	mai_.allocMinSize = minAllocSize;
	mai_.allocMaxSize = fullSize;

	mai_.poolSize = fullSize;
	mai_.poolAlign = alignment;
	mai_.poolOffset = nullptr;
	return &mai_;
}

zenith::vulkan::vMemoryChunkImpl_::vMemoryChunkImpl_(VkDevice dev, size_t size, uint32_t memType, size_t allocAlign, size_t allocMinSize, const VkAllocationCallbacks * extCB)
	: locAlloc_(), mainAlloc_(createMAI_(size, allocAlign, allocMinSize)), device_(dev), totalSize_(size), memType_(memType), extCallbacks_(extCB), usedSize_(0), mapPtr_(nullptr)
{
	VkMemoryAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	ai.pNext = nullptr;
	ai.allocationSize = totalSize_;
	ai.memoryTypeIndex = memType_;
	
	VkResult res = vkAllocateMemory(device_, &ai, extCallbacks_, &devMem_);
	if (res != VK_SUCCESS)
		throw vMemoryException(res, "vMemoryChunkImpl_::vMemoryChunkImpl_: failed to allocate vulkan memory!");
}

zenith::vulkan::vMemoryChunkImpl_::~vMemoryChunkImpl_()
{
	vkFreeMemory(device_, devMem_, extCallbacks_);
}

zenith::util::memory::MemAllocBlock zenith::vulkan::vMemoryChunkImpl_::allocate(size_t size)
{
	zenith::util::memory::MemAllocBlock res = mainAlloc_.allocate(size);
	usedSize_ += res.size;
	return res;
}

void zenith::vulkan::vMemoryChunkImpl_::deallocate(const zenith::util::memory::MemAllocBlock & mab)
{
	mainAlloc_.deallocate(mab);
	usedSize_ -= mab.size;
}
void * zenith::vulkan::vMemoryChunkImpl_::map(size_t offset, size_t size)
{
	if (mapPtr_)
		throw vMemoryException_AlreadyMapped("vMemoryChunkImpl_::map: memory already mapped.");
	if(size != WholeSize && offset + size > totalSize_)
		throw vMemoryException("vMemoryChunkImpl_::map: requested offset and size exceed total size of chunk.");
	if(size == WholeSize && offset >= totalSize_)
		throw vMemoryException("vMemoryChunkImpl_::map: requested offset exceeds total size of chunk.");
	auto res = vkMapMemory(device_, devMem_, offset, (size == WholeSize ? VK_WHOLE_SIZE : size), 0, &mapPtr_);
	if(res != VK_SUCCESS)
		throw vMemoryException(res, "vMemoryChunkImpl_::map: vkMapMemory failed to map memory.");
	return mapPtr_;
}

void zenith::vulkan::vMemoryChunkImpl_::unmap()
{
	if(!mapPtr_)
		throw vMemoryException("vMemoryChunkImpl_::unmap: memory is not mapped.");
	vkUnmapMemory(device_, devMem_);
	mapPtr_ = nullptr;
}

zenith::vulkan::vMemorySubPoolChunk_::vMemorySubPoolChunk_(vMemoryChunkImpl_ * chnk, const zenith::util::memory::MemAllocBlock &memBlk, const zenith::util::memory::MemAllocInfo * mai)
	: allocator_(mai), memBlk_(memBlk), memChunk_(chnk)
{
}

zenith::util::memory::MemAllocBlock zenith::vulkan::vMemorySubPoolChunk_::allocate(size_t sz)
{
	zenith::util::memory::MemAllocBlock blk = allocator_.allocate(sz);
	blk.ptr = static_cast<uint8_t*>(memBlk_.ptr) + reinterpret_cast<size_t>(blk.ptr);
	used_ += blk.size;
	return blk;
}

void zenith::vulkan::vMemorySubPoolChunk_::deallocate(const zenith::util::memory::MemAllocBlock & blk)
{
	zenith::util::memory::MemAllocBlock blk0 = blk;
	blk0.ptr = reinterpret_cast<void*>(static_cast<uint8_t*>(blk0.ptr) - static_cast<uint8_t*>(memBlk_.ptr));
	allocator_.deallocate(blk0);
	used_ -= blk.size;
}

inline bool zenith::vulkan::vMemorySubPoolChunk_::owns(const util::memory::MemAllocBlock & blk) const
{
	zenith::util::memory::MemAllocBlock blk0 = blk;
	blk0.ptr = reinterpret_cast<void*>(static_cast<uint8_t*>(blk0.ptr) - static_cast<uint8_t*>(memBlk_.ptr));
	return allocator_.owns(blk0);
}

void zenith::vulkan::vMemorySubPool_::addChunk_(vMemoryChunkImpl_ * physicalChunk_, size_t chunkSize_)
{
	if (memAllocInfo_.allocMinSize * 8 > chunkSize_)
		throw vMemoryException("vMemorySubPool_::addChunk_: failed to create sub-pool: physical chunk-size is not large enough for required blocks (must be at least 8 times larger).");
	if (memAllocInfo_.allocMaxSize> chunkSize_)
		throw vMemoryException("vMemorySubPool_::addChunk_: max alloc size could not be larger than physical chunk-size.");
	if (chunkSize_ % (memAllocInfo_.allocMinSize * 8))
		throw vMemoryException("vMemorySubPool_::addChunk_: physical chunk size should be a multiple of 8*block-size.");
	if(chunkSize_ > physicalChunk_->unusedMemory())
		throw vMemoryException_OutOfMemory("vMemorySubPool_::addChunk_: requested size is greater than unused memory in physical chunk.");

	ZLOG_DEBUG("vMemorySubPool_::addChunk_: allocating memory from physical chunk.");
	util::memory::MemAllocBlock blk = physicalChunk_->allocate(chunkSize_);
	if (blk.size < chunkSize_)
		throw vMemoryException_OutOfMemory("vMemorySubPool_::addChunk_: failed to alloc block from physical chunk of required size.");
	ZLOG_DEBUG("vMemorySubPool_::addChunk_: allocation complete.");

	util::memory::MemAllocInfo mai = memAllocInfo_;
	mai.poolSize = blk.size;
	memAllocInfo_.allocNumBlock = blk.size / memAllocInfo_.allocMinSize;
	subPoolChunks_.emplace_back(physicalChunk_, blk, &mai);
}

void zenith::vulkan::vMemorySubPool_::removeChunk_(std::list<vMemorySubPoolChunk_>::iterator & it)
{
	vMemoryChunkImpl_ * p = it->memChunk_;

	ZLOG_DEBUG("vMemorySubPool_::removeChunk_: deallocating memory from physical chunk.");
	p->deallocate(it->memBlk_);
	ZLOG_DEBUG("vMemorySubPool_::removeChunk_: deallocation complete.");

	subPoolChunks_.erase(it);
}

zenith::vulkan::vMemorySubPool_::~vMemorySubPool_()
{
	while (!subPoolChunks_.empty())
		removeChunk_(subPoolChunks_.begin());
}

zenith::vulkan::vMemorySubPool_::vMemorySubPool_(const vMemoryLogicalChunkConfig & lcc, void * locAllocPtr, util::memory::MemAllocInfo::PFN_Allocate pfnAlloc, util::memory::MemAllocInfo::PFN_Deallocate pfnDealloc)
{
	if (lcc.blockSize * 8 > lcc.chunkSize)
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: failed to create sub-pool: physical chunk-size is not large enough for required blocks (must be at least 8 times larger).");
	if(lcc.align != lcc.blockSize)
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: align should be equal to block-size.");
	if (!locAllocPtr)
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: local-allocator cannot be nullptr.");
	if(lcc.maxAllocSize > lcc.chunkSize)
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: max alloc size could not be larger than physical chunk-size.");
	if(lcc.maxAllocSize < lcc.blockSize)
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: max alloc size could not be smaller than logical block-size.");
	if (lcc.chunkSize % (lcc.blockSize*8))
		throw vMemoryException("vMemorySubPool_::vMemorySubPool_: physical chunk size should be a multiple of 8*block-size.");

	subTypeMask_ = lcc.subTypeMask;
	
	memAllocInfo_.allocExtension = nullptr;
	memAllocInfo_.poolExtension = nullptr;
	
	memAllocInfo_.allocMinSize = lcc.blockSize;
	memAllocInfo_.allocMaxSize = lcc.maxAllocSize;
	memAllocInfo_.allocNumBlock = lcc.chunkSize / lcc.blockSize;

	memAllocInfo_.poolAlign = lcc.align;
	memAllocInfo_.poolSize = lcc.chunkSize;
	memAllocInfo_.poolOffset = nullptr;

	memAllocInfo_.allocPtr = locAllocPtr;
	memAllocInfo_.pfnAllocate = pfnAlloc;
	memAllocInfo_.pfnDeallocate = pfnDealloc;
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemorySubPool_::allocate(size_t sz)
{
	vMemoryBlock res(nullptr, nullptr, zenith::util::memory::MemAllocBlock());
	for (auto &chnk : subPoolChunks_)
	{
		res.memBlk_ = chnk.allocate(sz);
		if (res.memBlk_.size >= sz)
		{
			res.physicalChunk_ = chnk.memChunk_;
			return res;
		}
	}
	throw vMemoryException_OutOfMemory("vMemorySubPool_::allocate: could not allocate block!");
}

void zenith::vulkan::vMemorySubPool_::deallocate(const zenith::vulkan::vMemoryBlock & blk)
{
	for (auto &chnk : subPoolChunks_)
	{
		if (!chnk.owns(blk.memBlk_))
			continue;
		chnk.deallocate(blk.memBlk_);
		return;
	}
	throw vMemoryException_NotOwn("vMemorySubPool_::deallocate: tried to deallocate not own block!");
}

bool zenith::vulkan::vMemorySubPool_::try_allocate(size_t sz, zenith::vulkan::vMemoryBlock & blk)
{
	for (auto &chnk : subPoolChunks_)
	{
		blk.memBlk_ = chnk.allocate(sz);
		if (blk.memBlk_.size >= sz)
		{
			blk.physicalChunk_ = chnk.memChunk_;
			return true;
		}
	}
	return false;
}

bool zenith::vulkan::vMemorySubPool_::try_deallocate(const zenith::vulkan::vMemoryBlock & blk)
{
	for (auto &chnk : subPoolChunks_)
	{
		if (!chnk.owns(blk.memBlk_))
			continue;
		chnk.deallocate(blk.memBlk_);
		return true;
	}
	return false;
}

bool zenith::vulkan::vMemorySubPool_::owns(const zenith::vulkan::vMemoryBlock & blk) const
{
	for (auto &chnk : subPoolChunks_)
	{
		if (!chnk.owns(blk.memBlk_))
			continue;
		return true;
	}
	return false;
}

void zenith::vulkan::vMemorySubPool_::addChunkOverride(vMemoryChunkImpl_ * physicalChunk, size_t chunkSize)
{
	addChunk_(physicalChunk, chunkSize);
}

void zenith::vulkan::vMemorySubPool_::addChunk(vMemoryChunkImpl_ * physicalChunk)
{
	addChunk_(physicalChunk, memAllocInfo_.poolSize);
}

void zenith::vulkan::vMemorySubPool_::releaseUnusedChunks()
{
	auto it = subPoolChunks_.begin();
	while(it != subPoolChunks_.end())
		if (it->isFree())
		{
			removeChunk_(it);
			it = subPoolChunks_.begin();
		}
		else it++;
}

zenith::vulkan::vMemoryPool_::vMemoryPool_(vDeviceImpl_ * device, const vMemoryPoolConfig & conf, uint32_t memType) : conf_(conf), memType_(memType), pDevice_(device)
{
	VkPhysicalDeviceProperties props;
	VkPhysicalDeviceMemoryProperties mprops;
	vkGetPhysicalDeviceProperties(device->getPhysicalDevice(), &props);
	memGranularity_ = props.limits.bufferImageGranularity;

	vkGetPhysicalDeviceMemoryProperties(device->getPhysicalDevice(), &mprops);

	memHeap_ = mprops.memoryTypes[memType_].heapIndex;
	memTypeFlags_ = mprops.memoryTypes[memType_].propertyFlags;
	memHeapFlags_ = mprops.memoryHeaps[memHeap_].flags;	
	
	if (conf.physicalChunk.minAllocSize < memGranularity_)
		throw vMemoryException("vMemoryPool_::vMemoryPool_: physical chunk size is less than buffer-image granularity!");

	ZLOG_REGULAR("vMemoryPool_::vMemoryPool_: creating memory pool.");
	for (size_t i = 0; i < conf_.logicalChunk.size(); i++)
		pools_.emplace_back(conf_.logicalChunk[i], static_cast<void*>(&locAlloc_), locAllocAlloc_, locAllocDealloc_);

	//ZLOG_REGULAR("vMemoryPool_::vMemoryPool_: creating first memory chunk.");
	//addPhysicalChunk_(conf_.physicalChunk.maxChunkSize);
	ZLOG_REGULAR("vMemoryPool_::vMemoryPool_: memory pool created.");

}

zenith::vulkan::vMemoryPool_::~vMemoryPool_()
{
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryPool_::allocate(size_t sz, size_t alignment, uint32_t subType, bool dedicatedMemoryObject)
{
	std::lock_guard<std::mutex> lg(poolMutex_);
	if (dedicatedMemoryObject)
		return allocateUser_(sz);
	vMemorySubPool_ * sp = findSuitingPool_(sz, alignment, subType);
	vMemoryBlock res(nullptr, nullptr, util::memory::MemAllocBlock());
	if (sp)
	{
		res = allocateLogical_(sp, sz);
	}else if(sz >= conf_.physicalChunk.minAllocSize && (conf_.physicalChunk.align % alignment == 0))
	{
		res = allocatePhysical_(sz);
	}
	else
		throw vMemoryException("vMemoryPool_::allocate: failed to select suitable source of memory.");
	if (!res.physicalChunk_ || res.memBlk_.size < sz)
		throw vMemoryException_OutOfMemory("vMemoryPool_::allocate: failed to allocate memory.");
	return res;
}

void zenith::vulkan::vMemoryPool_::deallocate(const vMemoryBlock & blk)
{
	std::lock_guard<std::mutex> lg(poolMutex_);
	for(auto it = userChunks_.begin(); it != userChunks_.end(); it++)
		if (&(*it) == blk.physicalChunk_)
		{
			deallocateUser_(it, blk);
			return;
		}
	vMemorySubPool_ * sp = findOwningPool_(blk);
	if (sp)
	{
		deallocateLogical_(sp, blk);
	}else
	{
		deallocatePhysical_(blk);
	}
}

void zenith::vulkan::vMemoryPool_::addPhysicalChunk_(size_t sz)
{
	memChunks_.emplace_back(pDevice_->getDevice(), sz, memType_, conf_.physicalChunk.minAllocSize, conf_.physicalChunk.align, nullptr);
}

void zenith::vulkan::vMemoryPool_::removePhysicalChunk_(std::list<vMemoryChunkImpl_>::iterator it)
{
	memChunks_.erase(it);
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryPool_::allocateUser_(size_t sz)
{
	if (sz >= conf_.physicalChunk.minChunkSize && sz <= conf_.physicalChunk.maxChunkSize && (sz % conf_.physicalChunk.align == 0))
	{
		addPhysicalChunk_(sz);
		auto it = memChunks_.end(); it--;
		userChunks_.splice(userChunks_.end(), memChunks_, it); /*move node to userChunks_*/
		return vMemoryBlock(this, &userChunks_.back(), userChunks_.back().allocate(sz));
	}
	else throw vMemoryException("vMemoryPool_::allocateUser_: physical chunk constraints are not met.");
}

void zenith::vulkan::vMemoryPool_::deallocateUser_(std::list<vMemoryChunkImpl_>::iterator it, const vMemoryBlock & mb)
{
	mb.physicalChunk_->deallocate(mb.memBlk_);
	memChunks_.splice(memChunks_.end(), userChunks_, it); /*move node to memChunks_*/
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryPool_::allocatePhysical_(vMemoryChunkImpl_ * chnk, size_t sz)
{
	zenith::util::memory::MemAllocBlock blk0, blk;
	if (chnk->unusedMemory() < sz) /*early optimization*/
		return vMemoryBlock(nullptr, nullptr, blk0);
	try
	{
		blk = chnk->allocate(sz);
	}
	catch (zenith::vulkan::vMemoryException_OutOfMemory &outofmem) /*catch only out of mem excpetions*/
	{
		return vMemoryBlock(nullptr, nullptr, blk0);
	}
	if (blk.size < sz)
		return vMemoryBlock(nullptr, nullptr, blk0);

	return vMemoryBlock(this, chnk, blk);
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryPool_::allocatePhysical_(size_t sz)
{
	for (auto &chnk : memChunks_)
	{
		zenith::vulkan::vMemoryBlock res = allocatePhysical_(&chnk, sz);
		if (res.physicalChunk_ != nullptr && res.memBlk_.size >= sz)
			return res;
	}
	/*allocate new chunk*/
	addPhysicalChunk_(conf_.physicalChunk.maxChunkSize);
	return allocatePhysical_(&memChunks_.back(), sz);
}

void zenith::vulkan::vMemoryPool_::deallocatePhysical_(const vMemoryBlock & mb)
{
	mb.physicalChunk_->deallocate(mb.memBlk_);
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryPool_::allocateLogical_(vMemorySubPool_ * sp, size_t sz)
{
	zenith::util::memory::MemAllocBlock blk0;
	zenith::vulkan::vMemoryBlock res(nullptr, nullptr, blk0);
	if (sp->try_allocate(sz, res))
		return res;

	for (auto &chnk : memChunks_)
	{
		try
		{
			sp->addChunk(&chnk);
			return sp->allocate(sz); /*may be exception here, but we assigned chunk before*/
		}
		catch (zenith::vulkan::vMemoryException_OutOfMemory &outofmem) /*catch only out of mem excpetions*/
		{
			continue;
		}
	}

	addPhysicalChunk_(conf_.physicalChunk.maxChunkSize);
	sp->addChunk(&memChunks_.back());
	return sp->allocate(sz);
}

void zenith::vulkan::vMemoryPool_::deallocateLogical_(vMemorySubPool_ * sp, const vMemoryBlock & mb)
{
	sp->deallocate(mb);
}

zenith::vulkan::vMemorySubPool_ * zenith::vulkan::vMemoryPool_::findSuitingPool_(size_t size, size_t alignment, uint32_t subType)
{
	std::list<vMemorySubPool_>::iterator pIter = pools_.begin();
	for (size_t i = 0; i < conf_.logicalChunk.size(); i++, pIter++)
		if (size <= conf_.logicalChunk[i].maxAllocSize && ((subType & conf_.logicalChunk[i].subTypeMask) == subType) && (conf_.logicalChunk[i].align % alignment == 0))
			return &(*pIter);
	return nullptr;
}

zenith::vulkan::vMemorySubPool_ * zenith::vulkan::vMemoryPool_::findOwningPool_(const vMemoryBlock & mb)
{
	for (vMemorySubPool_ &sp : pools_)
		if (sp.owns(mb))
			return &sp;
	return nullptr;
}

zenith::vulkan::vMemoryPool_ * zenith::vulkan::vMemoryManagerImpl_::findSuitingPool_(uint32_t memoryTypeBits, VkMemoryPropertyFlags propertyFlags)
{
	auto it = pools_.begin();
	for (size_t i = 0; i < memProps_.memoryTypeCount; i++, it++)
		if (((1 << i) & memoryTypeBits) && ((memProps_.memoryTypes[i].propertyFlags & propertyFlags) == propertyFlags))
			return &(*it);
	return nullptr;
}

zenith::vulkan::vMemoryManagerImpl_::vMemoryManagerImpl_(vDeviceImpl_ * dev, const vMemoryManagerConfig & config, const std::vector<vMemoryPoolConfig>& pools) : dev_(dev), conf_(config)
{
	ZLOG_REGULAR("vMemoryManagerImpl_::vMemoryManagerImpl_: creating memory manager. Mapping memory types to memory pools.");
	VkPhysicalDevice phDev = dev_->getPhysicalDevice();
	vkGetPhysicalDeviceMemoryProperties(phDev, &memProps_);

	static const uint8_t NotMapped= 255;
	std::vector<uint8_t> poolMapping(memProps_.memoryTypeCount, NotMapped);

#define ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(x,y)if (x == zenith::util::ExtendedBitMask::YES && ((memProps_.memoryTypes[j].propertyFlags & y) == 0))continue;\
	if (x == zenith::util::ExtendedBitMask::NO && ((memProps_.memoryTypes[j].propertyFlags & y) != 0))continue;\

	for (size_t i = 0; i < conf_.memoryType.size(); i++)
	{
		const auto &lc = conf_.memoryType[i];
		for (size_t j = 0; j < poolMapping.size(); j++)
			if (poolMapping[j] != NotMapped)
				continue;
			else
			{
				if (lc.typeID >= 0 && lc.typeID != j)
					continue;
				
				ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(lc.isDeviceLocal, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(lc.isHostVisible, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
				ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(lc.isHostCoherent, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
				ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(lc.isHostCached, VK_MEMORY_PROPERTY_HOST_CACHED_BIT);
				ZENITH_VULKAN_CHECK_MEMTYPE_FLAG(lc.isLazy, VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT);

				//set pool
				for(size_t k = 0; k < pools.size(); k++)
					if (lc.poolUID == pools[k].uid)
					{
						poolMapping[j] = static_cast<uint8_t>(k);
						break;
					}
			}
	}
#undef ZENITH_VULKAN_CHECK_MEMTYPE_FLAG

	/*check that all pools are assigned*/
	for (auto x : poolMapping)
		if (x == NotMapped)
			throw vMemoryException("vMemoryManagerImpl_::vMemoryManagerImpl_: failed to assign memory types to memory pools.");

	ZLOG_REGULAR("vMemoryManagerImpl_::vMemoryManagerImpl_: memory types mapped. Creating usage mapping.");

	for (size_t i = 1; i < static_cast<size_t>(vMemoryUsage::MAX); i++)
	{
		size_t j = 0;
		for (; j < conf_.memoryUsage.size(); j++)
			if (static_cast<size_t>(conf_.memoryUsage[j].usage) == i)
				break;
		if (j == conf_.memoryUsage.size())
			continue;

		const auto &xs = conf_.memoryUsage[j].memoryUsageOption;
		usageMap_[i].reserve(xs.size());
		for (const auto &opt : xs)
		{			
			VkMemoryPropertyFlags flgs = 0;
			for (const auto &bt : opt.memoryPropertyBit)
				flgs |= static_cast<VkMemoryPropertyFlags>(bt);
			usageMap_[i].push_back(flgs);
		}
	}

	ZLOG_REGULAR("vMemoryManagerImpl_::vMemoryManagerImpl_: usage mapped. Creating individual memory pools.");

	for (size_t i = 0; i < poolMapping.size(); i++)
		pools_.emplace_back(dev_, pools[poolMapping[i]], i);

	ZLOG_REGULAR("vMemoryManagerImpl_::vMemoryManagerImpl_: memory manager created.");
}

zenith::vulkan::vMemoryBlock zenith::vulkan::vMemoryManagerImpl_::allocate(size_t size, size_t alignment, uint32_t memoryTypeBits, vMemoryUsage usage, vMemoryLayout layout, bool dedicatedMemory)
{
	const auto &usg = usageMap_[static_cast<size_t>(usage)];
	vMemoryPool_ * ptr = nullptr;
	for (size_t i = 0; i < usg.size(); i++)
	{
		ptr = findSuitingPool_(memoryTypeBits, usg[i]);
		if (ptr)
			break;
	}
	if (!ptr)
		throw vMemoryException("vMemoryManagerImpl_::allocate: failed to find suiting memory pool.");
	auto res = ptr->allocate(size, alignment, static_cast<uint32_t>(layout), dedicatedMemory);
	if (res.offset() & (alignment - 1) != 0)
	{
		ptr->deallocate(res);
		throw vMemoryException_NotAligned("vMemoryManagerImpl_::allocate: alignment requirement violated.");
	}
	return res;
}

void zenith::vulkan::vMemoryManagerImpl_::deallocate(const zenith::vulkan::vMemoryBlock & blk)
{
	if (blk.memoryPool_)
		blk.memoryPool_->deallocate(blk);
}

void zenith::vulkan::vMemoryManager::create(vDevice & dev, const vMemoryConfig & conf)
{
	for (size_t i = 0; i < conf.memoryManagerReqOrder.size(); i++)
	{
		size_t idx = 0;
		for (; idx < conf.memoryManager.size(); idx++)
			if (conf.memoryManager[idx].uid == conf.memoryManagerReqOrder[i])
				break;
		if (idx == conf.memoryManager.size())
			continue;
		try
		{
			Impl::create(dev.raw(), conf.memoryManager[idx], conf.memoryPool);
			return;
		}
		catch (const VulkanException &)
		{
			if (i + 1 >= conf.memoryManagerReqOrder.size())
				throw;
			zenith::util::zLOG::log(zenith::util::LogType::WARNING, "vMemoryManager::create: Failed attempt at creating of memory manager. Retrying another config.");
		}
	}
	throw vMemoryException("vMemoryManager::create: none of tested configs of devices were statisfied!");
}
