#pragma once
#include "vulkan_general.h"
#include <Windows.h>
#include <Utils\log_config.h>
#include <Utils\pimpl.h>
#include <vector>

namespace zenith
{
	namespace vulkan
	{
		class vSurfaceException : public VulkanException
		{
		public:
			vSurfaceException() : VulkanException() {}
			vSurfaceException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vSurfaceException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vSurfaceException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vSurfaceException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vSurfaceException() {}
		};

		class vSurfaceImpl_
		{
			VkSurfaceKHR surface_;
			HWND wnd_;
			VkInstance inst_;

			PFN_vkCreateWin32SurfaceKHR _vkCreateWin32SurfaceKHR;
			PFN_vkDestroySurfaceKHR _vkDestroySurfaceKHR;
			PFN_vkGetPhysicalDeviceSurfaceSupportKHR _vkGetPhysicalDeviceSurfaceSupportKHR;
			PFN_vkGetPhysicalDeviceSurfaceFormatsKHR _vkGetPhysicalDeviceSurfaceFormatsKHR;
			PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR _vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
			PFN_vkGetPhysicalDeviceSurfacePresentModesKHR _vkGetPhysicalDeviceSurfacePresentModesKHR;

			void loadFunctions_();

			VkResult createWin32Surface_(HINSTANCE hinst, const VkAllocationCallbacks * pA);
			void detroySurface_(const VkAllocationCallbacks * pA);

		public:
			vSurfaceImpl_(VkInstance inst, HWND wnd);
			~vSurfaceImpl_();

			bool getPhysicalDeviceSurfaceSupport(VkPhysicalDevice dev, uint32_t qFamilyIndex) const;
			std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(VkPhysicalDevice dev) const;
			std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(VkPhysicalDevice dev) const;
			VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice dev) const;

			inline VkSurfaceKHR getSurface() const { return surface_; }
		};

		class vSurface : public zenith::util::pimpl<vSurfaceImpl_>
		{
			using Impl = zenith::util::pimpl<vSurfaceImpl_>;
			using RawImpl = vSurfaceImpl_;
		public:
			vSurface() {};
			vSurface(VkInstance inst, HWND wnd) { Impl::create(inst,wnd); }
			inline void create(VkInstance inst, HWND wnd) { Impl::create(inst, wnd); }
			inline void destroy() { Impl::destroy(); }
			const RawImpl * rawImpl() const { return Impl::get(); }
			inline bool getPhysicalDeviceSurfaceSupport(VkPhysicalDevice dev, uint32_t qFamilyIndex) const
			{
				return Impl::get()->getPhysicalDeviceSurfaceSupport(dev, qFamilyIndex);
			}
			inline std::vector<VkSurfaceFormatKHR> getPhysicalDeviceSurfaceFormats(VkPhysicalDevice dev) const
			{
				return Impl::get()->getPhysicalDeviceSurfaceFormats(dev);
			}
			inline VkSurfaceCapabilitiesKHR getPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice dev) const
			{
				return Impl::get()->getPhysicalDeviceSurfaceCapabilities(dev);
			}
			inline std::vector<VkPresentModeKHR> getPhysicalDeviceSurfacePresentModes(VkPhysicalDevice dev) const
			{
				return Impl::get()->getPhysicalDeviceSurfacePresentModes(dev);
			}
		};
	}
}