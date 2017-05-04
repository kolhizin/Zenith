#pragma once


#include "vulkan_config.h"

#include <Utils\object_map.h>
#include <Utils\log_config.h>
#include <vector>
#include "vUtil.h"
#include <Utils\str_cast.h>
#include <Utils\pimpl.h>
#include "vSurface.h"
#include <Utils\window.h>

namespace zenith
{
	namespace vulkan
	{


		

		class vInstanceException : public VulkanException
		{
		public:
			vInstanceException() : VulkanException() {}
			vInstanceException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vInstanceException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vInstanceException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vInstanceException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vInstanceException() {}
		};

		struct vInstanceCaps
		{
			std::vector<VkLayerProperties> layers;
			std::vector<std::vector<VkExtensionProperties>> layerExtensions;
			std::vector<VkExtensionProperties> instanceExtensions;
		};

		class vDeviceImpl_;

		class vInstanceImpl_
		{
			friend class vDeviceImpl_;
			struct vInstanceConfigInt_
			{
				std::vector<std::string> layerExts_;
				std::vector<const char *> enabledLayers_;
				std::vector<const char *> enabledExtensions_;
			};
			vInstanceCaps caps_;

			util::nameid_map<vSurface> surfaces_;

			vInstanceConfig config_;
			vInstanceConfigInt_ configInt_;

			std::vector<VkPhysicalDevice> devices_;

			PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
			PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;

			VkInstance instance_;


			void gatherLayerCaps_();
			void updateInternalConfig_(const vInstanceConfig &conf);
			void fillApplicationCreateInfo(VkApplicationInfo &app);
			void fillInstanceCreateInfo(VkInstanceCreateInfo &inst, VkApplicationInfo &app);

			void initDebug_();

			void gatherDevices_();

			bool checkPhysicalDevice_(const vDeviceReqs &rq, const VkPhysicalDevice &dev) const;
			VkPhysicalDevice selectPhysicalDevice_(const vDeviceReqs &rq) const;

			std::vector<vQueueFamilyProperties> getQFProperties_(const VkPhysicalDevice &dev) const;


			VkDebugReportCallbackEXT reportCallback_;

			static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback_(
				VkDebugReportFlagsEXT flags,
				VkDebugReportObjectTypeEXT objType,
				uint64_t obj,
				size_t location,
				int32_t code,
				const char* layerPrefix,
				const char* msg,
				void* userData);
		public:
			vInstanceImpl_(const vInstanceConfig &conf);
			~vInstanceImpl_();
			void addSurface(const util::nameid &name, const util::Window &wnd);
			void removeSurface(const util::nameid &name);
			vSurface &getSurface(const util::nameid &name);
			const vSurface &getSurface(const util::nameid &name) const;
			inline bool existSurface(const util::nameid &name) const { return surfaces_.exist(name); }
			//inline const vPhysicalDeviceCollection &getDevicesInfo() const { return deviceInfo_; };
		};

		class vDevice;
		class vInstance : public zenith::util::pimpl<vInstanceImpl_>
		{
			friend class vDevice;
			using Impl = zenith::util::pimpl<vInstanceImpl_>;
			using RawImpl = vInstanceImpl_;
			const RawImpl * raw() const { return Impl::get(); }
			RawImpl * raw(){ return Impl::get(); }

		public:
			vInstance() {};
			vInstance(const vInstanceConfig &conf) { Impl::create(conf); }
			void create(const vInstanceConfig &conf) { Impl::create(conf); }
			void destroy() { Impl::destroy(); }
			inline void addSurface(const util::nameid &name, const util::Window &wnd) { Impl::get()->addSurface(name, wnd); }
			inline void removeSurface(const util::nameid &name) { Impl::get()->removeSurface(name); }
			inline vSurface &getSurface(const util::nameid &name) { return Impl::get()->getSurface(name); }
			const inline vSurface &getSurface(const util::nameid &name) const { return Impl::get()->getSurface(name); }
			inline bool existSurface(const util::nameid &name) const { return Impl::get()->existSurface(name); }
		};
	}
}
