#include "vMemoryBase.h"
#include "vMemoryManager.h"

void zenith::vulkan::vMemoryBlockAuto::free_()
{
	if (physicalChunk_ && memoryPool_ && memBlk_.size > 0)
		memoryPool_->deallocate(vMemoryBlock(memoryPool_, physicalChunk_, memBlk_));
	memoryPool_ = nullptr;
	physicalChunk_ = nullptr;
	memBlk_ = util::memory::MemAllocBlock();
}

void * zenith::vulkan::vMemoryBlockAuto::map(size_t offset, size_t size)
{
	if (!physicalChunk_)
		throw vMemoryException("vMemoryBlockAuto::map: memory block is not valid!");
	if (size != WholeSize && offset + size > memBlk_.size)
		throw vMemoryException("vMemoryBlockAuto::map: size exceeds block size!");
	if (size == WholeSize && offset >= memBlk_.size)
		throw vMemoryException("vMemoryBlockAuto::map: offset exceeds block size!");
	return physicalChunk_->map(this->offset() + offset, size);
}

void zenith::vulkan::vMemoryBlockAuto::unmap()
{
	if (!physicalChunk_)
		throw vMemoryException("vMemoryBlockAuto::unmap: memory block is not valid!");
	physicalChunk_->unmap();
}

VkDeviceMemory zenith::vulkan::vMemoryBlockAuto::handle() const
{
	if (physicalChunk_)
		return physicalChunk_->handle();
	else throw vMemoryException("vMemoryBlockAuto::handle: no associated physical chunk!");
}

void * zenith::vulkan::vMemoryBlock::map(size_t offset, size_t size)
{
	if(!physicalChunk_)
		throw vMemoryException("vMemoryBlock::map: memory block is not valid!");
	if (size != WholeSize && offset + size > memBlk_.size)
		throw vMemoryException("vMemoryBlock::map: size exceeds block size!");
	if (size == WholeSize && offset >= memBlk_.size)
		throw vMemoryException("vMemoryBlock::map: offset exceeds block size!");
	return physicalChunk_->map(this->offset() + offset, size);
}

void zenith::vulkan::vMemoryBlock::unmap()
{
	if (!physicalChunk_)
		throw vMemoryException("vMemoryBlock::unmap: memory block is not valid!");
	physicalChunk_->unmap();
}

VkDeviceMemory zenith::vulkan::vMemoryBlock::handle() const
{

	if (physicalChunk_)
		return physicalChunk_->handle();
	else throw vMemoryException("vMemoryBlock::handle: no associated physical chunk!");
}
