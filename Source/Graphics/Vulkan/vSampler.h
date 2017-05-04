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
		class vSamplerException : public VulkanException
		{
		public:
			vSamplerException() : VulkanException() {}
			vSamplerException(const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(p, type)
			{
			}
			vSamplerException(const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(str, type)
			{
			}
			vSamplerException(VkResult res, const char * p, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, p, type)
			{
			}
			vSamplerException(VkResult res, const std::string &str, zenith::util::LogType type = zenith::util::LogType::ERROR) : VulkanException(res, str, type)
			{
			}
			virtual ~vSamplerException() {}
		};

		class vSamplerAddressing
		{
			vSamplerAddressingConfig conf_;
			void check_() const;
		public:
			vSamplerAddressing(bool normalized = true)
			{
				conf_.u = vSamplerAddress::CLAMP; conf_.v = vSamplerAddress::CLAMP; conf_.w = vSamplerAddress::CLAMP;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = vSamplerBorderColor::OPAQUE_BLACK;
				conf_.borderType = vSamplerBorderType::FLOAT;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, bool normalized = true)
			{
				conf_.u = u; conf_.v = vSamplerAddress::CLAMP; conf_.w = vSamplerAddress::CLAMP;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = vSamplerBorderColor::OPAQUE_BLACK;
				conf_.borderType = vSamplerBorderType::FLOAT;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, vSamplerAddress v, bool normalized = true)
			{
				conf_.u = u; conf_.v = v; conf_.w = vSamplerAddress::CLAMP;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = vSamplerBorderColor::OPAQUE_BLACK;
				conf_.borderType = vSamplerBorderType::FLOAT;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, vSamplerAddress v, vSamplerAddress w, bool normalized = true)
			{
				conf_.u = u; conf_.v = v; conf_.w = w;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = vSamplerBorderColor::OPAQUE_BLACK;
				conf_.borderType = vSamplerBorderType::FLOAT;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, vSamplerBorderColor bColor, vSamplerBorderType bType = vSamplerBorderType::FLOAT, bool normalized = true)
			{
				conf_.u = u; conf_.v = vSamplerAddress::CLAMP; conf_.w = vSamplerAddress::CLAMP;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = bColor;
				conf_.borderType = bType;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, vSamplerAddress v, vSamplerBorderColor bColor, vSamplerBorderType bType = vSamplerBorderType::FLOAT, bool normalized = true)
			{
				conf_.u = u; conf_.v = v; conf_.w = vSamplerAddress::CLAMP;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = bColor;
				conf_.borderType = bType;
				check_();
			}
			vSamplerAddressing(vSamplerAddress u, vSamplerAddress v, vSamplerAddress w, vSamplerBorderColor bColor, vSamplerBorderType bType = vSamplerBorderType::FLOAT, bool normalized = true)
			{
				conf_.u = u; conf_.v = v; conf_.w = w;
				conf_.normalizedCoord = normalized;
				conf_.borderColor = bColor;
				conf_.borderType = bType;
				check_();
			}
			vSamplerAddressing(const vSamplerAddressingConfig &conf) : conf_(conf) { check_(); }

			inline vSamplerAddress uAddr() const { return conf_.u; }
			inline vSamplerAddress vAddr() const { return conf_.v; }
			inline vSamplerAddress wAddr() const { return conf_.w; }
			inline bool isNormalized() const { return conf_.normalizedCoord; }
			inline bool useBorder() const
			{
				return (conf_.u == zenith::vulkan::vSamplerAddress::CLAMP_BORDER) || (conf_.v == zenith::vulkan::vSamplerAddress::CLAMP_BORDER) || (conf_.w == zenith::vulkan::vSamplerAddress::CLAMP_BORDER);
			}

			inline const vSamplerAddressingConfig& config() const { return conf_; }


			inline bool operator ==(const vSamplerAddressing &rhs) const
			{
				return (conf_.u == rhs.conf_.u) && (conf_.v == rhs.conf_.v) && (conf_.w == rhs.conf_.w) && (conf_.normalizedCoord == rhs.conf_.normalizedCoord);
			}
			inline bool operator !=(const vSamplerAddressing &rhs) const
			{
				return (conf_.u != rhs.conf_.u) || (conf_.v != rhs.conf_.v) || (conf_.w != rhs.conf_.w) || (conf_.normalizedCoord != rhs.conf_.normalizedCoord);
			}

			void vkSetUpCreateInfo(VkSamplerCreateInfo &ci) const;
		};

		class vSamplerFiltering
		{
			vSamplerFilteringConfig conf_;
			void check_() const;
		public:
			vSamplerFiltering(vSamplerFilter minFilter = vSamplerFilter::LINEAR, vSamplerFilter magFilter = vSamplerFilter::LINEAR, vSamplerFilter mipFilter = vSamplerFilter::LINEAR, float maxAnisotropy = 0.0f)
			{
				conf_.minFilter = minFilter;
				conf_.magFilter = magFilter;
				conf_.mipFilter = mipFilter;
				conf_.maxAnisotropy = maxAnisotropy;
				check_();
			}
			vSamplerFiltering(const vSamplerFilteringConfig &conf) : conf_(conf) { check_(); }

			inline vSamplerFilter minFilter() const { return conf_.minFilter; }
			inline vSamplerFilter magFilter() const { return conf_.magFilter; }
			inline vSamplerFilter mipFilter() const { return conf_.mipFilter; }
			inline bool isAnisotropic() const { return (conf_.maxAnisotropy > 1.0f); }
			inline float maxAnisotropy() const { return (conf_.maxAnisotropy > 1.0f ? conf_.maxAnisotropy : 1.0f); }

			inline bool operator ==(const vSamplerFiltering &rhs) const
			{
				return (conf_.minFilter == rhs.conf_.minFilter) && (conf_.magFilter == rhs.conf_.magFilter) && (conf_.mipFilter == rhs.conf_.mipFilter) && (isAnisotropic() == rhs.isAnisotropic()) && (maxAnisotropy() == rhs.maxAnisotropy());
			}
			inline bool operator !=(const vSamplerFiltering &rhs) const
			{
				return (conf_.minFilter != rhs.conf_.minFilter) || (conf_.magFilter != rhs.conf_.magFilter) || (conf_.mipFilter != rhs.conf_.mipFilter) || (isAnisotropic() != rhs.isAnisotropic()) || (maxAnisotropy() != rhs.maxAnisotropy());
			}

			void vkSetUpCreateInfo(VkSamplerCreateInfo &ci) const;
		};

		class vSamplerLOD
		{
			vSamplerLODConfig conf_;
			void check_() const;
		public:
			vSamplerLOD(float minLOD = 0.0f, float maxLOD = std::numeric_limits<float>::max(), float biasLOD = 0.0f)
			{
				conf_.biasLOD = biasLOD;
				conf_.maxLOD = maxLOD;
				conf_.minLOD = minLOD;
				check_();
			}
			vSamplerLOD(const vSamplerLODConfig &conf) : conf_(conf) { check_(); }
			float biasLOD() const { return conf_.biasLOD; }
			float maxLOD() const { return conf_.maxLOD; }
			float minLOD() const { return conf_.minLOD; }

			inline bool operator ==(const vSamplerLOD &rhs) const
			{
				return (conf_.minLOD == rhs.conf_.minLOD) && (conf_.maxLOD == rhs.conf_.maxLOD) && (conf_.biasLOD == rhs.conf_.biasLOD);
			}
			inline bool operator !=(const vSamplerLOD &rhs) const
			{
				return (conf_.minLOD != rhs.conf_.minLOD) || (conf_.maxLOD != rhs.conf_.maxLOD) || (conf_.biasLOD != rhs.conf_.biasLOD);
			}

			void vkSetUpCreateInfo(VkSamplerCreateInfo &ci) const;
		};


		class vSamplerImpl_
		{
			vDeviceImpl_ * dev_ = nullptr;
			VkSampler sampler_;

			vSamplerImpl_(const vSamplerImpl_ &) = delete;
			vSamplerImpl_(vSamplerImpl_ &&) = delete;
			vSamplerImpl_ &operator =(const vSamplerImpl_ &) = delete;
			vSamplerImpl_ &operator =(vSamplerImpl_ &&) = delete;
		public:
			vSamplerImpl_(vDeviceImpl_ * dev, vSamplerFiltering filtering, vSamplerAddressing addressing, vSamplerLOD lod);
			~vSamplerImpl_();

			inline VkSampler handle() const { return sampler_; }
		};
	}
}
