
#include "vInstance.h"
#include <Utils\str_cast.h>
#include <Utils\xml_tools.h>
#include <iostream>

using namespace zenith::vulkan;

void vInstanceImpl_::gatherLayerCaps_()
{
	caps_.instanceExtensions.clear();
	caps_.layerExtensions.clear();
	caps_.layers.clear();

	uint32_t layerCount = 0;
	VkResult vkRes;
	vkRes = vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	if (vkRes != VK_SUCCESS)
		throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to get count layer properties!");

	caps_.layers.resize(layerCount);
	caps_.layerExtensions.resize(layerCount);
	
	vkRes = vkEnumerateInstanceLayerProperties(&layerCount, &caps_.layers[0]);
	if (vkRes != VK_SUCCESS)
		throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to gather layer properties!");
	
	for (unsigned i = 0; i < layerCount; i++)
	{
		uint32_t extensionCount = 0;
		vkRes = vkEnumerateInstanceExtensionProperties(caps_.layers[i].layerName, &extensionCount, nullptr);
		if (vkRes != VK_SUCCESS)
			throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to get count of layer extensions!");

		if (extensionCount > 0)
		{
			caps_.layerExtensions[i].resize(extensionCount);
			vkRes = vkEnumerateInstanceExtensionProperties(caps_.layers[i].layerName, &extensionCount, &caps_.layerExtensions[i][0]);
			if (vkRes != VK_SUCCESS)
				throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to gather layer extensions!");
		}
	}

	uint32_t extensionCount = 0;
	vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	if (vkRes != VK_SUCCESS)
		throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to get count of instance extensions!");

	caps_.instanceExtensions.resize(extensionCount);
	vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, &caps_.instanceExtensions[0]);
	if (vkRes != VK_SUCCESS)
		throw vInstanceException(vkRes, "vInstanceImpl_::gatherCaps_(): Failed to gather instance extensions!");
}

void zenith::vulkan::vInstanceImpl_::updateInternalConfig_(const vInstanceConfig & conf)
{
	configInt_.enabledExtensions_.clear();
	configInt_.enabledLayers_.clear();
	config_.enabledLayers.clear();
	config_.requiredExtensions.clear();
	configInt_.layerExts_.clear();

	for (auto &reqLayer : conf.enabledLayers)
	{
		size_t i = 0;
		for (; i < caps_.layers.size(); i++)
			if (reqLayer.name == std::string(caps_.layers[i].layerName))
				break;
		if(i >= caps_.layers.size())
			throw vInstanceException(std::string("vInstanceImpl_::updateInternalConfig_(): required instance layer (") + reqLayer.name + ") not found among instance caps!");
		
		for (auto &reqExt : reqLayer.requiredExtensions)
		{
			size_t j = 0;
			for (; j < caps_.layerExtensions[i].size(); j++)
				if (reqExt == std::string(caps_.layerExtensions[i][j].extensionName))
					break;

			if (j >= caps_.layerExtensions[i].size())
				throw vInstanceException(std::string("vInstanceImpl_::updateInternalConfig_(): required layer extension (") + reqExt + ") not found among instance caps!");

			if(std::find(configInt_.layerExts_.begin(), configInt_.layerExts_.end(), reqExt) == configInt_.layerExts_.end())
				configInt_.layerExts_.push_back(reqExt);
		}

		config_.enabledLayers.push_back(reqLayer);
	}
	for (auto &reqExt : conf.requiredExtensions)
	{
		size_t j = 0;
		for (; j < caps_.instanceExtensions.size(); j++)
			if (reqExt == std::string(caps_.instanceExtensions[j].extensionName))
				break;
		if (j >= caps_.instanceExtensions.size())
			throw vInstanceException(std::string("vInstanceImpl_::updateInternalConfig_(): required instance extension (") + reqExt + ") not found among instance caps!");

		config_.requiredExtensions.push_back(reqExt);
		auto iter = std::find(configInt_.layerExts_.begin(), configInt_.layerExts_.end(), reqExt);
		if (iter != configInt_.layerExts_.end())
			configInt_.layerExts_.erase(iter);
	}

	
	config_.apiVersion = VK_API_VERSION_1_0;
	if(conf.apiVersion != config_.apiVersion)
		throw vInstanceException("vInstanceImpl_::updateInternalConfig_(): unsupported api version!");

	config_.appName = conf.appName;
	config_.appVersion = conf.appVersion;

	config_.engineName = conf.engineName;
	config_.engineVersion = conf.engineVersion;

	config_.debug = conf.debug;

	configInt_.enabledExtensions_.resize(config_.requiredExtensions.size());
	configInt_.enabledLayers_.resize(config_.enabledLayers.size());
	for (size_t i = 0; i < configInt_.enabledExtensions_.size(); i++)
		configInt_.enabledExtensions_[i] = config_.requiredExtensions[i].c_str();
	for (size_t i = 0; i < configInt_.layerExts_.size(); i++)
		configInt_.enabledExtensions_.push_back(configInt_.layerExts_[i].c_str());
	for (size_t i = 0; i < config_.enabledLayers.size(); i++)
		configInt_.enabledLayers_[i] = config_.enabledLayers[i].name.c_str();
}

void zenith::vulkan::vInstanceImpl_::fillApplicationCreateInfo(VkApplicationInfo & app)
{
	app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;

	app.pApplicationName = config_.appName.c_str();
	app.applicationVersion = config_.appVersion;

	app.pEngineName = config_.engineName.c_str();
	app.engineVersion = config_.engineVersion;

	app.apiVersion = config_.apiVersion;

	app.pNext = nullptr;
}

void zenith::vulkan::vInstanceImpl_::fillInstanceCreateInfo(VkInstanceCreateInfo & inst, VkApplicationInfo & app)
{
	fillApplicationCreateInfo(app);

	inst.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	inst.flags = 0;
	
	inst.enabledLayerCount = static_cast<uint32_t>(configInt_.enabledLayers_.size());
	inst.ppEnabledLayerNames = (configInt_.enabledLayers_.empty() ? nullptr : &configInt_.enabledLayers_[0]);

	inst.enabledExtensionCount = static_cast<uint32_t>(configInt_.enabledExtensions_.size());
	inst.ppEnabledExtensionNames = (configInt_.enabledExtensions_.empty() ? nullptr : &configInt_.enabledExtensions_[0]);

	inst.pApplicationInfo = &app;

	inst.pNext = nullptr;
}

void zenith::vulkan::vInstanceImpl_::initDebug_()
{
	vkCreateDebugReportCallback = VK_NULL_HANDLE;
	vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkCreateDebugReportCallbackEXT");

	if(vkCreateDebugReportCallback == VK_NULL_HANDLE)
		throw vInstanceException(std::string("vInstanceImpl_::initDebug_(): failed to getProcAddr for vkCreateDebugReportCallbackEXT!"));

	vkDestroyDebugReportCallback = VK_NULL_HANDLE;
	vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugReportCallbackEXT");

	if (vkDestroyDebugReportCallback == VK_NULL_HANDLE)
		throw vInstanceException(std::string("vInstanceImpl_::initDebug_(): failed to getProcAddr for vkDestroyDebugReportCallbackEXT!"));

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT
		| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;

	createInfo.pfnCallback = debugCallback_;

	createInfo.pNext = nullptr;
	createInfo.pUserData = nullptr;	

	VkResult res;
	res = vkCreateDebugReportCallback(instance_, &createInfo, nullptr, &reportCallback_);
	if (res != VK_SUCCESS)
		throw vInstanceException(res, std::string("vInstanceImpl_::initDebug_(): failed to create a callback in vkCreateDebugReportCallback!"));
}

void zenith::vulkan::vInstanceImpl_::gatherDevices_()
{
	uint32_t deviceCount = 0;
	VkResult res;
	res = vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (res != VK_SUCCESS)
		throw vInstanceException(res, std::string("vInstanceImpl_::gatherDevices_(): failed to get number of physical devices!"));

	devices_.clear();
	devices_.resize(deviceCount);
	res = vkEnumeratePhysicalDevices(instance_, &deviceCount, &devices_[0]);
	if (res != VK_SUCCESS)
		throw vInstanceException(res, std::string("vInstanceImpl_::gatherDevices_(): failed to get all physical devices!"));
}

bool zenith::vulkan::vInstanceImpl_::checkPhysicalDevice_(const vDeviceReqs & rq, const VkPhysicalDevice &dev) const
{
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	VkPhysicalDeviceMemoryProperties deviceMemory;

	vkGetPhysicalDeviceProperties(dev, &deviceProperties);
	vkGetPhysicalDeviceFeatures(dev, &deviceFeatures);
	vkGetPhysicalDeviceMemoryProperties(dev, &deviceMemory);

	uint64_t memSize = 0;
	for (size_t i = 0; i < deviceMemory.memoryHeapCount; i++)
		memSize += deviceMemory.memoryHeaps[i].size;


	if (rq.memorySize > memSize)
		return false;
	if (rq.physicalType != static_cast<vPhysicalDeviceType>(deviceProperties.deviceType))
		return false;
	
	/*
	update definition to check for queue requirements
	*/
	return true;
}

VkPhysicalDevice zenith::vulkan::vInstanceImpl_::selectPhysicalDevice_(const vDeviceReqs & rq) const
{
	for (size_t i = 0; i < devices_.size(); i++)
		if (checkPhysicalDevice_(rq, devices_[i]))
			return devices_[i];
	throw vInstanceException("vInstanceImpl_::selectPhysicalDevice_: no physical device matches requirements!");
}

std::vector<vQueueFamilyProperties> zenith::vulkan::vInstanceImpl_::getQFProperties_(const VkPhysicalDevice &dev) const
{
	std::vector<vQueueFamilyProperties> res;

	std::vector<VkQueueFamilyProperties> tmp;
	
	uint32_t cnt = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(dev, &cnt, nullptr);

	if (cnt == 0)
		return res;

	tmp.resize(cnt);
	res.resize(cnt);

	vkGetPhysicalDeviceQueueFamilyProperties(dev, &cnt, &tmp[0]);

	for (uint32_t i = 0; i < cnt; i++)
	{
		res[i].generalProperties = tmp[i];
		for (const zenith::util::nameid &srf_uid : surfaces_.names())
		{
			const vSurface &srf = surfaces_.at(srf_uid);
			if (srf.getPhysicalDeviceSurfaceSupport(dev, i))
				res[i].supportedSurfaces.insert(srf_uid);
		}
	}

	return res;
}

VKAPI_ATTR VkBool32 VKAPI_CALL zenith::vulkan::vInstanceImpl_::debugCallback_(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objType,
		uint64_t obj, size_t location, int32_t code, const char * layerPrefix, const char * msg, void * userData)
{
	std::string str = std::string("Vulkan-Debug-Callback: ") + msg;
	if(objType == VK_DEBUG_REPORT_ERROR_BIT_EXT)
		zenith::util::zLOG::log(zenith::util::LogType::ERROR, str);
	else if (objType == VK_DEBUG_REPORT_WARNING_BIT_EXT)
		zenith::util::zLOG::log(zenith::util::LogType::WARNING, str);
	else zenith::util::zLOG::log(zenith::util::LogType::REGULAR, str);
	return VK_FALSE;
}

vInstanceImpl_::vInstanceImpl_(const vInstanceConfig &conf)
{
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Gathering instance-level caps.");
	gatherLayerCaps_();

	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Caps gathered. Checking with InstanceConfig.");
	updateInternalConfig_(conf);

	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Config checked. Creating VULKAN-Instance.");

	VkInstanceCreateInfo inst_info;
	VkApplicationInfo app_info;
	fillInstanceCreateInfo(inst_info, app_info);

	VkResult res;

	res = vkCreateInstance(&inst_info, nullptr, &instance_);
	if(res != VK_SUCCESS)
		throw vInstanceException(res, std::string("vInstanceImpl_::vInstanceImpl_(): failed to create an instance in vkCreateInstance!"));

	if (config_.debug)
	{
		zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Creating debug callbacks.");
		initDebug_();
	}

	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Instance created. Gathering physical devices.");
	gatherDevices_();	
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_: Devices gathered.");
}

zenith::vulkan::vInstanceImpl_::~vInstanceImpl_()
{

	surfaces_.clear();

	if (config_.debug && vkDestroyDebugReportCallback)
	{
		zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_~: Destroying debug callback.");

		vkDestroyDebugReportCallback(instance_, reportCallback_, nullptr);
	}
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_~: Destroying instance.");
	vkDestroyInstance(instance_, nullptr);
	zenith::util::zLOG::log(zenith::util::LogType::REGULAR, "vInstanceImpl_~: Destroyed instance.");
}

void zenith::vulkan::vInstanceImpl_::addSurface(const util::nameid & name, const util::Window & wnd)
{
	ZLOG_DEBUG("vInstanceImpl_::addSurface: Adding new surface.");
	vSurface surf(instance_, wnd.hwnd());
	surfaces_.insert(name, std::move(surf));
	ZLOG_DEBUG("vInstanceImpl_::addSurface: Surface added.");
}

void zenith::vulkan::vInstanceImpl_::removeSurface(const util::nameid & name)
{
	ZLOG_DEBUG("vInstanceImpl_::removeSurface: Removing surface.");
	surfaces_.remove(name);
	ZLOG_DEBUG("vInstanceImpl_::removeSurface: Surface removed.");
}

vSurface & zenith::vulkan::vInstanceImpl_::getSurface(const util::nameid & name)
{
	return surfaces_.at(name);
}

const vSurface & zenith::vulkan::vInstanceImpl_::getSurface(const util::nameid & name) const
{
	return surfaces_.at(name);
}
