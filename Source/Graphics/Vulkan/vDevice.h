#pragma once
#include "vInstance.h"
#include "vSwapchain.h"
#include "vMemoryManager.h"

namespace zenith
{
	namespace vulkan
	{
		class vDeviceException : public VulkanException
		{
		public:
			vDeviceException() : VulkanException() {}
			vDeviceException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vDeviceException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vDeviceException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vDeviceException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vDeviceException() {}
		};

		class vSwapchainImpl_;
		class vDeviceImpl_
		{
			friend class vSwapchainImpl_;
			struct vQueueData_
			{
				VkQueue queue;
				uint32_t qFamily;
				bool supportGraphics;
				bool supportCompute;
				bool supportTransfer;
				bool supportSparse;
				bool supportPresent;
			};

			VkPhysicalDevice physDevice_;
			VkPhysicalDeviceFeatures physDeviceFeatures_;
			VkDevice device_;
			vDeviceReqs req_;

			vMemoryManagerImpl_ * memManImpl_ = nullptr;

			util::nameid_map<vSwapchain> swapchains_;

			std::vector<const char *> rawExts_;
			std::vector<const char *> rawLayers_;

			zenith::util::nameid_map<vQueueData_> queues_;

			void fillExtsAndLayers_();
			void fillQueueDistribution_(const vDeviceQueueReqs &req, const std::vector<vQueueFamilyProperties> &props, std::vector < std::vector<zenith::util::nameid> > &used) const;
			void fillFeatureInfo_(VkPhysicalDeviceFeatures &df);
		public:
			vDeviceImpl_(const vInstanceImpl_ * inst, const vDeviceReqs &req);
			~vDeviceImpl_();
			inline VkPhysicalDevice getPhysicalDevice() const { return physDevice_; }
			inline VkDevice getDevice() const { return device_; }
			inline uint32_t getQueueFamilyIndex(const util::nameid &quid) const { return queues_.at(quid).qFamily; }
			inline VkQueue getQueue(const util::nameid &quid) const { return queues_.at(quid).queue; }
			inline const util::nameid &getUID() const { return req_.uid; }

			inline void attachMemoryManager(vMemoryManagerImpl_ * impl)
			{
				if (memManImpl_)
					throw vDeviceException("vDeviceImpl_::attachMemoryManager: memory manager already attached.");
				memManImpl_ = impl;
			}
			inline void detachMemoryManager(vMemoryManagerImpl_ * impl)
			{
				if (!memManImpl_)
					throw vDeviceException("vDeviceImpl_::detachMemoryManager: memory manager is not attached.");
				memManImpl_ = nullptr;
			}

			inline vMemoryBlock allocate(size_t size, size_t alignment, uint32_t memoryTypeBits, vMemoryUsage usage, vMemoryLayout layout, bool dedicatedMemory = false)
			{
				if(memManImpl_)
					return memManImpl_->allocate(size, alignment, memoryTypeBits, usage, layout, dedicatedMemory);
				else throw vDeviceException("vDeviceImpl_::allocate: memory manager is not attached.");
			}
			inline void deallocate(const vMemoryBlock &mb)
			{
				if (memManImpl_)
					memManImpl_->deallocate(mb);
				else throw vDeviceException("vDeviceImpl_::deallocate: memory manager is not attached.");
			}

			void addSwapchain(const util::nameid &name, const vSwapchainConfig &conf, const vSurface &surf, const VkExtent2D &size);
			void removeSwapchain(const util::nameid &name);
			void recreateSwapchain(const util::nameid &name, const VkExtent2D &size);
			inline vSwapchain &getSwapchain(const util::nameid &name) { return swapchains_.at(name); }
			inline const vSwapchain &getSwapchain(const util::nameid &name) const { return swapchains_.at(name); }
			inline const util::nameid_seq &getSwapchainUIDs() const { return swapchains_.names(); }
			inline void wait() const
			{
				vkDeviceWaitIdle(device_);
			}
		};


		class vDevice : public zenith::util::pimpl<vDeviceImpl_>
		{			
			friend class vMemoryManager;
			using Impl = zenith::util::pimpl<vDeviceImpl_>;
			using RawImpl = vDeviceImpl_;

			vMemoryManager memManager_;

			RawImpl * raw() { return Impl::get(); }
		public:
			inline vDevice() {};
			inline vDevice(const vInstance &inst, const vDeviceConfig &conf) { create(inst, conf); }
			/*
			inline vDevice(vDevice &&d) : Impl(std::forward<Impl>(d)), memManager_(std::move(d.memManager_)) {}
			inline vDevice &operator =(vDevice &&d)
			{
				Impl::operator=(std::forward<Impl>(d));
				memManager_ = std::move(d.memManager_);
				return *this;
			}

			vDevice(const vDevice &) = delete;
			vDevice &operator =(const vDevice &) = delete;
			*/
			void create(const vInstance &inst, const vDeviceConfig &conf);
			void destroy() { Impl::destroy(); }
			inline VkPhysicalDevice getPhysicalDevice() const { return Impl::get()->getPhysicalDevice(); }
			inline VkDevice getDevice() const { return Impl::get()->getDevice(); }
			const RawImpl * rawImpl() const { return Impl::get(); }

			const util::nameid &getImplUID() const { return Impl::get()->getUID(); }

			inline void addSwapchain(const util::nameid &name, const vSwapchainConfig &conf, const vSurface &surf, const VkExtent2D &size) { Impl::get()->addSwapchain(name, conf, surf, size); }
			inline void removeSwapchain(const util::nameid &name) { Impl::get()->removeSwapchain(name); }
			inline vSwapchain &getSwapchain(const util::nameid &name) { return Impl::get()->getSwapchain(name); }
			const inline vSwapchain &getSwapchain(const util::nameid &name) const { return Impl::get()->getSwapchain(name); }

			inline const util::nameid_seq &getSwapchainUIDs() const { return Impl::get()->getSwapchainUIDs(); }

			inline uint32_t getQueueFamilyIndex(const util::nameid &quid) const { return Impl::get()->getQueueFamilyIndex(quid); }
			inline VkQueue getQueue(const util::nameid &quid) const { return Impl::get()->getQueue(quid); }

			inline void startMemoryManager(const vMemoryConfig &mc)
			{
				if(!valid())
					throw vDeviceException("vDevice::initMemoryManager: device is not created.");
				if (memManager_.valid())
					throw vDeviceException("vDevice::initMemoryManager: memory manager already started. Need to explicitly stop memory manager before restarting.");
				memManager_.create(*this, mc);
				Impl::get()->attachMemoryManager(memManager_.raw());
			}
			inline bool startedMemoryManager() const { return memManager_.check(); }
			inline vMemoryBlock allocate(size_t size, size_t alignment, uint32_t memoryTypeBits, vMemoryUsage usage, vMemoryLayout layout, bool dedicatedMemory = false) { return Impl::get()->allocate(size, alignment, memoryTypeBits, usage, layout, dedicatedMemory); }
			inline void deallocate(const vMemoryBlock &mb) { Impl::get()->deallocate(mb); }

			inline void wait() const { Impl::get()->wait(); }
		};
	}
}