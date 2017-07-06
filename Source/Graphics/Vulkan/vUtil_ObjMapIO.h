
#pragma once
#include <Utils\object_map.h>
#include "vUtil.h"
#include "vUtilRaw.h"

#pragma warning(push)
#pragma warning(disable:4101)

namespace zenith
{
	namespace vulkan
	{
		inline void to_objmap(const vLayerConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vLayerConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("name", obj.name.c_str(), zenith::util::ObjectMapValueHint::ATTR);
			for (auto x : obj.requiredExtensions)
				om.addValue("requiredExtensions", x.c_str());
		}
		inline void from_objmap(vLayerConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.name, "name");
			{
				auto v = om.getValues("requiredExtensions");
				for (auto it = v.first; it != v.second; it++)
					obj.requiredExtensions.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vInstanceConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vInstanceConfig", zenith::util::ObjectMapValueHint::ATTR);

			for (auto x : obj.enabledLayers)
				to_objmap(x, om.addObject("enabledLayers"));
			for (auto x : obj.requiredExtensions)
				om.addValue("requiredExtensions", x.c_str());
			om.addValue("appName", obj.appName.c_str());
			om.addValue("engineName", obj.engineName.c_str());
			zenith::util::str_cast(obj.debug, buff, 128);
			om.addValue("debug", buff);
			{
				auto &r=om.addObject("appVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.appVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.appVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.appVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			{
				auto &r=om.addObject("engineVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.engineVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.engineVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.engineVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			{
				auto &r=om.addObject("apiVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.apiVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.apiVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.apiVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
		}
		inline void from_objmap(vInstanceConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			{
				auto v = om.getObjects("enabledLayers");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.enabledLayers.resize(obj.enabledLayers.size()+1);
					from_objmap(obj.enabledLayers.back(), it->second);
				}
			}
			{
				auto v = om.getValues("requiredExtensions");
				for (auto it = v.first; it != v.second; it++)
					obj.requiredExtensions.emplace_back(it->second.first.c_str());
			}
			OBJMAP_GET_ONE_VALUE(om, obj.appName, "appName");
			OBJMAP_GET_ONE_VALUE(om, obj.engineName, "engineName");
			OBJMAP_GET_ONE_VALUE(om, obj.debug, "debug");
			{
				auto &r = om.getObjects("appVersion", zenith::util::ObjectMapPresence::ONE).first->second;
				uint32_t major,minor,patch;
				OBJMAP_GET_ONE_VALUE(r, major, "major");
				OBJMAP_GET_ONE_VALUE(r, minor, "minor");
				OBJMAP_GET_ONE_VALUE(r, patch, "patch");
				obj.appVersion = ZENITH_MAKE_VERSION(major, minor, patch);
			}
			{
				auto &r = om.getObjects("engineVersion", zenith::util::ObjectMapPresence::ONE).first->second;
				uint32_t major,minor,patch;
				OBJMAP_GET_ONE_VALUE(r, major, "major");
				OBJMAP_GET_ONE_VALUE(r, minor, "minor");
				OBJMAP_GET_ONE_VALUE(r, patch, "patch");
				obj.engineVersion = ZENITH_MAKE_VERSION(major, minor, patch);
			}
			{
				auto &r = om.getObjects("apiVersion", zenith::util::ObjectMapPresence::ONE).first->second;
				uint32_t major,minor,patch;
				OBJMAP_GET_ONE_VALUE(r, major, "major");
				OBJMAP_GET_ONE_VALUE(r, minor, "minor");
				OBJMAP_GET_ONE_VALUE(r, patch, "patch");
				obj.apiVersion = ZENITH_MAKE_VERSION(major, minor, patch);
			}
		}


		inline void to_objmap(const vSamplerFilteringConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSamplerFilteringConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.minFilter, buff, 128);
			om.addValue("minFilter", buff);
			zenith::util::str_cast(obj.magFilter, buff, 128);
			om.addValue("magFilter", buff);
			zenith::util::str_cast(obj.mipFilter, buff, 128);
			om.addValue("mipFilter", buff);
			zenith::util::str_cast(obj.maxAnisotropy, buff, 128);
			om.addValue("maxAnisotropy", buff);
		}
		inline void from_objmap(vSamplerFilteringConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minFilter, "minFilter", zenith::vulkan::vSamplerFilter::LINEAR);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.magFilter, "magFilter", zenith::vulkan::vSamplerFilter::LINEAR);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.mipFilter, "mipFilter", zenith::vulkan::vSamplerFilter::LINEAR);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.maxAnisotropy, "maxAnisotropy", 0.0f);
		}


		inline void to_objmap(const vSamplerAddressingConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSamplerAddressingConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.u, buff, 128);
			om.addValue("u", buff);
			zenith::util::str_cast(obj.v, buff, 128);
			om.addValue("v", buff);
			zenith::util::str_cast(obj.w, buff, 128);
			om.addValue("w", buff);
			zenith::util::str_cast(obj.borderColor, buff, 128);
			om.addValue("borderColor", buff);
			zenith::util::str_cast(obj.borderType, buff, 128);
			om.addValue("borderType", buff);
			zenith::util::str_cast(obj.normalizedCoord, buff, 128);
			om.addValue("normalizedCoord", buff);
		}
		inline void from_objmap(vSamplerAddressingConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.u, "u", zenith::vulkan::vSamplerAddress::CLAMP);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.v, "v", zenith::vulkan::vSamplerAddress::CLAMP);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.w, "w", zenith::vulkan::vSamplerAddress::CLAMP);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.borderColor, "borderColor", zenith::vulkan::vSamplerBorderColor::OPAQUE_BLACK);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.borderType, "borderType", zenith::vulkan::vSamplerBorderType::FLOAT);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.normalizedCoord, "normalizedCoord", true);
		}


		inline void to_objmap(const vSamplerLODConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSamplerLODConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.minLOD, buff, 128);
			om.addValue("minLOD", buff);
			zenith::util::str_cast(obj.maxLOD, buff, 128);
			om.addValue("maxLOD", buff);
			zenith::util::str_cast(obj.biasLOD, buff, 128);
			om.addValue("biasLOD", buff);
		}
		inline void from_objmap(vSamplerLODConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.minLOD, "minLOD", 0.0f);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.maxLOD, "maxLOD", 1024.0f);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.biasLOD, "biasLOD", 0.0f);
		}


		inline void to_objmap(const VkLayerProperties &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "VkLayerProperties", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("layerName", obj.layerName);
			{
				auto &r=om.addObject("specVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.specVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.specVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.specVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			{
				auto &r=om.addObject("implementationVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.implementationVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.implementationVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.implementationVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			om.addValue("description", obj.description);
		}


		inline void to_objmap(const VkExtensionProperties &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "VkExtensionProperties", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("extensionName", obj.extensionName);
			{
				auto &r=om.addObject("specVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.specVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.specVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.specVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
		}


		inline void to_objmap(const VkPhysicalDeviceLimits &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "VkPhysicalDeviceLimits", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.maxImageDimension1D, buff, 128);
			om.addValue("maxImageDimension1D", buff);
			zenith::util::str_cast(obj.maxImageDimension2D, buff, 128);
			om.addValue("maxImageDimension2D", buff);
			zenith::util::str_cast(obj.maxImageDimension3D, buff, 128);
			om.addValue("maxImageDimension3D", buff);
			zenith::util::str_cast(obj.maxImageDimensionCube, buff, 128);
			om.addValue("maxImageDimensionCube", buff);
			zenith::util::str_cast(obj.maxImageArrayLayers, buff, 128);
			om.addValue("maxImageArrayLayers", buff);
			zenith::util::str_cast(obj.maxTexelBufferElements, buff, 128);
			om.addValue("maxTexelBufferElements", buff);
			zenith::util::str_cast(obj.maxUniformBufferRange, buff, 128);
			om.addValue("maxUniformBufferRange", buff);
			zenith::util::str_cast(obj.maxStorageBufferRange, buff, 128);
			om.addValue("maxStorageBufferRange", buff);
			zenith::util::str_cast(obj.maxPushConstantsSize, buff, 128);
			om.addValue("maxPushConstantsSize", buff);
			zenith::util::str_cast(obj.maxMemoryAllocationCount, buff, 128);
			om.addValue("maxMemoryAllocationCount", buff);
			zenith::util::str_cast(obj.maxSamplerAllocationCount, buff, 128);
			om.addValue("maxSamplerAllocationCount", buff);
			zenith::util::str_cast(obj.bufferImageGranularity, buff, 128);
			om.addValue("bufferImageGranularity", buff);
			zenith::util::str_cast(obj.sparseAddressSpaceSize, buff, 128);
			om.addValue("sparseAddressSpaceSize", buff);
			zenith::util::str_cast(obj.maxBoundDescriptorSets, buff, 128);
			om.addValue("maxBoundDescriptorSets", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorSamplers, buff, 128);
			om.addValue("maxPerStageDescriptorSamplers", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorUniformBuffers, buff, 128);
			om.addValue("maxPerStageDescriptorUniformBuffers", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorStorageBuffers, buff, 128);
			om.addValue("maxPerStageDescriptorStorageBuffers", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorSampledImages, buff, 128);
			om.addValue("maxPerStageDescriptorSampledImages", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorStorageImages, buff, 128);
			om.addValue("maxPerStageDescriptorStorageImages", buff);
			zenith::util::str_cast(obj.maxPerStageDescriptorInputAttachments, buff, 128);
			om.addValue("maxPerStageDescriptorInputAttachments", buff);
			zenith::util::str_cast(obj.maxPerStageResources, buff, 128);
			om.addValue("maxPerStageResources", buff);
			zenith::util::str_cast(obj.maxDescriptorSetSamplers, buff, 128);
			om.addValue("maxDescriptorSetSamplers", buff);
			zenith::util::str_cast(obj.maxDescriptorSetUniformBuffers, buff, 128);
			om.addValue("maxDescriptorSetUniformBuffers", buff);
			zenith::util::str_cast(obj.maxDescriptorSetUniformBuffersDynamic, buff, 128);
			om.addValue("maxDescriptorSetUniformBuffersDynamic", buff);
			zenith::util::str_cast(obj.maxDescriptorSetStorageBuffers, buff, 128);
			om.addValue("maxDescriptorSetStorageBuffers", buff);
			zenith::util::str_cast(obj.maxDescriptorSetStorageBuffersDynamic, buff, 128);
			om.addValue("maxDescriptorSetStorageBuffersDynamic", buff);
			zenith::util::str_cast(obj.maxDescriptorSetSampledImages, buff, 128);
			om.addValue("maxDescriptorSetSampledImages", buff);
			zenith::util::str_cast(obj.maxDescriptorSetStorageImages, buff, 128);
			om.addValue("maxDescriptorSetStorageImages", buff);
			zenith::util::str_cast(obj.maxDescriptorSetInputAttachments, buff, 128);
			om.addValue("maxDescriptorSetInputAttachments", buff);
			zenith::util::str_cast(obj.maxVertexInputAttributes, buff, 128);
			om.addValue("maxVertexInputAttributes", buff);
			zenith::util::str_cast(obj.maxVertexInputBindings, buff, 128);
			om.addValue("maxVertexInputBindings", buff);
			zenith::util::str_cast(obj.maxVertexInputAttributeOffset, buff, 128);
			om.addValue("maxVertexInputAttributeOffset", buff);
			zenith::util::str_cast(obj.maxVertexInputBindingStride, buff, 128);
			om.addValue("maxVertexInputBindingStride", buff);
			zenith::util::str_cast(obj.maxVertexOutputComponents, buff, 128);
			om.addValue("maxVertexOutputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationGenerationLevel, buff, 128);
			om.addValue("maxTessellationGenerationLevel", buff);
			zenith::util::str_cast(obj.maxTessellationPatchSize, buff, 128);
			om.addValue("maxTessellationPatchSize", buff);
			zenith::util::str_cast(obj.maxTessellationControlPerVertexInputComponents, buff, 128);
			om.addValue("maxTessellationControlPerVertexInputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationControlPerVertexOutputComponents, buff, 128);
			om.addValue("maxTessellationControlPerVertexOutputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationControlPerPatchOutputComponents, buff, 128);
			om.addValue("maxTessellationControlPerPatchOutputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationControlTotalOutputComponents, buff, 128);
			om.addValue("maxTessellationControlTotalOutputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationEvaluationInputComponents, buff, 128);
			om.addValue("maxTessellationEvaluationInputComponents", buff);
			zenith::util::str_cast(obj.maxTessellationEvaluationOutputComponents, buff, 128);
			om.addValue("maxTessellationEvaluationOutputComponents", buff);
			zenith::util::str_cast(obj.maxGeometryShaderInvocations, buff, 128);
			om.addValue("maxGeometryShaderInvocations", buff);
			zenith::util::str_cast(obj.maxGeometryInputComponents, buff, 128);
			om.addValue("maxGeometryInputComponents", buff);
			zenith::util::str_cast(obj.maxGeometryOutputComponents, buff, 128);
			om.addValue("maxGeometryOutputComponents", buff);
			zenith::util::str_cast(obj.maxGeometryOutputVertices, buff, 128);
			om.addValue("maxGeometryOutputVertices", buff);
			zenith::util::str_cast(obj.maxGeometryTotalOutputComponents, buff, 128);
			om.addValue("maxGeometryTotalOutputComponents", buff);
			zenith::util::str_cast(obj.maxFragmentInputComponents, buff, 128);
			om.addValue("maxFragmentInputComponents", buff);
			zenith::util::str_cast(obj.maxFragmentOutputAttachments, buff, 128);
			om.addValue("maxFragmentOutputAttachments", buff);
			zenith::util::str_cast(obj.maxFragmentDualSrcAttachments, buff, 128);
			om.addValue("maxFragmentDualSrcAttachments", buff);
			zenith::util::str_cast(obj.maxFragmentCombinedOutputResources, buff, 128);
			om.addValue("maxFragmentCombinedOutputResources", buff);
			zenith::util::str_cast(obj.maxComputeSharedMemorySize, buff, 128);
			om.addValue("maxComputeSharedMemorySize", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupInvocations, buff, 128);
			om.addValue("maxComputeWorkGroupInvocations", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupCount[0], buff, 128);
			om.addValue("maxComputeWorkGroupCount[0]", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupSize[0], buff, 128);
			om.addValue("maxComputeWorkGroupSize[0]", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupCount[1], buff, 128);
			om.addValue("maxComputeWorkGroupCount[1]", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupSize[1], buff, 128);
			om.addValue("maxComputeWorkGroupSize[1]", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupCount[2], buff, 128);
			om.addValue("maxComputeWorkGroupCount[2]", buff);
			zenith::util::str_cast(obj.maxComputeWorkGroupSize[2], buff, 128);
			om.addValue("maxComputeWorkGroupSize[2]", buff);
			zenith::util::str_cast(obj.subPixelPrecisionBits, buff, 128);
			om.addValue("subPixelPrecisionBits", buff);
			zenith::util::str_cast(obj.subTexelPrecisionBits, buff, 128);
			om.addValue("subTexelPrecisionBits", buff);
			zenith::util::str_cast(obj.mipmapPrecisionBits, buff, 128);
			om.addValue("mipmapPrecisionBits", buff);
			zenith::util::str_cast(obj.maxDrawIndexedIndexValue, buff, 128);
			om.addValue("maxDrawIndexedIndexValue", buff);
			zenith::util::str_cast(obj.maxDrawIndirectCount, buff, 128);
			om.addValue("maxDrawIndirectCount", buff);
			zenith::util::str_cast(obj.maxSamplerLodBias, buff, 128);
			om.addValue("maxSamplerLodBias", buff);
			zenith::util::str_cast(obj.maxSamplerAnisotropy, buff, 128);
			om.addValue("maxSamplerAnisotropy", buff);
			zenith::util::str_cast(obj.maxViewports, buff, 128);
			om.addValue("maxViewports", buff);
			zenith::util::str_cast(obj.maxViewportDimensions[0], buff, 128);
			om.addValue("maxViewportDimensions[0]", buff);
			zenith::util::str_cast(obj.maxViewportDimensions[1], buff, 128);
			om.addValue("maxViewportDimensions[1]", buff);
			zenith::util::str_cast(obj.viewportBoundsRange[0], buff, 128);
			om.addValue("viewportBoundsRange[0]", buff);
			zenith::util::str_cast(obj.viewportBoundsRange[1], buff, 128);
			om.addValue("viewportBoundsRange[1]", buff);
			zenith::util::str_cast(obj.viewportSubPixelBits, buff, 128);
			om.addValue("viewportSubPixelBits", buff);
			zenith::util::str_cast(obj.minMemoryMapAlignment, buff, 128);
			om.addValue("minMemoryMapAlignment", buff);
			zenith::util::str_cast(obj.minTexelBufferOffsetAlignment, buff, 128);
			om.addValue("minTexelBufferOffsetAlignment", buff);
			zenith::util::str_cast(obj.minUniformBufferOffsetAlignment, buff, 128);
			om.addValue("minUniformBufferOffsetAlignment", buff);
			zenith::util::str_cast(obj.minStorageBufferOffsetAlignment, buff, 128);
			om.addValue("minStorageBufferOffsetAlignment", buff);
			zenith::util::str_cast(obj.minTexelOffset, buff, 128);
			om.addValue("minTexelOffset", buff);
			zenith::util::str_cast(obj.maxTexelOffset, buff, 128);
			om.addValue("maxTexelOffset", buff);
			zenith::util::str_cast(obj.minTexelGatherOffset, buff, 128);
			om.addValue("minTexelGatherOffset", buff);
			zenith::util::str_cast(obj.maxTexelGatherOffset, buff, 128);
			om.addValue("maxTexelGatherOffset", buff);
			zenith::util::str_cast(obj.minInterpolationOffset, buff, 128);
			om.addValue("minInterpolationOffset", buff);
			zenith::util::str_cast(obj.maxInterpolationOffset, buff, 128);
			om.addValue("maxInterpolationOffset", buff);
			zenith::util::str_cast(obj.subPixelInterpolationOffsetBits, buff, 128);
			om.addValue("subPixelInterpolationOffsetBits", buff);
			zenith::util::str_cast(obj.maxFramebufferWidth, buff, 128);
			om.addValue("maxFramebufferWidth", buff);
			zenith::util::str_cast(obj.maxFramebufferHeight, buff, 128);
			om.addValue("maxFramebufferHeight", buff);
			zenith::util::str_cast(obj.maxFramebufferLayers, buff, 128);
			om.addValue("maxFramebufferLayers", buff);
			zenith::util::str_cast(obj.maxColorAttachments, buff, 128);
			om.addValue("maxColorAttachments", buff);
			zenith::util::str_cast(obj.maxSampleMaskWords, buff, 128);
			om.addValue("maxSampleMaskWords", buff);
			zenith::util::str_cast(obj.timestampComputeAndGraphics, buff, 128);
			om.addValue("timestampComputeAndGraphics", buff);
			zenith::util::str_cast(obj.timestampPeriod, buff, 128);
			om.addValue("timestampPeriod", buff);
			zenith::util::str_cast(obj.maxClipDistances, buff, 128);
			om.addValue("maxClipDistances", buff);
			zenith::util::str_cast(obj.maxCullDistances, buff, 128);
			om.addValue("maxCullDistances", buff);
			zenith::util::str_cast(obj.maxCombinedClipAndCullDistances, buff, 128);
			om.addValue("maxCombinedClipAndCullDistances", buff);
			zenith::util::str_cast(obj.discreteQueuePriorities, buff, 128);
			om.addValue("discreteQueuePriorities", buff);
			zenith::util::str_cast(obj.strictLines, buff, 128);
			om.addValue("strictLines", buff);
			zenith::util::str_cast(obj.standardSampleLocations, buff, 128);
			om.addValue("standardSampleLocations", buff);
			zenith::util::str_cast(obj.optimalBufferCopyOffsetAlignment, buff, 128);
			om.addValue("optimalBufferCopyOffsetAlignment", buff);
			zenith::util::str_cast(obj.optimalBufferCopyRowPitchAlignment, buff, 128);
			om.addValue("optimalBufferCopyRowPitchAlignment", buff);
			zenith::util::str_cast(obj.nonCoherentAtomSize, buff, 128);
			om.addValue("nonCoherentAtomSize", buff);
			zenith::util::str_cast(obj.pointSizeGranularity, buff, 128);
			om.addValue("pointSizeGranularity", buff);
			zenith::util::str_cast(obj.lineWidthGranularity, buff, 128);
			om.addValue("lineWidthGranularity", buff);
			zenith::util::str_cast(obj.pointSizeRange[0], buff, 128);
			om.addValue("pointSizeRange[0]", buff);
			zenith::util::str_cast(obj.pointSizeRange[1], buff, 128);
			om.addValue("pointSizeRange[1]", buff);
			zenith::util::str_cast(obj.lineWidthRange[0], buff, 128);
			om.addValue("lineWidthRange[0]", buff);
			zenith::util::str_cast(obj.lineWidthRange[1], buff, 128);
			om.addValue("lineWidthRange[1]", buff);
			zenith::util::str_cast(obj.framebufferColorSampleCounts, buff, 128);
			om.addValue("framebufferColorSampleCounts", buff);
			zenith::util::str_cast(obj.framebufferDepthSampleCounts, buff, 128);
			om.addValue("framebufferDepthSampleCounts", buff);
			zenith::util::str_cast(obj.framebufferStencilSampleCounts, buff, 128);
			om.addValue("framebufferStencilSampleCounts", buff);
			zenith::util::str_cast(obj.framebufferNoAttachmentsSampleCounts, buff, 128);
			om.addValue("framebufferNoAttachmentsSampleCounts", buff);
			zenith::util::str_cast(obj.sampledImageColorSampleCounts, buff, 128);
			om.addValue("sampledImageColorSampleCounts", buff);
			zenith::util::str_cast(obj.sampledImageIntegerSampleCounts, buff, 128);
			om.addValue("sampledImageIntegerSampleCounts", buff);
			zenith::util::str_cast(obj.sampledImageDepthSampleCounts, buff, 128);
			om.addValue("sampledImageDepthSampleCounts", buff);
			zenith::util::str_cast(obj.sampledImageStencilSampleCounts, buff, 128);
			om.addValue("sampledImageStencilSampleCounts", buff);
			zenith::util::str_cast(obj.storageImageSampleCounts, buff, 128);
			om.addValue("storageImageSampleCounts", buff);
		}


		inline void to_objmap(const VkPhysicalDeviceProperties &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "VkPhysicalDeviceProperties", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("deviceName", obj.deviceName);
			zenith::util::str_cast(obj.vendorID, buff, 128);
			om.addValue("vendorID", buff);
			zenith::util::str_cast(obj.deviceID, buff, 128);
			om.addValue("deviceID", buff);
			{
				auto &r=om.addObject("driverVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.driverVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.driverVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.driverVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			{
				auto &r=om.addObject("apiVersion");
				r.addValue("type", "version", zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MAJOR(obj.apiVersion), buff, 128);
				r.addValue("major", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_MINOR(obj.apiVersion), buff, 128);
				r.addValue("minor", buff, zenith::util::ObjectMapValueHint::ATTR);
				zenith::util::str_cast(ZENITH_VERSION_PATCH(obj.apiVersion), buff, 128);
				r.addValue("patch", buff, zenith::util::ObjectMapValueHint::ATTR);
			}
			zenith::util::str_cast(static_cast<vPhysicalDeviceType>(obj.deviceType), buff, 128);
			om.addValue("deviceType", buff);
			to_objmap(obj.limits, om.addObject("limits"));
		}


		inline void to_objmap(const vDeviceQueueReqs &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vDeviceQueueReqs", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			om.addValue("surfaceUID", obj.surfaceUID.c_str());
			zenith::util::str_cast(obj.fGraphics, buff, 128);
			om.addValue("fGraphics", buff);
			zenith::util::str_cast(obj.fCompute, buff, 128);
			om.addValue("fCompute", buff);
			zenith::util::str_cast(obj.fTransfer, buff, 128);
			om.addValue("fTransfer", buff);
			zenith::util::str_cast(obj.fSparse, buff, 128);
			om.addValue("fSparse", buff);
			zenith::util::str_cast(obj.fPresent, buff, 128);
			om.addValue("fPresent", buff);
			zenith::util::str_cast(obj.priority, buff, 128);
			om.addValue("priority", buff);
			for (auto x : obj.aliases)
				om.addValue("aliases", x.c_str());
		}
		inline void from_objmap(vDeviceQueueReqs &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.surfaceUID, "surfaceUID", "UNDEF");
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.fGraphics, "fGraphics", false);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.fCompute, "fCompute", false);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.fTransfer, "fTransfer", false);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.fSparse, "fSparse", false);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.fPresent, "fPresent", false);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.priority, "priority", 1.0f);
			{
				auto v = om.getValues("aliases");
				for (auto it = v.first; it != v.second; it++)
					obj.aliases.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vDeviceReqs &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vDeviceReqs", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			zenith::util::str_cast(obj.physicalType, buff, 128);
			om.addValue("physicalType", buff);
			zenith::util::str_cast(obj.memorySize, buff, 128);
			om.addValue("memorySize", buff);
			for (auto x : obj.queues)
				to_objmap(x, om.addObject("queues"));
			for (auto x : obj.enabledLayers)
				to_objmap(x, om.addObject("enabledLayers"));
			for (auto x : obj.requiredExtensions)
				om.addValue("requiredExtensions", x.c_str());
		}
		inline void from_objmap(vDeviceReqs &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.physicalType, "physicalType");
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.memorySize, "memorySize", 536870912);
			{
				auto v = om.getObjects("queues");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.queues.resize(obj.queues.size()+1);
					from_objmap(obj.queues.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("enabledLayers");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.enabledLayers.resize(obj.enabledLayers.size()+1);
					from_objmap(obj.enabledLayers.back(), it->second);
				}
			}
			{
				auto v = om.getValues("requiredExtensions");
				for (auto it = v.first; it != v.second; it++)
					obj.requiredExtensions.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vDeviceConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vDeviceConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			for (auto x : obj.deviceReqs)
				to_objmap(x, om.addObject("deviceReqs"));
			for (auto x : obj.deviceReqOrder)
				om.addValue("deviceReqOrder", x.c_str());
		}
		inline void from_objmap(vDeviceConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			{
				auto v = om.getObjects("deviceReqs");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.deviceReqs.resize(obj.deviceReqs.size()+1);
					from_objmap(obj.deviceReqs.back(), it->second);
				}
			}
			{
				auto v = om.getValues("deviceReqOrder");
				for (auto it = v.first; it != v.second; it++)
					obj.deviceReqOrder.emplace_back(it->second.first.c_str());
			}
		}




		inline void to_objmap(const vSharingInfo &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSharingInfo", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.mode, buff, 128);
			om.addValue("mode", buff);
			for (auto x : obj.queues)
				om.addValue("queues", x.c_str());
		}
		inline void from_objmap(vSharingInfo &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.mode, "mode");
			{
				auto v = om.getValues("queues");
				for (auto it = v.first; it != v.second; it++)
					obj.queues.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vSwapchainReqs &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSwapchainReqs", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			to_objmap(obj.sharingInfo, om.addObject("sharingInfo"));
			zenith::util::str_cast(obj.presentMode, buff, 128);
			om.addValue("presentMode", buff);
			zenith::util::str_cast(obj.imageCount, buff, 128);
			om.addValue("imageCount", buff);
			zenith::util::str_cast(obj.imageFormat, buff, 128);
			om.addValue("imageFormat", buff);
			for (auto x : obj.imageUsage)
			{
				zenith::util::str_cast(x, buff, 128);
				om.addValue("imageUsage", buff);
			}
		}
		inline void from_objmap(vSwapchainReqs &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			from_objmap(obj.sharingInfo, om.getObjects("sharingInfo", zenith::util::ObjectMapPresence::ONE).first->second);
			OBJMAP_GET_ONE_VALUE(om, obj.presentMode, "presentMode");
			OBJMAP_GET_ONE_VALUE(om, obj.imageCount, "imageCount");
			OBJMAP_GET_ONE_VALUE(om, obj.imageFormat, "imageFormat");
			{
				auto v = om.getValues("imageUsage");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.imageUsage.resize(obj.imageUsage.size()+1);
					om.setValue(obj.imageUsage.back(), it);
				}
			}
		}


		inline void to_objmap(const vSwapchainConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSwapchainConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			om.addValue("surfaceUID", obj.surfaceUID.c_str());
			om.addValue("deviceUID", obj.deviceUID.c_str());
			for (auto x : obj.swapchainReqs)
				to_objmap(x, om.addObject("swapchainReqs"));
			for (auto x : obj.swapchainReqOrder)
				om.addValue("swapchainReqOrder", x.c_str());
		}
		inline void from_objmap(vSwapchainConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.surfaceUID, "surfaceUID");
			OBJMAP_GET_ONE_VALUE(om, obj.deviceUID, "deviceUID");
			{
				auto v = om.getObjects("swapchainReqs");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.swapchainReqs.resize(obj.swapchainReqs.size()+1);
					from_objmap(obj.swapchainReqs.back(), it->second);
				}
			}
			{
				auto v = om.getValues("swapchainReqOrder");
				for (auto it = v.first; it != v.second; it++)
					obj.swapchainReqOrder.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vSurfaceConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSurfaceConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			om.addValue("windowUID", obj.windowUID.c_str());
		}
		inline void from_objmap(vSurfaceConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.windowUID, "windowUID");
		}


		inline void to_objmap(const vMemoryPhysicalChunkConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryPhysicalChunkConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.minAllocSize, buff, 128);
			om.addValue("minAllocSize", buff);
			zenith::util::str_cast(obj.maxChunkSize, buff, 128);
			om.addValue("maxChunkSize", buff);
			zenith::util::str_cast(obj.defChunkSize, buff, 128);
			om.addValue("defChunkSize", buff);
			zenith::util::str_cast(obj.minChunkSize, buff, 128);
			om.addValue("minChunkSize", buff);
			zenith::util::str_cast(obj.align, buff, 128);
			om.addValue("align", buff);
		}
		inline void from_objmap(vMemoryPhysicalChunkConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.minAllocSize, "minAllocSize");
			OBJMAP_GET_ONE_VALUE(om, obj.maxChunkSize, "maxChunkSize");
			OBJMAP_GET_ONE_VALUE(om, obj.defChunkSize, "defChunkSize");
			OBJMAP_GET_ONE_VALUE(om, obj.minChunkSize, "minChunkSize");
			OBJMAP_GET_ONE_VALUE(om, obj.align, "align");
		}


		inline void to_objmap(const vMemoryLogicalChunkConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryLogicalChunkConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.maxAllocSize, buff, 128);
			om.addValue("maxAllocSize", buff);
			zenith::util::str_cast(obj.chunkSize, buff, 128);
			om.addValue("chunkSize", buff);
			zenith::util::str_cast(obj.blockSize, buff, 128);
			om.addValue("blockSize", buff);
			zenith::util::str_cast(obj.align, buff, 128);
			om.addValue("align", buff);
			zenith::util::str_cast(obj.subTypeMask, buff, 128);
			om.addValue("subTypeMask", buff);
		}
		inline void from_objmap(vMemoryLogicalChunkConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.maxAllocSize, "maxAllocSize");
			OBJMAP_GET_ONE_VALUE(om, obj.chunkSize, "chunkSize");
			OBJMAP_GET_ONE_VALUE(om, obj.blockSize, "blockSize");
			OBJMAP_GET_ONE_VALUE(om, obj.align, "align");
			OBJMAP_GET_ONE_VALUE(om, obj.subTypeMask, "subTypeMask");
		}


		inline void to_objmap(const vMemoryPoolConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryPoolConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			to_objmap(obj.physicalChunk, om.addObject("physicalChunk"));
			for (auto x : obj.logicalChunk)
				to_objmap(x, om.addObject("logicalChunk"));
		}
		inline void from_objmap(vMemoryPoolConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			from_objmap(obj.physicalChunk, om.getObjects("physicalChunk", zenith::util::ObjectMapPresence::ONE).first->second);
			{
				auto v = om.getObjects("logicalChunk");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.logicalChunk.resize(obj.logicalChunk.size()+1);
					from_objmap(obj.logicalChunk.back(), it->second);
				}
			}
		}


		inline void to_objmap(const vMemoryTypeConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryTypeConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.typeID, buff, 128);
			om.addValue("typeID", buff);
			zenith::util::str_cast(obj.isDeviceLocal, buff, 128);
			om.addValue("isDeviceLocal", buff);
			zenith::util::str_cast(obj.isHostVisible, buff, 128);
			om.addValue("isHostVisible", buff);
			zenith::util::str_cast(obj.isHostCoherent, buff, 128);
			om.addValue("isHostCoherent", buff);
			zenith::util::str_cast(obj.isHostCached, buff, 128);
			om.addValue("isHostCached", buff);
			zenith::util::str_cast(obj.isLazy, buff, 128);
			om.addValue("isLazy", buff);
			om.addValue("poolUID", obj.poolUID.c_str());
		}
		inline void from_objmap(vMemoryTypeConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.typeID, "typeID", -1);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.isDeviceLocal, "isDeviceLocal", zenith::util::ExtendedBitMask::ANY);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.isHostVisible, "isHostVisible", zenith::util::ExtendedBitMask::ANY);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.isHostCoherent, "isHostCoherent", zenith::util::ExtendedBitMask::ANY);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.isHostCached, "isHostCached", zenith::util::ExtendedBitMask::ANY);
			OBJMAP_GET_ONE_VALUE_DEFAULT(om, obj.isLazy, "isLazy", zenith::util::ExtendedBitMask::ANY);
			OBJMAP_GET_ONE_VALUE(om, obj.poolUID, "poolUID");
		}


		inline void to_objmap(const vMemoryUsageOption &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryUsageOption", zenith::util::ObjectMapValueHint::ATTR);

			for (auto x : obj.memoryPropertyBit)
			{
				zenith::util::str_cast(x, buff, 128);
				om.addValue("memoryPropertyBit", buff);
			}
		}
		inline void from_objmap(vMemoryUsageOption &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			{
				auto v = om.getValues("memoryPropertyBit");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryPropertyBit.resize(obj.memoryPropertyBit.size()+1);
					om.setValue(obj.memoryPropertyBit.back(), it);
				}
			}
		}


		inline void to_objmap(const vMemoryUsageConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryUsageConfig", zenith::util::ObjectMapValueHint::ATTR);

			zenith::util::str_cast(obj.usage, buff, 128);
			om.addValue("usage", buff);
			for (auto x : obj.memoryUsageOption)
				to_objmap(x, om.addObject("memoryUsageOption"));
		}
		inline void from_objmap(vMemoryUsageConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.usage, "usage");
			{
				auto v = om.getObjects("memoryUsageOption");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryUsageOption.resize(obj.memoryUsageOption.size()+1);
					from_objmap(obj.memoryUsageOption.back(), it->second);
				}
			}
		}


		inline void to_objmap(const vMemoryManagerConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryManagerConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			for (auto x : obj.memoryType)
				to_objmap(x, om.addObject("memoryType"));
			for (auto x : obj.memoryUsage)
				to_objmap(x, om.addObject("memoryUsage"));
		}
		inline void from_objmap(vMemoryManagerConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			{
				auto v = om.getObjects("memoryType");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryType.resize(obj.memoryType.size()+1);
					from_objmap(obj.memoryType.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("memoryUsage");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryUsage.resize(obj.memoryUsage.size()+1);
					from_objmap(obj.memoryUsage.back(), it->second);
				}
			}
		}


		inline void to_objmap(const vMemoryConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vMemoryConfig", zenith::util::ObjectMapValueHint::ATTR);

			om.addValue("uid", obj.uid.c_str());
			om.addValue("deviceUID", obj.deviceUID.c_str());
			for (auto x : obj.memoryPool)
				to_objmap(x, om.addObject("memoryPool"));
			for (auto x : obj.memoryManager)
				to_objmap(x, om.addObject("memoryManager"));
			for (auto x : obj.memoryManagerReqOrder)
				om.addValue("memoryManagerReqOrder", x.c_str());
		}
		inline void from_objmap(vMemoryConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			OBJMAP_GET_ONE_VALUE(om, obj.uid, "uid");
			OBJMAP_GET_ONE_VALUE(om, obj.deviceUID, "deviceUID");
			{
				auto v = om.getObjects("memoryPool");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryPool.resize(obj.memoryPool.size()+1);
					from_objmap(obj.memoryPool.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("memoryManager");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memoryManager.resize(obj.memoryManager.size()+1);
					from_objmap(obj.memoryManager.back(), it->second);
				}
			}
			{
				auto v = om.getValues("memoryManagerReqOrder");
				for (auto it = v.first; it != v.second; it++)
					obj.memoryManagerReqOrder.emplace_back(it->second.first.c_str());
			}
		}


		inline void to_objmap(const vSystemConfig &obj, zenith::util::ObjectMap<char, char> &om)
		{
			char buff[128];
			om.addValue("type", "vSystemConfig", zenith::util::ObjectMapValueHint::ATTR);

			to_objmap(obj.instance, om.addObject("instance"));
			for (auto x : obj.device)
				to_objmap(x, om.addObject("device"));
			for (auto x : obj.memory)
				to_objmap(x, om.addObject("memory"));
			for (auto x : obj.surface)
				to_objmap(x, om.addObject("surface"));
			for (auto x : obj.swapchain)
				to_objmap(x, om.addObject("swapchain"));
		}
		inline void from_objmap(vSystemConfig &obj, const zenith::util::ObjectMap<char, char> &om)
		{
			from_objmap(obj.instance, om.getObjects("instance", zenith::util::ObjectMapPresence::ONE).first->second);
			{
				auto v = om.getObjects("device");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.device.resize(obj.device.size()+1);
					from_objmap(obj.device.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("memory");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.memory.resize(obj.memory.size()+1);
					from_objmap(obj.memory.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("surface");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.surface.resize(obj.surface.size()+1);
					from_objmap(obj.surface.back(), it->second);
				}
			}
			{
				auto v = om.getObjects("swapchain");
				for (auto it = v.first; it != v.second; it++)
				{
					obj.swapchain.resize(obj.swapchain.size()+1);
					from_objmap(obj.swapchain.back(), it->second);
				}
			}
		}

	}
}


#pragma warning(pop)
