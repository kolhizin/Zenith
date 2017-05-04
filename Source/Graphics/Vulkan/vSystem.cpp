#include "vSystem.h"
#include <Utils\wndUtil.h>

zenith::vulkan::vSystem::vSystem(const vInstanceConfig & conf) : instance_(conf)
{
	ZLOG_REGULAR("vSystem::vSystem: finished creation.");
}

zenith::vulkan::vSystem::vSystem(const vSystemConfig & conf, const std::vector<vSystemWindowInfo>& windows) : instance_(conf.instance)
{
	ZLOG_REGULAR("vSystem::vSystem: registering windows.");
	for (const vSystemWindowInfo &info : windows)
		registerWindow(info);

	ZLOG_REGULAR("vSystem::vSystem: adding surfaces.");
	for (const auto &conf : conf.surface)
		addSurface(conf);

	ZLOG_REGULAR("vSystem::vSystem: adding devices.");
	for (const auto &conf : conf.device)
		addDevice(conf);

	ZLOG_REGULAR("vSystem::vSystem: starting memory managers.");
	for (const auto &conf : conf.memory)
		startMemoryManager(conf);
	
	ZLOG_REGULAR("vSystem::vSystem: adding swapchains.");
	for (const auto &conf : conf.swapchain)
		addSwapchain(conf);

	ZLOG_REGULAR("vSystem::vSystem: finished creation.");
}

zenith::vulkan::vSystem::vSystem(const vSystemConfig & conf, const vSystemWindowInfo & window) : instance_(conf.instance)
{
	ZLOG_REGULAR("vSystem::vSystem: registering window.");
	registerWindow(window);

	ZLOG_REGULAR("vSystem::vSystem: adding surfaces.");
	for (const auto &conf : conf.surface)
		addSurface(conf);

	ZLOG_REGULAR("vSystem::vSystem: adding devices.");
	for (const auto &conf : conf.device)
		addDevice(conf);

	ZLOG_REGULAR("vSystem::vSystem: starting memory managers.");
	for (const auto &conf : conf.memory)
		startMemoryManager(conf);

	ZLOG_REGULAR("vSystem::vSystem: adding swapchains.");
	for (const auto &conf : conf.swapchain)
		addSwapchain(conf);
	ZLOG_REGULAR("vSystem::vSystem: finished creation.");
}

zenith::vulkan::vSystem::~vSystem()
{
	ZLOG_REGULAR("vSystem::~vSystem: destroying devices.");
	devices_.clear();
	ZLOG_REGULAR("vSystem::~vSystem: devices destroyed.");
}

void zenith::vulkan::vSystem::startMemoryManager(const vMemoryConfig & conf)
{
	devices_.at(conf.deviceUID).startMemoryManager(conf);
}

void zenith::vulkan::vSystem::addDevice(const vDeviceConfig & conf)
{
	vDevice dev0;
	devices_.insert(conf.uid, std::move(dev0));
	devices_.at(conf.uid).create(instance_, conf);

	ZLOG_REGULAR(std::string("vSystem::addDevice: created device (uid=<") + conf.uid.c_str() + ">, impl-uid=<" + devices_.at(conf.uid).getImplUID().c_str() + ">).");
}

void zenith::vulkan::vSystem::addSurface(const vSurfaceConfig & conf)
{
	auto &w = windows_.at(conf.windowUID);
	instance_.addSurface(conf.uid, *w.pWnd);
	surf2window_.insert(conf.uid, conf.windowUID);
}

void zenith::vulkan::vSystem::addSwapchain(const vSwapchainConfig & conf)
{
	auto &d = devices_.at(conf.deviceUID);
	auto &s = instance_.getSurface(conf.surfaceUID);
	auto &w = windows_.at(surf2window_.at(conf.surfaceUID));

	util::WndSize sz = w.pfnGetWndSize(w.pWnd);
	VkExtent2D ext; ext.width = sz.width; ext.height = sz.height;

	d.addSwapchain(conf.uid, conf, s, ext);


	ZLOG_REGULAR(std::string("vSystem::addSwapchain: created swapchain (device-uid=<") + conf.deviceUID.c_str()
		+ ">, surface-uid=<" + conf.surfaceUID.c_str() + ">, uid=<" + conf.uid.c_str()
		+ ">, impl-uid=<" + devices_.at(conf.deviceUID).getSwapchain(conf.uid).getImplUID().c_str() + ">).");
}

void zenith::vulkan::vSystem::registerWindow(const vSystemWindowInfo & info)
{
	windows_.insert(info.uid, info);
}
