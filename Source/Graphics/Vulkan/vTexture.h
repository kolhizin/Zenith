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
		class vTextureException : public VulkanException
		{
		public:
			vTextureException() : VulkanException() {}
			vTextureException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vTextureException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vTextureException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vTextureException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vTextureException() {}
		};

		class vTextureAutoImpl_;
		class vTextureSharedImpl_;
		class vTextureRawImpl_;


		class vTextureDimensions
		{
			uint32_t width_, height_, depth_, arraySize_;
			uint32_t mipLevels_;
		public:
			inline vTextureDimensions() : width_(1), height_(1), depth_(1), arraySize_(1), mipLevels_(1) {}
			inline vTextureDimensions(uint32_t w, uint32_t h = 1, uint32_t d = 1, uint32_t arrSize = 1, uint32_t mipLevels = 1) : width_(w), height_(h), depth_(d), arraySize_(arrSize), mipLevels_(mipLevels)
			{
				if (mipLevels == 0 || arrSize == 0)
					throw vTextureException("vTextureDimensions::vTextureDimensions: incorrect mip-count or array-count (zero).");

				if (d == 0 || h == 0 || w == 0)
					throw vTextureException("vTextureDimensions::vTextureDimensions: incorrect size (zero).");
			}
			inline uint32_t width() const { return width_; }
			inline uint32_t height() const { return height_; }
			inline uint32_t depth() const { return depth_; }
			inline uint32_t arraySize() const { return arraySize_; }
			inline uint32_t mipLevels() const { return mipLevels_; }
			inline uint64_t size() const { return uint64_t(width_) * uint64_t(height_) * uint64_t(depth_); }
			inline uint32_t dim() const { return (depth_ > 1 ? 3 : (height_ > 1 ? 2 : 1)); }
			inline VkImageType vkImageType() const { return static_cast<VkImageType>((dim() - 1) + VK_IMAGE_TYPE_1D); }

			inline static vTextureDimensions SingleTextureWithoutMip(uint32_t w, uint32_t h = 1, uint32_t d = 1)
			{
				return vTextureDimensions(w, h, d, 1, 1);
			}
		};
		class vTextureSubresource
		{
			uint32_t arrOffset_, arrSize_;
			uint32_t mipOffset_, mipSize_;
		public:
			inline vTextureSubresource() : arrOffset_(0), arrSize_(1), mipOffset_(0), mipSize_(1){}
			inline vTextureSubresource(uint32_t mipBase, uint32_t mipCount, uint32_t arrOffset, uint32_t arrCount) : arrOffset_(arrOffset), arrSize_(arrCount), mipOffset_(mipBase), mipSize_(mipCount)
			{
				if (mipCount == 0 || arrCount == 0)
					throw vTextureException("vTextureSubresource::vTextureSubresource: incorrect mip-count or array-count.");
			}

			inline uint32_t arrayOffset() const { return arrOffset_; }
			inline uint32_t arraySize() const { return arrSize_; }
			inline uint32_t mipBase() const { return mipOffset_; }
			inline uint32_t mipCount() const { return mipSize_; }

			inline uint32_t arrayLast() const { return arrOffset_ + arrSize_ - 1; }
			inline uint32_t mipLast() const { return mipOffset_ + mipSize_ - 1; }

			inline bool isRange() const { return (mipSize_ > 1 || arrSize_ > 1); }
			inline bool isLayers() const { return (mipSize_ == 1); }
			inline bool isSingle() const { return (mipSize_ == 1 && arrSize_ == 1); }

			inline bool validFor(const vTextureDimensions &td) const
			{
				if (arrayLast() >= td.arraySize())
					return false;
				if (mipLast() >= td.mipLevels())
					return false;
				return true;
			}

			inline VkImageSubresource vkImageSubresource(VkImageAspectFlags flgs) const
			{
				if(!isSingle())
					throw vTextureException("vTextureSubresource::vkImageSubresource: subresource is range - not single resource.");
				VkImageSubresource res;
				res.aspectMask = flgs;
				res.arrayLayer = arrOffset_;
				res.mipLevel = mipOffset_;
				return res;
			}
			inline VkImageSubresourceLayers vkImageSubresourceLayers(VkImageAspectFlags flgs) const
			{
				if(!isLayers())
					throw vTextureException("vTextureSubresource::vkImageSubresourceLayers: subresource is range - not layered resource.");
				VkImageSubresourceLayers res;
				res.aspectMask = flgs;
				res.baseArrayLayer = arrOffset_;
				res.layerCount = arrSize_;

				res.mipLevel = mipOffset_;

				return res;
			}
			inline VkImageSubresourceRange vkImageSubresourceRange(VkImageAspectFlags flgs) const
			{
				VkImageSubresourceRange res;
				res.aspectMask = flgs;
				res.baseArrayLayer = arrOffset_;
				res.layerCount = arrSize_;

				res.baseMipLevel = mipOffset_;
				res.levelCount = mipSize_;
				return res;
			}

			inline static vTextureSubresource SingleMipSingleElement(uint32_t mipLevel = 0, uint32_t arrayIndex = 0)
			{
				return vTextureSubresource(mipLevel, 1, arrayIndex, 1);
			}
			inline static vTextureSubresource Full(const vTextureDimensions &d)
			{
				return vTextureSubresource(0, d.mipLevels(), 0, d.arraySize());
			}
		};

		class vTextureViewRawImpl_
		{
			friend class vTextureRawImpl_;

			VkImageView view_;
			vTextureRawImpl_ * texture_;
			vTextureSubresource subres_;

			vTextureViewRawImpl_(const vTextureViewRawImpl_ &) = delete;
			vTextureViewRawImpl_(vTextureViewRawImpl_ &&) = delete;
			vTextureViewRawImpl_ &operator =(const vTextureViewRawImpl_ &) = delete;
			vTextureViewRawImpl_ &operator =(vTextureViewRawImpl_ &&) = delete;
			
			vTextureViewRawImpl_(vTextureRawImpl_ * texture, VkFormat format, VkImageViewType viewType, VkImageAspectFlags imageAspect, vTextureSubresource subRes);
		public:
			~vTextureViewRawImpl_();
			VkImageView handle() const { return view_; }
		};

		class vTextureRawImpl_
		{
			friend class vTextureAutoImpl_;
			friend class vTextureSharedImpl_;
			friend class vTextureViewRawImpl_;

			vDeviceImpl_ * dev_ = nullptr;
			vObjectSharingInfo sharingInfo_;
			vTextureDimensions size_;
			VkImage image_;
			VkFormat format_;
			bool memBound_ = false;
			bool mainViewCreated_ = false;
			bool optimalTiling_ = true;

			uint8_t mainTextureViewBuffer_[sizeof(vTextureViewRawImpl_)];
			VkImageViewType mainTextureViewType_;
			VkImageAspectFlags mainTextureViewAspect_;

			vTextureRawImpl_(const vTextureRawImpl_ &) = delete;
			vTextureRawImpl_(vTextureRawImpl_ &&) = delete;
			vTextureRawImpl_ &operator =(const vTextureRawImpl_ &) = delete;
			vTextureRawImpl_ &operator =(vTextureRawImpl_ &&) = delete;

			inline vTextureViewRawImpl_ * getMainView_() { return reinterpret_cast<vTextureViewRawImpl_ *>(mainTextureViewBuffer_); }
			inline const vTextureViewRawImpl_ * getMainView_() const { return reinterpret_cast<const vTextureViewRawImpl_ *>(mainTextureViewBuffer_); }
		public:
			vTextureRawImpl_(vDeviceImpl_ * dev, vTextureDimensions texDim, VkFormat format, VkImageUsageFlags usageFlags, vObjectSharingInfo objSharing, VkImageLayout initialLayout, bool optimalTiling = true);
			~vTextureRawImpl_();

			VkMemoryRequirements getMemoryRequirements() const;
			void createMainView();
			void bindMemory(const vMemoryBlock &blk, size_t offset = 0);
			void bindMemory(const vMemoryBlockAuto &blk, size_t offset = 0);
			inline VkImage handle() const { return image_; }
			inline const vTextureDimensions &getDimensions() const { return size_; }
			inline vTextureViewRawImpl_ &mainView()
			{
				if (!mainViewCreated_)
					throw vTextureException("vTextureRawImpl_::mainView: main view is not created.");
				return *getMainView_();
			}
			inline const vTextureViewRawImpl_ &mainView() const
			{
				if (!mainViewCreated_)
					throw vTextureException("vTextureRawImpl_::mainView: main view is not created.");
				return *getMainView_();
			}

			inline bool isMemoryBound() const { return memBound_; }
			inline bool isMainViewCreated() const { return mainViewCreated_; }
		};


		class vTextureAutoImpl_
		{
			vTextureRawImpl_ raw_;
			vMemoryBlockAuto blk_;
		public:
			inline vTextureAutoImpl_(vDeviceImpl_ * dev, vTextureDimensions texDim, VkFormat format, VkImageUsageFlags usageFlags, vMemoryUsage memUsage, vObjectSharingInfo objSharing, VkImageLayout initialLayout, bool optimalTiling = true)
				: raw_(dev, texDim, format, usageFlags, objSharing, initialLayout, optimalTiling)
			{
				VkMemoryRequirements rq = raw_.getMemoryRequirements();
				blk_ = dev->allocate(rq.size, rq.alignment, rq.memoryTypeBits, memUsage, (optimalTiling ? vMemoryLayout::OPTIMAL : vMemoryLayout::LINEAR));
				raw_.bindMemory(blk_);
				raw_.createMainView();
			}

			inline VkImage handle() const { return raw_.image_; }

			inline vMemoryBlockAuto &memoryBlock() { return blk_; }

			inline const vTextureDimensions &getDimensions() const { return raw_.getDimensions(); }
			inline vTextureViewRawImpl_ &mainView() { return raw_.mainView(); }
			inline const vTextureViewRawImpl_ &mainView() const { return raw_.mainView(); }
		};
	}
}