#pragma once


#include "vulkan_config.h"
#include <Utils\pimpl.h>
#include "vUtil.h"
#include "vMemoryBase.h"
#include "vDevice.h"

namespace zenith
{
	namespace vulkan
	{
		class vBufferException : public VulkanException
		{
		public:
			vBufferException() : VulkanException() {}
			vBufferException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vBufferException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vBufferException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vBufferException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vBufferException() {}
		};

		class vBufferAutoImpl_;
		class vBufferSharedImpl_;

		class vBufferRawImpl_
		{
			friend class vBufferAutoImpl_;
			friend class vBufferSharedImpl_;

			vDeviceImpl_ * dev_ = nullptr;
			vObjectSharingInfo sharingInfo_;
			VkBuffer buffer_;
			bool memBound_ = false;

			vBufferRawImpl_(const vBufferRawImpl_ &) = delete;
			vBufferRawImpl_(vBufferRawImpl_ &&) = delete;
			vBufferRawImpl_ &operator =(const vBufferRawImpl_ &) = delete;
			vBufferRawImpl_ &operator =(vBufferRawImpl_ &&) = delete;
		public:
			vBufferRawImpl_(vDeviceImpl_ * dev, size_t size, VkBufferUsageFlags usageFlags, vObjectSharingInfo objSharing);
			~vBufferRawImpl_();

			VkMemoryRequirements getMemoryRequirements() const;
			void bindMemory(const vMemoryBlock &blk, size_t offset = 0);
			void bindMemory(const vMemoryBlockAuto &blk, size_t offset = 0);
			inline VkBuffer handle() const { return buffer_; }
		};

		class vBufferAutoImpl_
		{
			vBufferRawImpl_ raw_;
			vMemoryBlockAuto blk_;
		public:
			inline vBufferAutoImpl_(vDeviceImpl_ * dev, size_t size, VkBufferUsageFlags usageFlags, vMemoryUsage memUsage, vObjectSharingInfo objSharing)
				: raw_(dev, size, usageFlags, objSharing)
			{
				VkMemoryRequirements rq = raw_.getMemoryRequirements();
				blk_ = dev->allocate(rq.size, rq.alignment, rq.memoryTypeBits, memUsage, vMemoryLayout::LINEAR);
				raw_.bindMemory(blk_);
			}

			inline VkBuffer handle() const { return raw_.buffer_; }
			
			inline vMemoryBlockAuto &memoryBlock() { return blk_; }
		};
	}
}