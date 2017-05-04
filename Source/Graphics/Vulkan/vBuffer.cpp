
#include "vBuffer.h"

zenith::vulkan::vBufferRawImpl_::vBufferRawImpl_(vDeviceImpl_ * dev, size_t size, VkBufferUsageFlags usageFlags, vObjectSharingInfo objSharing) : sharingInfo_(objSharing), dev_(dev), memBound_(false)
{
	

	VkBufferCreateInfo bci = {};
	bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bci.flags = 0;
	bci.pNext = nullptr;

	std::vector<uint32_t> qfs;
	bci.sharingMode = sharingInfo_.vkSharingMode();
	if (bci.sharingMode == VK_SHARING_MODE_CONCURRENT)
	{
		qfs = std::move(sharingInfo_.supportedQueueFamilies());
		bci.queueFamilyIndexCount = qfs.size();
		bci.pQueueFamilyIndices = qfs.data();
	}
	
	bci.size = size;
	bci.usage = usageFlags;

	VkResult res = vkCreateBuffer(dev_->getDevice(), &bci, nullptr, &buffer_);
	if (res != VK_SUCCESS)
		throw vBufferException(res, "vBufferRawImpl_::vBufferRawImpl_: failed to create buffer.");
}

zenith::vulkan::vBufferRawImpl_::~vBufferRawImpl_()
{
	vkDestroyBuffer(dev_->getDevice(), buffer_, nullptr);
}

VkMemoryRequirements zenith::vulkan::vBufferRawImpl_::getMemoryRequirements() const
{
	if(memBound_)
		throw vBufferException("vBufferRawImpl_::getMemoryRequirements: memory already bound.");
	VkMemoryRequirements res;
	vkGetBufferMemoryRequirements(dev_->getDevice(), buffer_, &res);
	return res;
}

void zenith::vulkan::vBufferRawImpl_::bindMemory(const vMemoryBlock & blk, size_t offset)
{
	if (memBound_)
		throw vBufferException("vBufferRawImpl_::bindMemory: memory already bound.");

	VkMemoryRequirements mrq;
	vkGetBufferMemoryRequirements(dev_->getDevice(), buffer_, &mrq);

	if((blk.offset() + offset) % mrq.alignment)
		throw vBufferException("vBufferRawImpl_::bindMemory: alignment requirements violated.");

	VkResult res = vkBindBufferMemory(dev_->getDevice(), buffer_, blk.handle(), blk.offset() + offset);
	if (res != VK_SUCCESS)
		throw vBufferException(res, "vBufferRawImpl_::vBufferRawImpl_: failed to create buffer.");

	memBound_ = true;
}

void zenith::vulkan::vBufferRawImpl_::bindMemory(const vMemoryBlockAuto & blk, size_t offset)
{
	if (memBound_)
		throw vBufferException("vBufferRawImpl_::bindMemory: memory already bound.");

	VkMemoryRequirements mrq;
	vkGetBufferMemoryRequirements(dev_->getDevice(), buffer_, &mrq);

	if ((blk.offset() + offset) % mrq.alignment)
		throw vBufferException("vBufferRawImpl_::bindMemory: alignment requirements violated.");

	VkResult res = vkBindBufferMemory(dev_->getDevice(), buffer_, blk.handle(), blk.offset() + offset);
	if (res != VK_SUCCESS)
		throw vBufferException(res, "vBufferRawImpl_::vBufferRawImpl_: failed to create buffer.");

	memBound_ = true;
}
