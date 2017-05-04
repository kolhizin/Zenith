#include "vSurface.h"


VkResult zenith::vulkan::vSurfaceImpl_::createWin32Surface_(HINSTANCE hinst, const VkAllocationCallbacks * pA)
{
	VkWin32SurfaceCreateInfoKHR ci = {};
	ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	ci.pNext = nullptr;
	ci.flags = 0;

	ci.hwnd = wnd_;
	ci.hinstance = hinst;

	return _vkCreateWin32SurfaceKHR(inst_, &ci, pA, &surface_);
}
void zenith::vulkan::vSurfaceImpl_::detroySurface_(const VkAllocationCallbacks * pA)
{
	_vkDestroySurfaceKHR(inst_, surface_, pA);
}


void zenith::vulkan::vSurfaceImpl_::loadFunctions_()
{
	_vkCreateWin32SurfaceKHR = nullptr;
	_vkDestroySurfaceKHR = nullptr;

	ZLOG_REGULAR("vSurfaceImpl_::loadFunctions_: Loading functions.");

	_vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(inst_, "vkCreateWin32SurfaceKHR"));
	_vkDestroySurfaceKHR = reinterpret_cast<PFN_vkDestroySurfaceKHR>(vkGetInstanceProcAddr(inst_, "vkDestroySurfaceKHR"));
	_vkGetPhysicalDeviceSurfaceSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(vkGetInstanceProcAddr(inst_, "vkGetPhysicalDeviceSurfaceSupportKHR"));
	_vkGetPhysicalDeviceSurfaceFormatsKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(vkGetInstanceProcAddr(inst_, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
	_vkGetPhysicalDeviceSurfaceCapabilitiesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(vkGetInstanceProcAddr(inst_, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
	_vkGetPhysicalDeviceSurfacePresentModesKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(vkGetInstanceProcAddr(inst_, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

	if (!_vkCreateWin32SurfaceKHR)
		throw zenith::vulkan::vSurfaceException("vSurfaceImpl_::loadFunctions_: Could not load vkCreateWin32SurfaceKHR function!");


	if (!_vkGetPhysicalDeviceSurfaceSupportKHR || !_vkGetPhysicalDeviceSurfaceFormatsKHR || !_vkDestroySurfaceKHR || !_vkGetPhysicalDeviceSurfacePresentModesKHR || !_vkGetPhysicalDeviceSurfaceCapabilitiesKHR)
		throw zenith::vulkan::vSurfaceException("vSurfaceImpl_::loadFunctions_: Could not load vkDestroySurfaceKHR or vkGet* functions!");

	ZLOG_REGULAR("vSurfaceImpl_::loadFunctions_: Functions loaded.");
}

zenith::vulkan::vSurfaceImpl_::vSurfaceImpl_(VkInstance inst, HWND wnd) : wnd_(wnd), inst_(inst)
{	
	ZLOG_REGULAR("vSurfaceImpl_::vSurfaceImpl_: Loading extension-functions.");
	loadFunctions_();
	ZLOG_REGULAR("vSurfaceImpl_::vSurfaceImpl_: Creating vulkan-surface.");
	VkResult res = createWin32Surface_(GetModuleHandle(nullptr), nullptr);
	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::vSurfaceImpl_: failed to create Win32Surface!");
	ZLOG_REGULAR("vSurfaceImpl_::vSurfaceImpl_: Vulkan-surface created.");
}

zenith::vulkan::vSurfaceImpl_::~vSurfaceImpl_()
{
	ZLOG_REGULAR("vSurfaceImpl_::~vSurfaceImpl_: Destroying surface.");
	detroySurface_(nullptr);
	ZLOG_REGULAR("vSurfaceImpl_::~vSurfaceImpl_: Surface destroyed.");
}

bool zenith::vulkan::vSurfaceImpl_::getPhysicalDeviceSurfaceSupport(VkPhysicalDevice dev, uint32_t qFamilyIndex) const
{
	VkBool32 fSup = 0;
	VkResult res = _vkGetPhysicalDeviceSurfaceSupportKHR(dev, qFamilyIndex, surface_, &fSup);
	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfaceSupport: failed to check surface support by device!");

	return (fSup == VK_TRUE);
}

VkSurfaceCapabilitiesKHR zenith::vulkan::vSurfaceImpl_::getPhysicalDeviceSurfaceCapabilities(VkPhysicalDevice dev) const
{
	VkSurfaceCapabilitiesKHR caps;
	VkResult res = _vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface_, &caps);
	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfaceCapabilities: failed to get device-surface capabilities!");
	return caps;
}

std::vector<VkSurfaceFormatKHR> zenith::vulkan::vSurfaceImpl_::getPhysicalDeviceSurfaceFormats(VkPhysicalDevice dev) const
{
	uint32_t cnt = 0;
	VkResult res = _vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface_, &cnt, nullptr);

	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfaceFormats: failed to get number of formats!");

	std::vector<VkSurfaceFormatKHR> fm(cnt);
	if (cnt == 0)
		return fm;
	
	res = _vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface_, &cnt, &fm[0]);

	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfaceFormats: failed to enumerate formats!");

	return fm;
}

std::vector<VkPresentModeKHR> zenith::vulkan::vSurfaceImpl_::getPhysicalDeviceSurfacePresentModes(VkPhysicalDevice dev) const
{
	uint32_t cnt = 0;
	VkResult res = _vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface_, &cnt, nullptr);

	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfacePresentModes: failed to get number of modes!");

	std::vector<VkPresentModeKHR> pm(cnt);
	if (cnt == 0)
		return pm;

	res = _vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface_, &cnt, &pm[0]);

	if (res != VK_SUCCESS)
		throw vSurfaceException(res, "vSurfaceImpl_::getPhysicalDeviceSurfacePresentModes: failed to enumerate modes!");

	return pm;
}


