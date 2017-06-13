#pragma once
#include "ff_img.h"
#include <vulkan\vulkan.h>

namespace zenith
{
	namespace util
	{
		namespace zfile_format
		{
			inline ImageFormat Vulkan2ImageFormat(VkFormat fmt)
			{
				switch (fmt)
				{
				case VK_FORMAT_UNDEFINED: return ImageFormat::UNDEF;
				case VK_FORMAT_R8_UNORM:
				case VK_FORMAT_R8_SNORM:
				case VK_FORMAT_R8_USCALED:
				case VK_FORMAT_R8_SSCALED:
				case VK_FORMAT_R8_UINT:
				case VK_FORMAT_R8_SINT:
				case VK_FORMAT_R8_SRGB:
					return ImageFormat::R8;
				case VK_FORMAT_R8G8_UNORM:
				case VK_FORMAT_R8G8_SNORM:
				case VK_FORMAT_R8G8_USCALED:
				case VK_FORMAT_R8G8_SSCALED:
				case VK_FORMAT_R8G8_UINT:
				case VK_FORMAT_R8G8_SINT:
				case VK_FORMAT_R8G8_SRGB:
					return ImageFormat::R8G8;
				case VK_FORMAT_R8G8B8_UNORM:
				case VK_FORMAT_R8G8B8_SNORM:
				case VK_FORMAT_R8G8B8_USCALED:
				case VK_FORMAT_R8G8B8_SSCALED:
				case VK_FORMAT_R8G8B8_UINT:
				case VK_FORMAT_R8G8B8_SINT:
				case VK_FORMAT_R8G8B8_SRGB:
					return ImageFormat::R8G8B8;
				case VK_FORMAT_R8G8B8A8_UNORM:
				case VK_FORMAT_R8G8B8A8_SNORM:
				case VK_FORMAT_R8G8B8A8_USCALED:
				case VK_FORMAT_R8G8B8A8_SSCALED:
				case VK_FORMAT_R8G8B8A8_UINT:
				case VK_FORMAT_R8G8B8A8_SINT:
				case VK_FORMAT_R8G8B8A8_SRGB:
					return ImageFormat::R8G8B8A8;
				case VK_FORMAT_R16_UNORM:
				case VK_FORMAT_R16_SNORM:
				case VK_FORMAT_R16_USCALED:
				case VK_FORMAT_R16_SSCALED:
				case VK_FORMAT_R16_UINT:
				case VK_FORMAT_R16_SINT:
					return ImageFormat::R16;
				case VK_FORMAT_R16G16_UNORM:
				case VK_FORMAT_R16G16_SNORM:
				case VK_FORMAT_R16G16_USCALED:
				case VK_FORMAT_R16G16_SSCALED:
				case VK_FORMAT_R16G16_UINT:
				case VK_FORMAT_R16G16_SINT:
					return ImageFormat::R16G16;
				case VK_FORMAT_R16G16B16_UNORM:
				case VK_FORMAT_R16G16B16_SNORM:
				case VK_FORMAT_R16G16B16_USCALED:
				case VK_FORMAT_R16G16B16_SSCALED:
				case VK_FORMAT_R16G16B16_UINT:
				case VK_FORMAT_R16G16B16_SINT:
					return ImageFormat::R16G16B16;
				case VK_FORMAT_R16G16B16A16_UNORM:
				case VK_FORMAT_R16G16B16A16_SNORM:
				case VK_FORMAT_R16G16B16A16_USCALED:
				case VK_FORMAT_R16G16B16A16_SSCALED:
				case VK_FORMAT_R16G16B16A16_UINT:
				case VK_FORMAT_R16G16B16A16_SINT:
					return ImageFormat::R16G16B16A16;
				case VK_FORMAT_R32_UINT:
				case VK_FORMAT_R32_SINT:
					return ImageFormat::R32;
				case VK_FORMAT_R32_SFLOAT:
					return ImageFormat::R32F;
				case VK_FORMAT_R32G32_UINT:
				case VK_FORMAT_R32G32_SINT:
					return ImageFormat::R32G32;
				case VK_FORMAT_R32G32_SFLOAT:
					return ImageFormat::R32G32F;
				case VK_FORMAT_R32G32B32_UINT:
				case VK_FORMAT_R32G32B32_SINT:
					return ImageFormat::R32G32B32;
				case VK_FORMAT_R32G32B32_SFLOAT:
					return ImageFormat::R32G32B32F;
				case VK_FORMAT_R32G32B32A32_UINT:
				case VK_FORMAT_R32G32B32A32_SINT:
					return ImageFormat::R32G32B32A32;
				case VK_FORMAT_R32G32B32A32_SFLOAT:
					return ImageFormat::R32G32B32A32F;
				default:
					break;
				}
				throw ZFileException("Vulkan2ImageFormat: vulkan format is not supported by z-image-format.");
			}


			//fmtUsage = UNDEF selects optimal format
			inline VkFormat ImageFormat2Vulkan(ImageFormat fmt, ImageFormatUsage fmtUsage = ImageFormatUsage::UNDEF)
			{
				switch (fmt)
				{
				case zenith::util::zfile_format::ImageFormat::UNDEF: return VK_FORMAT_UNDEFINED;
				case zenith::util::zfile_format::ImageFormat::R8:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R8_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R8_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R8_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R8_SINT;
					}
					break;
				}					
				case zenith::util::zfile_format::ImageFormat::R16:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R16_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R16_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R16_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R16_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R32_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R32_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32F:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::FLOAT:
						return VK_FORMAT_R32_SFLOAT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R8G8:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R8G8_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R8G8_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R8G8_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R8G8_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R16G16:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R16G16_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R16G16_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R16G16_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R16G16_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R32G32_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R32G32_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32F:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::FLOAT:
						return VK_FORMAT_R32G32_SFLOAT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R8G8B8:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R8G8B8_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R8G8B8_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R8G8B8_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R8G8B8_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R16G16B16:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R16G16B16_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R16G16B16_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R16G16B16_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R16G16B16_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32B32:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R32G32B32_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R32G32B32_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32B32F:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::FLOAT:
						return VK_FORMAT_R32G32B32_SFLOAT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R8G8B8A8:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R8G8B8A8_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R8G8B8A8_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R8G8B8A8_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R8G8B8A8_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R16G16B16A16:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UNORM:
						return VK_FORMAT_R16G16B16A16_UNORM;
					case zenith::util::zfile_format::ImageFormatUsage::SNORM:
						return VK_FORMAT_R16G16B16A16_SNORM;
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R16G16B16A16_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R16G16B16A16_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::UINT:
						return VK_FORMAT_R32G32B32A32_UINT;
					case zenith::util::zfile_format::ImageFormatUsage::SINT:
						return VK_FORMAT_R32G32B32A32_SINT;
					}
					break;
				}
				case zenith::util::zfile_format::ImageFormat::R32G32B32A32F:
				{
					switch (fmtUsage)
					{
					case zenith::util::zfile_format::ImageFormatUsage::UNDEF:
					case zenith::util::zfile_format::ImageFormatUsage::FLOAT:
						return VK_FORMAT_R32G32B32A32_SFLOAT;
					}
					break;
				}
				default:
					break;
				}
				throw ZFileException("ImageFormat2Vulkan: unsupported combination of format and usage.");
			}
		}
	}
}