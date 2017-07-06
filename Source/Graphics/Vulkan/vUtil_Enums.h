
#pragma once
#include <Utils\ioconv\io_config.h>
#include <Utils\macro_version.h>
#include <Utils\nameid.h>
#include <Graphics\Vulkan\vulkan_config.h>
#include <vector>
#include <list>

#pragma warning(push)
#pragma warning(disable:4101)

namespace zenith
{
	namespace vulkan
	{
		enum class vSwapchainResult {SUCCESS=0, TIMEOUT=VK_TIMEOUT, NOT_READY=VK_NOT_READY, SUBOPTIMAL=VK_SUBOPTIMAL_KHR};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSwapchainResult)
		ZENITH_ENUM2STR_ELEM(SUCCESS)
		ZENITH_ENUM2STR_ELEM(TIMEOUT)
		ZENITH_ENUM2STR_ELEM(NOT_READY)
		ZENITH_ENUM2STR_ELEM(SUBOPTIMAL)
		ZENITH_ENUM2STR_END


		enum class vSamplerAddress {REPEAT=VK_SAMPLER_ADDRESS_MODE_REPEAT, REPEAT_MIRROR=VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT, CLAMP=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE, CLAMP_BORDER=VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSamplerAddress)
		ZENITH_ENUM2STR_ELEM(REPEAT)
		ZENITH_ENUM2STR_ELEM(REPEAT_MIRROR)
		ZENITH_ENUM2STR_ELEM(CLAMP)
		ZENITH_ENUM2STR_ELEM(CLAMP_BORDER)
		ZENITH_ENUM2STR_END


		enum class vSamplerBorderColor {UNDEF=0, TRANSPARENT_BLACK, OPAQUE_BLACK, OPAQUE_WHITE};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSamplerBorderColor)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(TRANSPARENT_BLACK)
		ZENITH_ENUM2STR_ELEM(OPAQUE_BLACK)
		ZENITH_ENUM2STR_ELEM(OPAQUE_WHITE)
		ZENITH_ENUM2STR_END


		enum class vSamplerBorderType {UNDEF=0, INT, FLOAT};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSamplerBorderType)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(INT)
		ZENITH_ENUM2STR_ELEM(FLOAT)
		ZENITH_ENUM2STR_END


		enum class vSamplerFilter {NEAREST=0, LINEAR=1};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSamplerFilter)
		ZENITH_ENUM2STR_ELEM(NEAREST)
		ZENITH_ENUM2STR_ELEM(LINEAR)
		ZENITH_ENUM2STR_END


		enum class vPhysicalDeviceType {UNDEF=0, INTEGRATED_GPU=VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, DISCRETE_GPU=VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU, VIRTUAL_GPU=VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU, CPU=VK_PHYSICAL_DEVICE_TYPE_CPU};
		ZENITH_ENUM2STR_START(zenith::vulkan::vPhysicalDeviceType)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(INTEGRATED_GPU)
		ZENITH_ENUM2STR_ELEM(DISCRETE_GPU)
		ZENITH_ENUM2STR_ELEM(VIRTUAL_GPU)
		ZENITH_ENUM2STR_ELEM(CPU)
		ZENITH_ENUM2STR_END


		enum class vImageFormat {UNDEF=0, R8G8B8A8_UNORM=VK_FORMAT_R8G8B8A8_UNORM, B8G8R8A8_UNORM=VK_FORMAT_B8G8R8A8_UNORM, B8G8R8A8_SRGB=VK_FORMAT_B8G8R8A8_SRGB};
		ZENITH_ENUM2STR_START(zenith::vulkan::vImageFormat)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(R8G8B8A8_UNORM)
		ZENITH_ENUM2STR_ELEM(B8G8R8A8_UNORM)
		ZENITH_ENUM2STR_ELEM(B8G8R8A8_SRGB)
		ZENITH_ENUM2STR_END


		enum class vImageUsageBit {UNDEF=0, TRANSFER_SRC=VK_IMAGE_USAGE_TRANSFER_SRC_BIT, TRANSFER_DST=VK_IMAGE_USAGE_TRANSFER_DST_BIT, SAMPLED=VK_IMAGE_USAGE_SAMPLED_BIT, STORAGE=VK_IMAGE_USAGE_STORAGE_BIT, ATTACHMENT_COLOR=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, ATTACHMENT_DEPTHSTENCIL=VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, ATTACHMENT_TRANSIENT=VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, ATTACHMENT_INPUT=VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT};
		ZENITH_ENUM2STR_START(zenith::vulkan::vImageUsageBit)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(TRANSFER_SRC)
		ZENITH_ENUM2STR_ELEM(TRANSFER_DST)
		ZENITH_ENUM2STR_ELEM(SAMPLED)
		ZENITH_ENUM2STR_ELEM(STORAGE)
		ZENITH_ENUM2STR_ELEM(ATTACHMENT_COLOR)
		ZENITH_ENUM2STR_ELEM(ATTACHMENT_DEPTHSTENCIL)
		ZENITH_ENUM2STR_ELEM(ATTACHMENT_TRANSIENT)
		ZENITH_ENUM2STR_ELEM(ATTACHMENT_INPUT)
		ZENITH_ENUM2STR_END


		enum class vMemoryUsage {UNDEF=0, GPU_STATIC, STAGING, MAX};
		ZENITH_ENUM2STR_START(zenith::vulkan::vMemoryUsage)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(GPU_STATIC)
		ZENITH_ENUM2STR_ELEM(STAGING)
		ZENITH_ENUM2STR_ELEM(MAX)
		ZENITH_ENUM2STR_END


		enum class vMemoryLayout {UNDEF=0, LINEAR=1, OPTIMAL=2};
		ZENITH_ENUM2STR_START(zenith::vulkan::vMemoryLayout)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(LINEAR)
		ZENITH_ENUM2STR_ELEM(OPTIMAL)
		ZENITH_ENUM2STR_END


		enum class vMemoryPropertyBit {UNDEF=0, DEVICE_LOCAL=VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, HOST_VISIBLE=VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, HOST_COHERENT=VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, HOST_CACHED=VK_MEMORY_PROPERTY_HOST_CACHED_BIT, LAZY=VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT};
		ZENITH_ENUM2STR_START(zenith::vulkan::vMemoryPropertyBit)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(DEVICE_LOCAL)
		ZENITH_ENUM2STR_ELEM(HOST_VISIBLE)
		ZENITH_ENUM2STR_ELEM(HOST_COHERENT)
		ZENITH_ENUM2STR_ELEM(HOST_CACHED)
		ZENITH_ENUM2STR_ELEM(LAZY)
		ZENITH_ENUM2STR_END


		enum class vPresentMode {UNDEF=0, IMMEDIATE=VK_PRESENT_MODE_IMMEDIATE_KHR, FIFO=VK_PRESENT_MODE_FIFO_KHR, FIFO_RELAXED=VK_PRESENT_MODE_FIFO_RELAXED_KHR, MAILBOX=VK_PRESENT_MODE_MAILBOX_KHR};
		ZENITH_ENUM2STR_START(zenith::vulkan::vPresentMode)
		ZENITH_ENUM2STR_ELEM(UNDEF)
		ZENITH_ENUM2STR_ELEM(IMMEDIATE)
		ZENITH_ENUM2STR_ELEM(FIFO)
		ZENITH_ENUM2STR_ELEM(FIFO_RELAXED)
		ZENITH_ENUM2STR_ELEM(MAILBOX)
		ZENITH_ENUM2STR_END


		enum class vSharingMode {EXCLUSIVE=VK_SHARING_MODE_EXCLUSIVE, CONCURRENT=VK_SHARING_MODE_CONCURRENT};
		ZENITH_ENUM2STR_START(zenith::vulkan::vSharingMode)
		ZENITH_ENUM2STR_ELEM(EXCLUSIVE)
		ZENITH_ENUM2STR_ELEM(CONCURRENT)
		ZENITH_ENUM2STR_END

	}
}


#pragma warning(pop)
