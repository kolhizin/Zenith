#include "vSwapchain.h"
#include "vDevice.h"
#include <Utils\bit_utils.h>
#include <limits>

#undef max

void zenith::vulkan::vSwapchainImpl_::loadFunctions_()
{
	_vkCreateSwapchainKHR = nullptr;	
	_vkDestroySwapchainKHR = nullptr;

	ZLOG_REGULAR("vSwapchainImpl_::loadFunctions_: Loading functions.");

	_vkCreateSwapchainKHR = reinterpret_cast<PFN_vkCreateSwapchainKHR>(vkGetDeviceProcAddr(device_, "vkCreateSwapchainKHR"));
	_vkDestroySwapchainKHR = reinterpret_cast<PFN_vkDestroySwapchainKHR>(vkGetDeviceProcAddr(device_, "vkDestroySwapchainKHR"));
	_vkGetSwapchainImagesKHR = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(vkGetDeviceProcAddr(device_, "vkGetSwapchainImagesKHR"));

	_vkAcquireNextImageKHR = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device_, "vkAcquireNextImageKHR"));
	_vkQueuePresentKHR = reinterpret_cast<PFN_vkQueuePresentKHR>(vkGetDeviceProcAddr(device_, "vkQueuePresentKHR"));
	
	if (!_vkCreateSwapchainKHR || !_vkDestroySwapchainKHR)
		throw zenith::vulkan::vSurfaceException("vSwapchainImpl_::loadFunctions_: Could not load _vkCreateSwapchainKHR or _vkDestroySwapchainKHR functions!");

	if (!_vkGetSwapchainImagesKHR || !_vkAcquireNextImageKHR || !_vkQueuePresentKHR)
		throw zenith::vulkan::vSurfaceException("vSwapchainImpl_::loadFunctions_: Could not load _vkGetSwapchainImagesKHR or _vkAcquireNextImageKHR or _vkQueuePresentKHR functions!");

	ZLOG_REGULAR("vSwapchainImpl_::loadFunctions_: Functions loaded.");
}

VkResult zenith::vulkan::vSwapchainImpl_::createSwapchain_(const VkAllocationCallbacks * pA, VkSwapchainKHR oldSwapchain)
{
	VkSwapchainCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	ci.pNext = nullptr;
	ci.flags = 0;

	auto caps = pSurface_->getPhysicalDeviceSurfaceCapabilities(pDevice_->getPhysicalDevice());
	
	ci.surface = pSurface_->getSurface();

	if (caps.maxImageCount > 0 && caps.maxImageCount < req_.imageCount)
		throw vSwapchainException("vSwapchainImpl_::createSwapchain_: number of requested images exceeds device capabilities!");

	auto iFormats(std::move(pSurface_->getPhysicalDeviceSurfaceFormats(pDevice_->getPhysicalDevice())));
	bool foundFormat = false;
	for (const auto &x : iFormats)
		if (x.format == static_cast<VkFormat>(req_.imageFormat))
		{
			foundFormat = true;
			break;
		}
	if(!foundFormat)
		throw vSwapchainException("vSwapchainImpl_::createSwapchain_: required image format is not supported!");
		
	auto pModes(std::move(pSurface_->getPhysicalDeviceSurfacePresentModes(pDevice_->getPhysicalDevice())));
	bool foundMode = false;
	for (const auto &x : pModes)
		if (x == static_cast<VkPresentModeKHR>(req_.presentMode))
		{
			foundMode = true;
			break;
		}
	if (!foundMode)
		throw vSwapchainException("vSwapchainImpl_::createSwapchain_: required present mode is not supported!");
	
	ci.minImageCount = req_.imageCount;
	ci.imageFormat = static_cast<VkFormat>(req_.imageFormat);
	ci.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	ci.presentMode = static_cast<VkPresentModeKHR>(req_.presentMode);
	ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	ci.imageExtent = extent_;	

	ci.imageArrayLayers = 1;
	ci.imageUsage = zenith::util::collection2bitmask<VkImageUsageFlags>(req_.imageUsage);
	
	std::vector<uint32_t> fIndices_; fIndices_.reserve(5);
	for (const std::string &qName : req_.sharingInfo.queues)
	{
		uint32_t ind = pDevice_->getQueueFamilyIndex(qName);
		if (std::find(fIndices_.begin(), fIndices_.end(), ind) == fIndices_.end())
			fIndices_.push_back(ind);
	}

	if (req_.sharingInfo.mode == vSharingMode::EXCLUSIVE)
	{		
		ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		if (fIndices_.size() > 1)
		{
			ZLOG_WARNING("vSwapchainImpl_::createSwapchain_: specified exclusive mode and more than one queue families!");
			ci.queueFamilyIndexCount = static_cast<uint32_t>(fIndices_.size());
			ci.pQueueFamilyIndices = &fIndices_[0];
		}
		else
		{
			ci.queueFamilyIndexCount = 0;
			ci.pQueueFamilyIndices = nullptr;
		}
		
	}
	else if (req_.sharingInfo.mode == vSharingMode::CONCURRENT)
	{
		if (fIndices_.size() <= 1)
			throw vSwapchainException("vSwapchainImpl_::createSwapchain_: for concurrent sharing mode you must specify at least 2 distinct queue family indices!");

		ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;

		ci.queueFamilyIndexCount = static_cast<uint32_t>(fIndices_.size());
		ci.pQueueFamilyIndices = &fIndices_[0];	
	}else
		throw vSwapchainException("vSwapchainImpl_::createSwapchain_: specified unknown sharing mode!");
	
	ci.preTransform = caps.currentTransform;

	ci.clipped = VK_TRUE;

	ci.oldSwapchain = oldSwapchain;

	VkResult res = _vkCreateSwapchainKHR(device_, &ci, pA, &swapchain_.replace());

	if (res != VK_SUCCESS)
		throw vSwapchainException(res, "vSwapchainImpl_::createSwapchain_: failed to create swapchain.");

	getImages_(pA);
	createImageViews_(pA);
	
	return res;
}

void zenith::vulkan::vSwapchainImpl_::destroySwapchain_(const VkAllocationCallbacks * pA)
{
	_vkDestroySwapchainKHR(device_, swapchain_, pA);
}

void zenith::vulkan::vSwapchainImpl_::getImages_(const VkAllocationCallbacks * pA)
{
	VkResult res;
	uint32_t imgCount = 0;

	ZLOG_REGULAR("vSwapchainImpl_::getImages_: Getting images from swapchain.");
	res = _vkGetSwapchainImagesKHR(device_, swapchain_, &imgCount, nullptr);
	if (res != VK_SUCCESS)
		throw vSwapchainException(res, "vSwapchainImpl_::getImages_: failed to get count of images from swapchain.");
	if (imgCount < req_.imageCount)
	{
		_vkDestroySwapchainKHR(device_, swapchain_, pA);
		throw vSwapchainException("vSwapchainImpl_::getImages_: got fewer images than expected from swapchain.");
	}
	images_.resize(imgCount);
	res = _vkGetSwapchainImagesKHR(device_, swapchain_, &imgCount, &images_[0]);
	if (res != VK_SUCCESS)
		throw vSwapchainException(res, "vSwapchainImpl_::getImages_: failed to get images from swapchain.");
	ZLOG_REGULAR("vSwapchainImpl_::getImages_: Images queried from swapchain.");
}

void zenith::vulkan::vSwapchainImpl_::createImageViews_(const VkAllocationCallbacks * pA)
{
	ZLOG_REGULAR("vSwapchainImpl_::createImageViews_: Creating image views.");
	VkResult res;
	views_.reserve(images_.size());
	for (size_t i = 0; i < images_.size(); i++)
	{
		VkImageViewCreateInfo ci = {};
		ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ci.flags = 0;
		ci.pNext = nullptr;

		ci.format = static_cast<VkFormat>(req_.imageFormat);
		ci.image = images_[i];
		ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ci.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ci.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ci.subresourceRange.baseArrayLayer = 0;
		ci.subresourceRange.layerCount = 1;

		ci.subresourceRange.baseMipLevel = 0;
		ci.subresourceRange.levelCount = 1;

		views_.emplace_back(vkDestroyImageView, device_, pA);

		res = vkCreateImageView(device_, &ci, pA, &views_.back().replace());
		if (res != VK_SUCCESS)
			throw vSwapchainException(res, "vSwapchainImpl_::getImages_: failed create image views.");
	}
	ZLOG_REGULAR("vSwapchainImpl_::createImageViews_: Images views created.");
}

zenith::vulkan::vSwapchainImpl_::vSwapchainImpl_(const vSwapchainReqs &req, const VkExtent2D &extent, const vDeviceImpl_ * dev, const vSurfaceImpl_ * surf)
	: device_(dev->device_), req_(req), pDevice_(dev), pSurface_(surf), extent_(extent), swapchain_(nullptr, dev->device_, nullptr)
{	
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Creating swapchain.");
	loadFunctions_();
	swapchain_.replaceDeleter(_vkDestroySwapchainKHR);	
	createSwapchain_(nullptr);
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Swapchain created.");
}

zenith::vulkan::vSwapchainImpl_::vSwapchainImpl_(const vSwapchainReqs & req, const VkExtent2D & extent, const vDeviceImpl_ * dev, const vSwapchainImpl_ * old)
	: device_(dev->device_), req_(req), pDevice_(dev), pSurface_(old->pSurface_), extent_(extent), swapchain_(nullptr, dev->device_, nullptr)
{
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Creating swapchain (from existing swapchain).");
	loadFunctions_();
	swapchain_.replaceDeleter(_vkDestroySwapchainKHR);
	createSwapchain_(nullptr, old->swapchain_);
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Swapchain created (from existing swapchain).");
}

zenith::vulkan::vSwapchainImpl_::vSwapchainImpl_(const VkExtent2D & extent, const vSwapchainImpl_ * old)
	: device_(old->device_), req_(old->req_), pDevice_(old->pDevice_), pSurface_(old->pSurface_), extent_(extent), swapchain_(nullptr, old->device_, nullptr)
{
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Creating swapchain (from existing swapchain).");
	loadFunctions_();
	swapchain_.replaceDeleter(_vkDestroySwapchainKHR);
	createSwapchain_(nullptr, old->swapchain_);
	ZLOG_REGULAR("vSwapchainImpl_::vSwapchainImpl_: Swapchain created (from existing swapchain).");
}

zenith::vulkan::vSwapchainImpl_::~vSwapchainImpl_()
{
	ZLOG_REGULAR("vSwapchainImpl_::~vSwapchainImpl_: Destroying image views.");
	views_.clear();
	ZLOG_REGULAR("vSwapchainImpl_::~vSwapchainImpl_: Views destroyed. Destroying swapchain.");	
	swapchain_.replace();
	ZLOG_REGULAR("vSwapchainImpl_::~vSwapchainImpl_: Swapchain destroyed.");
}

uint32_t zenith::vulkan::vSwapchainImpl_::acquireNextImage(VkSemaphore s, VkFence f, vSwapchainResult * info)
{
	uint32_t imgInd = 0;
	VkResult res;
	res = _vkAcquireNextImageKHR(device_, swapchain_, std::numeric_limits<uint64_t>::max(), s, f, &imgInd);
	if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR || res == VK_TIMEOUT || res == VK_NOT_READY)
	{
		if (info)
			*info = static_cast<vSwapchainResult>(res);
		return imgInd;
	}
	throw vSwapchainException(res, "vSwapchainImpl_::acquireNextImage: error during vkAcquireNextImageKHR.");
}

void zenith::vulkan::vSwapchainImpl_::queuePresent(VkQueue q, uint32_t imgInd, vSwapchainResult * info)
{
	uint32_t tmpImgInd = imgInd;
	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;

	pi.swapchainCount = 1;
	pi.pSwapchains = &swapchain_;

	pi.waitSemaphoreCount = 0;
	pi.pWaitSemaphores = nullptr;

	pi.pImageIndices = &tmpImgInd;
	pi.pResults = nullptr;

	VkResult res = _vkQueuePresentKHR(q, &pi);
	if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR || res == VK_TIMEOUT || res == VK_NOT_READY)
	{
		if (info)
			*info = static_cast<vSwapchainResult>(res);
		imgInd;
	}
}

void zenith::vulkan::vSwapchainImpl_::queuePresent(VkQueue q, uint32_t imgInd, VkSemaphore signalSemaphore, vSwapchainResult * info)
{
	uint32_t tmpImgInd = imgInd;
	VkSemaphore tmpSem = signalSemaphore;
	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;

	pi.swapchainCount = 1;
	pi.pSwapchains = &swapchain_;

	pi.waitSemaphoreCount = 1;
	pi.pWaitSemaphores = &tmpSem;

	pi.pImageIndices = &tmpImgInd;
	pi.pResults = nullptr;

	VkResult res = _vkQueuePresentKHR(q, &pi);
	if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR || res == VK_TIMEOUT || res == VK_NOT_READY)
	{
		if (info)
			*info = static_cast<vSwapchainResult>(res);
		imgInd;
	}
}

void zenith::vulkan::vSwapchainImpl_::queuePresent(VkQueue q, uint32_t imgInd, std::vector<VkSemaphore>& signalSemaphore, vSwapchainResult * info)
{
	uint32_t tmpImgInd = imgInd;
	VkPresentInfoKHR pi = {};
	pi.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	pi.pNext = nullptr;
	
	pi.swapchainCount = 1;
	pi.pSwapchains = &swapchain_;

	pi.waitSemaphoreCount = static_cast<uint32_t>(signalSemaphore.size());
	pi.pWaitSemaphores = &signalSemaphore[0];

	pi.pImageIndices = &tmpImgInd;
	pi.pResults = nullptr;

	VkResult res = _vkQueuePresentKHR(q, &pi);
	if (res == VK_SUCCESS || res == VK_SUBOPTIMAL_KHR || res == VK_TIMEOUT || res == VK_NOT_READY)
	{
		if (info)
			*info = static_cast<vSwapchainResult>(res);
		imgInd;
	}
}

zenith::vulkan::vSwapchain::vSwapchain(const vDeviceImpl_ * device, const vSurface & surface, const vSwapchainConfig & conf, const VkExtent2D & size)
{
	create(device, surface, conf, size);
}

void zenith::vulkan::vSwapchain::create(const vDeviceImpl_ * device, const vSurface & surface, const vSwapchainConfig & conf, const VkExtent2D & size)
{
	for (size_t i = 0; i < conf.swapchainReqOrder.size(); i++)
	{
		size_t idx = 0;
		for (; idx < conf.swapchainReqs.size(); idx++)
			if (conf.swapchainReqs[idx].uid == conf.swapchainReqOrder[i])
				break;
		if (idx == conf.swapchainReqs.size())
			continue;
		try
		{
			Impl::create(conf.swapchainReqs[idx], size, device, surface.rawImpl());
			return;
		}
		catch (const VulkanException &)
		{
			if (i + 1 >= conf.swapchainReqOrder.size())
				throw;
			zenith::util::zLOG::log(zenith::util::LogType::WARNING, "vSwapchain::create: Failed attempt at creating of swapchain. Retrying another config.");
		}
	}
	throw vDeviceException("vSwapchain::create: none of tested configs of swapchain were statisfied!");
}

void zenith::vulkan::vSwapchain::recreate(const VkExtent2D & size)
{
	util::pimpl<vSwapchainImpl_> tmp(size, Impl::get());
	Impl::operator=(std::move(tmp));
}
