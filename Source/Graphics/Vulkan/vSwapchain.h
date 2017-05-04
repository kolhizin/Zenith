#pragma once
#include "vulkan_config.h"
#include "vSurface.h"
#include "vUtil.h"

namespace zenith
{
	namespace vulkan
	{
		class vSwapchainException : public VulkanException
		{
		public:
			vSwapchainException() : VulkanException() {}
			vSwapchainException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vSwapchainException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vSwapchainException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vSwapchainException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vSwapchainException() {}
		};

		class vDeviceImpl_;
		class vSwapchainImpl_
		{
			
			vAutoVar<VkSwapchainKHR> swapchain_;
			VkDevice device_;
			vSwapchainReqs req_;

			VkExtent2D extent_;

			const vDeviceImpl_ * pDevice_;
			const vSurfaceImpl_ * pSurface_;

			PFN_vkCreateSwapchainKHR _vkCreateSwapchainKHR;
			PFN_vkDestroySwapchainKHR _vkDestroySwapchainKHR;
			PFN_vkGetSwapchainImagesKHR _vkGetSwapchainImagesKHR;

			PFN_vkAcquireNextImageKHR _vkAcquireNextImageKHR;
			PFN_vkQueuePresentKHR _vkQueuePresentKHR;

			std::vector<VkImage> images_;
			std::vector< vAutoVar<VkImageView> > views_;
			
			void loadFunctions_();

			VkResult createSwapchain_(const VkAllocationCallbacks * pA, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
			
			void destroySwapchain_(const VkAllocationCallbacks * pA);
			void getImages_(const VkAllocationCallbacks * pA);
			void createImageViews_(const VkAllocationCallbacks * pA);

		public:
			vSwapchainImpl_(const vSwapchainReqs &req, const VkExtent2D &extent, const vDeviceImpl_ * dev, const vSurfaceImpl_ * surf);
			vSwapchainImpl_(const vSwapchainReqs &req, const VkExtent2D &extent, const vDeviceImpl_ * dev, const vSwapchainImpl_ * old);
			vSwapchainImpl_(const VkExtent2D &extent, const vSwapchainImpl_ * old);
			~vSwapchainImpl_();

			const util::nameid &getUID() const { return req_.uid; }

			vImageFormat getFormat() const { return req_.imageFormat; }
			VkExtent2D getSize() const { return extent_; }
			size_t getNumImages() const { return images_.size(); }
			VkImageView getImageView(size_t ind) const { return views_.at(ind); }
			VkImage getImage(size_t ind) const { return images_.at(ind); }

			uint32_t acquireNextImage(VkSemaphore s, VkFence f, vSwapchainResult * info = nullptr);
			void queuePresent(VkQueue q, uint32_t imgInd, vSwapchainResult * info = nullptr);
			void queuePresent(VkQueue q, uint32_t imgInd, VkSemaphore signalSemaphore, vSwapchainResult * info = nullptr);
			void queuePresent(VkQueue q, uint32_t imgInd, std::vector<VkSemaphore> &signalSemaphore, vSwapchainResult * info = nullptr);
			
		};

		class vSurface;
		class vSwapchain : public zenith::util::pimpl<vSwapchainImpl_>
		{
			using Impl = zenith::util::pimpl<vSwapchainImpl_>;
			using RawImpl = vSwapchainImpl_;			
		public:
			vSwapchain() {}
			vSwapchain(const vDeviceImpl_ *device, const vSurface &surface, const vSwapchainConfig &conf, const VkExtent2D &size);
			void create(const vDeviceImpl_ *device, const vSurface &surface, const vSwapchainConfig &conf, const VkExtent2D &size);
			void recreate(const VkExtent2D &size);
			inline void destroy() { Impl::destroy(); }

			const util::nameid &getImplUID() const { return Impl::get()->getUID(); }

			vImageFormat getFormat() const { return Impl::get()->getFormat(); }
			VkExtent2D getSize() const { return Impl::get()->getSize(); }
			size_t getNumImages() const { return Impl::get()->getNumImages(); }
			VkImageView getImageView(size_t ind) const { return Impl::get()->getImageView(ind); }
			VkImage getImage(size_t ind) const { return Impl::get()->getImage(ind); }

			uint32_t acquireNextImage(VkSemaphore s, VkFence f, vSwapchainResult * info = nullptr) { return Impl::get()->acquireNextImage(s, f, info); }
			void queuePresent(VkQueue q, uint32_t imgInd, vSwapchainResult * info = nullptr) { Impl::get()->queuePresent(q, imgInd, info); }
			void queuePresent(VkQueue q, uint32_t imgInd, VkSemaphore signalSemaphore, vSwapchainResult * info = nullptr) { Impl::get()->queuePresent(q, imgInd, signalSemaphore, info); }
			void queuePresent(VkQueue q, uint32_t imgInd, std::vector<VkSemaphore> &signalSemaphore, vSwapchainResult * info = nullptr) { Impl::get()->queuePresent(q, imgInd, signalSemaphore, info); }

		};

	}
}