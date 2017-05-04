#pragma once
#include "vulkan_config.h"
#include "vDevice.h"
#include "vInstance.h"
#include "vMemoryManager.h"
#include <Utils\wndUtil.h>

namespace zenith
{
	namespace vulkan
	{
		class vSystemException : public VulkanException
		{
		public:
			vSystemException() : VulkanException() {}
			vSystemException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vSystemException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vSystemException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vSystemException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vSystemException() {}
		};

		typedef util::WndSize(*PFN_vSystem_getWndSize)(const util::Window * pWnd);

		struct vSystemWindowInfo
		{
			util::nameid uid;
			void * pUserData;
			util::Window * pWnd;
			PFN_vSystem_getWndSize pfnGetWndSize;
		};

		class vSystem
		{
			vInstance instance_;
			util::nameid_map<vDevice> devices_;
			util::nameid_map<vSystemWindowInfo> windows_;
			util::nameid_map<util::nameid> surf2window_;

		public:
			vSystem(const vInstanceConfig &conf);
			vSystem(const vSystemConfig &conf, const std::vector<vSystemWindowInfo> &windows);
			vSystem(const vSystemConfig &conf, const vSystemWindowInfo &window);
			~vSystem();
			
			void startMemoryManager(const vMemoryConfig &conf);

			void addDevice(const vDeviceConfig &conf);

			void addSurface(const vSurfaceConfig &conf);

			void addSwapchain(const vSwapchainConfig &conf);

			void registerWindow(const vSystemWindowInfo &info);

			inline vInstance &getInstance() { return instance_; }
			inline const vInstance &getInstance() const { return instance_; }

			inline vDevice &getDevice(const util::nameid &name) { return devices_.at(name); }
			inline const vDevice &getDevice(const util::nameid &name) const { return devices_.at(name); }

			inline const util::nameid_seq &getDeviceUIDs() const { return devices_.names(); }
		};
	}
}