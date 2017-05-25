#include "vTexture.h"

zenith::vulkan::vTextureRawImpl_::vTextureRawImpl_(vDeviceImpl_ * dev, vTextureDimensions size, VkFormat format, VkImageUsageFlags usageFlags, vObjectSharingInfo objSharing, VkImageLayout initialLayout, bool optimalTiling) :
	sharingInfo_(objSharing), dev_(dev), memBound_(false), size_(size), optimalTiling_(optimalTiling), format_(format), mainViewCreated_(false)
{
	VkImageCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	ci.pNext = nullptr;

	ci.flags = 0;

	ci.extent.width = size_.width();
	ci.extent.height = size_.height();
	ci.extent.depth = size_.depth();
	ci.imageType = size_.vkImageType();

	ci.arrayLayers = size_.arraySize();
	ci.mipLevels = size_.mipLevels();

	ci.format = format_;
	ci.tiling = (optimalTiling_ ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR);

	std::vector<uint32_t> qfs;
	ci.sharingMode = sharingInfo_.vkSharingMode();
	if (ci.sharingMode == VK_SHARING_MODE_CONCURRENT)
	{
		qfs = std::move(sharingInfo_.supportedQueueFamilies());
		ci.queueFamilyIndexCount = static_cast<uint32_t>(qfs.size());
		ci.pQueueFamilyIndices = qfs.data();
	}

	ci.initialLayout = initialLayout;
	ci.samples = VK_SAMPLE_COUNT_1_BIT;
	ci.usage = usageFlags;

	if (size_.arraySize() > 1)
	{
		if (size_.dim() == 1)
			mainTextureViewType_ = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
		else if (size_.dim() == 2)
			mainTextureViewType_ = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		else throw vTextureException("vTextureRawImpl_::vTextureRawImpl_: 3d texture arrays are not supported.");
	}
	else
	{
		if (size_.dim() == 1)
			mainTextureViewType_ = VK_IMAGE_VIEW_TYPE_1D;
		else if (size_.dim() == 2)
			mainTextureViewType_ = VK_IMAGE_VIEW_TYPE_2D;
		else if (size_.dim() == 3)
			mainTextureViewType_ = VK_IMAGE_VIEW_TYPE_2D;
		else throw vTextureException("vTextureRawImpl_::vTextureRawImpl_: unsupported texture dimension.");
	}

	mainTextureViewAspect_ = VK_IMAGE_ASPECT_COLOR_BIT;
	if (format_ == VK_FORMAT_D16_UNORM)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (format_ == VK_FORMAT_X8_D24_UNORM_PACK32)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (format_ == VK_FORMAT_D32_SFLOAT)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (format_ == VK_FORMAT_S8_UINT)mainTextureViewAspect_ = VK_IMAGE_ASPECT_STENCIL_BIT;
	if (format_ == VK_FORMAT_D16_UNORM_S8_UINT)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if (format_ == VK_FORMAT_D24_UNORM_S8_UINT)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	if (format_ == VK_FORMAT_D32_SFLOAT_S8_UINT)mainTextureViewAspect_ = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

	VkResult res = vkCreateImage(dev_->getDevice(), &ci, nullptr, &image_);
	if (res != VK_SUCCESS)
		throw vTextureException(res, "vTextureRawImpl_::vTextureRawImpl_: failed to create image.");
}

zenith::vulkan::vTextureRawImpl_::~vTextureRawImpl_()
{
	getMainView_()->~vTextureViewRawImpl_();
	vkDestroyImage(dev_->getDevice(), image_, nullptr);
}

VkMemoryRequirements zenith::vulkan::vTextureRawImpl_::getMemoryRequirements() const
{
	if (memBound_)
		throw vTextureException("vTextureRawImpl_::getMemoryRequirements: memory already bound.");
	VkMemoryRequirements res;
	VkDevice dev = dev_->getDevice();
	vkGetImageMemoryRequirements(dev, image_, &res);
	return res;
}

void zenith::vulkan::vTextureRawImpl_::createMainView()
{
	if(mainViewCreated_)
		throw vTextureException("vTextureRawImpl_::createMainView: main view already created.");
	new (mainTextureViewBuffer_) vTextureViewRawImpl_(this, format_, mainTextureViewType_, mainTextureViewAspect_, vTextureSubresource::Full(size_));
	mainViewCreated_ = true;
}

void zenith::vulkan::vTextureRawImpl_::bindMemory(const vMemoryBlock & blk, size_t offset)
{
	if (memBound_)
		throw vTextureException("vTextureRawImpl_::bindMemory: memory already bound.");

	VkMemoryRequirements mrq;
	vkGetImageMemoryRequirements(dev_->getDevice(), image_, &mrq);

	if ((blk.offset() + offset) % mrq.alignment)
		throw vTextureException("vTextureRawImpl_::bindMemory: alignment requirements violated.");

	VkResult res = vkBindImageMemory(dev_->getDevice(), image_, blk.handle(), blk.offset() + offset);
	if (res != VK_SUCCESS)
		throw vTextureException(res, "vTextureRawImpl_::vBufferRawImpl_: failed to create buffer.");

	memBound_ = true;
}

void zenith::vulkan::vTextureRawImpl_::bindMemory(const vMemoryBlockAuto & blk, size_t offset)
{
	if (memBound_)
		throw vTextureException("vTextureRawImpl_::bindMemory: memory already bound.");

	VkMemoryRequirements mrq;
	vkGetImageMemoryRequirements(dev_->getDevice(), image_, &mrq);

	if ((blk.offset() + offset) % mrq.alignment)
		throw vTextureException("vTextureRawImpl_::bindMemory: alignment requirements violated.");

	VkResult res = vkBindImageMemory(dev_->getDevice(), image_, blk.handle(), blk.offset() + offset);
	if (res != VK_SUCCESS)
		throw vTextureException(res, "vTextureRawImpl_::vBufferRawImpl_: failed to create buffer.");

	memBound_ = true;
}

zenith::vulkan::vTextureViewRawImpl_::vTextureViewRawImpl_(vTextureRawImpl_ * texture, VkFormat format, VkImageViewType viewType, VkImageAspectFlags imageAspect, vTextureSubresource subRes)
	: texture_(texture), subres_(subRes)
{
	if(!subres_.validFor(texture->getDimensions()))
		throw vTextureException("vTextureViewRawImpl_::vTextureViewRawImpl_: subresource range is not suitable for texture dimensions.");

	VkImageViewCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;
	ci.subresourceRange = subres_.vkImageSubresourceRange(imageAspect);
	ci.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
	ci.format = format;
	ci.viewType = viewType;
	ci.image = texture->image_;

	auto res = vkCreateImageView(texture_->dev_->getDevice(), &ci, nullptr, &view_);
	if (res != VK_SUCCESS)
		throw vTextureException(res, "vTextureViewRawImpl_::vTextureViewRawImpl_: failed to create image view.");
}

zenith::vulkan::vTextureViewRawImpl_::~vTextureViewRawImpl_()
{
	vkDestroyImageView(texture_->dev_->getDevice(), view_, nullptr);
}
