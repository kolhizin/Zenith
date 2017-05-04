#include "vSampler.h"



void zenith::vulkan::vSamplerAddressing::check_() const
{
	if (useBorder())
	{
		if ((conf_.borderType != vSamplerBorderType::FLOAT) && (conf_.borderType != vSamplerBorderType::INT))
			throw vSamplerException("vSamplerAddressing::check_: unsupported border type.");
		if ((conf_.borderColor != vSamplerBorderColor::OPAQUE_BLACK) && (conf_.borderColor != vSamplerBorderColor::OPAQUE_WHITE) && (conf_.borderColor != vSamplerBorderColor::TRANSPARENT_BLACK))
			throw vSamplerException("vSamplerAddressing::check_: unsupported border color.");
	}
	if (!conf_.normalizedCoord)
	{
		if((conf_.u != vSamplerAddress::CLAMP) && (conf_.u != vSamplerAddress::CLAMP_BORDER))
			throw vSamplerException("vSamplerAddressing::check_: unsupported u-addressing for unnormalized coords.");
		if ((conf_.v != vSamplerAddress::CLAMP) && (conf_.v != vSamplerAddress::CLAMP_BORDER))
			throw vSamplerException("vSamplerAddressing::check_: unsupported v-addressing for unnormalized coords.");
		if ((conf_.w != vSamplerAddress::CLAMP) && (conf_.w != vSamplerAddress::CLAMP_BORDER))
			throw vSamplerException("vSamplerAddressing::check_: unsupported w-addressing for unnormalized coords.");
	}
}

void zenith::vulkan::vSamplerAddressing::vkSetUpCreateInfo(VkSamplerCreateInfo & ci) const
{
	ci.addressModeU = toVkSamplerAddressMode(conf_.u);
	ci.addressModeV = toVkSamplerAddressMode(conf_.v);
	ci.addressModeW = toVkSamplerAddressMode(conf_.w);

	ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	/*all combinations checked during creation*/
	if (useBorder())
	{
		if (conf_.borderType == vSamplerBorderType::FLOAT)
		{
			switch (conf_.borderColor)
			{
			case vSamplerBorderColor::OPAQUE_BLACK: ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK; break;
			case vSamplerBorderColor::OPAQUE_WHITE: ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE; break;
			case vSamplerBorderColor::TRANSPARENT_BLACK: ci.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK; break;
			}
		}
		else if (conf_.borderType == vSamplerBorderType::INT)
		{
			switch (conf_.borderColor)
			{
			case vSamplerBorderColor::OPAQUE_BLACK: ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; break;
			case vSamplerBorderColor::OPAQUE_WHITE: ci.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE; break;
			case vSamplerBorderColor::TRANSPARENT_BLACK: ci.borderColor = VK_BORDER_COLOR_INT_TRANSPARENT_BLACK; break;
			}
		}
	}
		

	ci.unnormalizedCoordinates = (conf_.normalizedCoord ? VK_FALSE : VK_TRUE);
}

void zenith::vulkan::vSamplerFiltering::check_() const
{
	if (conf_.maxAnisotropy < 0.0f)
		throw vSamplerException("vSamplerFiltering::check_: negative anisotropy is not supported.");
	if (conf_.maxAnisotropy > 128.0f)
		throw vSamplerException("vSamplerFiltering::check_: too high anisotropy.");
}

void zenith::vulkan::vSamplerFiltering::vkSetUpCreateInfo(VkSamplerCreateInfo & ci) const
{
	ci.minFilter = toVkFilter(conf_.minFilter);
	ci.magFilter = toVkFilter(conf_.magFilter);
	ci.mipmapMode = toVkSamplerMipmapMode(conf_.mipFilter);

	ci.anisotropyEnable = (isAnisotropic() ? VK_TRUE : VK_FALSE);
	ci.maxAnisotropy = conf_.maxAnisotropy;

	ci.compareOp = VK_COMPARE_OP_NEVER;
	ci.compareEnable = VK_FALSE;
}

void zenith::vulkan::vSamplerLOD::check_() const
{
	if (conf_.minLOD < 0.0f)
		throw vSamplerException("vSamplerFiltering::check_: negative min LOD is not supported.");
	if (conf_.maxLOD < conf_.minLOD)
		throw vSamplerException("vSamplerFiltering::check_: max LOD should not be lower than min LOD.");
}

void zenith::vulkan::vSamplerLOD::vkSetUpCreateInfo(VkSamplerCreateInfo & ci) const
{
	ci.minLod = conf_.minLOD;
	ci.maxLod = conf_.maxLOD;
	ci.mipLodBias = conf_.biasLOD;
}

zenith::vulkan::vSamplerImpl_::vSamplerImpl_(vDeviceImpl_ * dev, vSamplerFiltering filtering, vSamplerAddressing addressing, vSamplerLOD lod) : dev_(dev)
{
	VkSamplerCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	filtering.vkSetUpCreateInfo(ci);
	addressing.vkSetUpCreateInfo(ci);
	lod.vkSetUpCreateInfo(ci);

	VkResult res = vkCreateSampler(dev_->getDevice(), &ci, nullptr, &sampler_);
	if (res != VK_SUCCESS)
		throw vSamplerException(res, "vSamplerImpl_::vSamplerImpl_: failed to create sampler.");
}

zenith::vulkan::vSamplerImpl_::~vSamplerImpl_()
{
	vkDestroySampler(dev_->getDevice(), sampler_, nullptr);
}
