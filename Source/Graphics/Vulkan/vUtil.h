
#pragma once
#include <Utils\ioconv\io_config.h>
#include <Utils\macro_version.h>
#include <Utils\nameid.h>
#include <Graphics\Vulkan\vulkan_config.h>
#include <vector>
#include <list>
#include "vUtil_Enums.h"

#pragma warning(push)
#pragma warning(disable:4101)

namespace zenith
{
	namespace vulkan
	{
		struct vLayerConfig
		{
			std::string name;
			std::vector<std::string> requiredExtensions;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vLayerConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vLayerConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vLayerConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.name, it, "name");
				zenith::util::ioconv::input_named_multiple(val.requiredExtensions, it, "requiredExtensions");
			}
			inline static void output(const zenith::vulkan::vLayerConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.name, it.append_value("name", zenith::util::ioconv::NodeValueHint::ATTRIBUTE));
				zenith::util::ioconv::output_multiple(val.requiredExtensions, it, "requiredExtensions");
			}
		};


		struct vInstanceConfig
		{
			std::vector<vLayerConfig> enabledLayers;
			std::vector<std::string> requiredExtensions;
			std::string appName;
			std::string engineName;
			bool debug;
			zenith::version32_t appVersion;
			zenith::version32_t engineVersion;
			zenith::version32_t apiVersion;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vInstanceConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vInstanceConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vInstanceConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_multiple(val.enabledLayers, it, "enabledLayers");
				zenith::util::ioconv::input_named_multiple(val.requiredExtensions, it, "requiredExtensions");
				zenith::util::ioconv::input_named_required(val.appName, it, "appName");
				zenith::util::ioconv::input_named_required(val.engineName, it, "engineName");
				zenith::util::ioconv::input_named_required(val.debug, it, "debug");
				zenith::util::ioconv::input_named_required(val.appVersion, it, "appVersion");
				zenith::util::ioconv::input_named_required(val.engineVersion, it, "engineVersion");
				zenith::util::ioconv::input_named_required(val.apiVersion, it, "apiVersion");
			}
			inline static void output(const zenith::vulkan::vInstanceConfig &val, It &it)
			{
				zenith::util::ioconv::output_multiple(val.enabledLayers, it, "enabledLayers");
				zenith::util::ioconv::output_multiple(val.requiredExtensions, it, "requiredExtensions");
				zenith::util::ioconv::output_single(val.appName, it.append_value("appName"));
				zenith::util::ioconv::output_single(val.engineName, it.append_value("engineName"));
				zenith::util::ioconv::output_single(val.debug, it.append_value("debug"));
				zenith::util::ioconv::output_single(val.appVersion, it.append_complex("appVersion"));
				zenith::util::ioconv::output_single(val.engineVersion, it.append_complex("engineVersion"));
				zenith::util::ioconv::output_single(val.apiVersion, it.append_complex("apiVersion"));
			}
		};


		struct vSamplerFilteringConfig
		{
			vSamplerFilter minFilter;
			vSamplerFilter magFilter;
			vSamplerFilter mipFilter;
			float maxAnisotropy;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSamplerFilteringConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSamplerFilteringConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSamplerFilteringConfig &val, const It &it)
			{
				val.minFilter = zenith::vulkan::vSamplerFilter::LINEAR;
				zenith::util::ioconv::input_named_optional(val.minFilter, it, "minFilter");
				val.magFilter = zenith::vulkan::vSamplerFilter::LINEAR;
				zenith::util::ioconv::input_named_optional(val.magFilter, it, "magFilter");
				val.mipFilter = zenith::vulkan::vSamplerFilter::LINEAR;
				zenith::util::ioconv::input_named_optional(val.mipFilter, it, "mipFilter");
				val.maxAnisotropy = 0.0f;
				zenith::util::ioconv::input_named_optional(val.maxAnisotropy, it, "maxAnisotropy");
			}
			inline static void output(const zenith::vulkan::vSamplerFilteringConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.minFilter, it.append_value("minFilter"));
				zenith::util::ioconv::output_single(val.magFilter, it.append_value("magFilter"));
				zenith::util::ioconv::output_single(val.mipFilter, it.append_value("mipFilter"));
				zenith::util::ioconv::output_single(val.maxAnisotropy, it.append_value("maxAnisotropy"));
			}
		};


		struct vSamplerAddressingConfig
		{
			vSamplerAddress u;
			vSamplerAddress v;
			vSamplerAddress w;
			vSamplerBorderColor borderColor;
			vSamplerBorderType borderType;
			bool normalizedCoord;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSamplerAddressingConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSamplerAddressingConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSamplerAddressingConfig &val, const It &it)
			{
				val.u = zenith::vulkan::vSamplerAddress::CLAMP;
				zenith::util::ioconv::input_named_optional(val.u, it, "u");
				val.v = zenith::vulkan::vSamplerAddress::CLAMP;
				zenith::util::ioconv::input_named_optional(val.v, it, "v");
				val.w = zenith::vulkan::vSamplerAddress::CLAMP;
				zenith::util::ioconv::input_named_optional(val.w, it, "w");
				val.borderColor = zenith::vulkan::vSamplerBorderColor::OPAQUE_BLACK;
				zenith::util::ioconv::input_named_optional(val.borderColor, it, "borderColor");
				val.borderType = zenith::vulkan::vSamplerBorderType::FLOAT;
				zenith::util::ioconv::input_named_optional(val.borderType, it, "borderType");
				val.normalizedCoord = true;
				zenith::util::ioconv::input_named_optional(val.normalizedCoord, it, "normalizedCoord");
			}
			inline static void output(const zenith::vulkan::vSamplerAddressingConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.u, it.append_value("u"));
				zenith::util::ioconv::output_single(val.v, it.append_value("v"));
				zenith::util::ioconv::output_single(val.w, it.append_value("w"));
				zenith::util::ioconv::output_single(val.borderColor, it.append_value("borderColor"));
				zenith::util::ioconv::output_single(val.borderType, it.append_value("borderType"));
				zenith::util::ioconv::output_single(val.normalizedCoord, it.append_value("normalizedCoord"));
			}
		};


		struct vSamplerLODConfig
		{
			float minLOD;
			float maxLOD;
			float biasLOD;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSamplerLODConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSamplerLODConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSamplerLODConfig &val, const It &it)
			{
				val.minLOD = 0.0f;
				zenith::util::ioconv::input_named_optional(val.minLOD, it, "minLOD");
				val.maxLOD = 1024.0f;
				zenith::util::ioconv::input_named_optional(val.maxLOD, it, "maxLOD");
				val.biasLOD = 0.0f;
				zenith::util::ioconv::input_named_optional(val.biasLOD, it, "biasLOD");
			}
			inline static void output(const zenith::vulkan::vSamplerLODConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.minLOD, it.append_value("minLOD"));
				zenith::util::ioconv::output_single(val.maxLOD, it.append_value("maxLOD"));
				zenith::util::ioconv::output_single(val.biasLOD, it.append_value("biasLOD"));
			}
		};


		struct vDeviceQueueReqs
		{
			zenith::util::nameid uid;
			zenith::util::nameid surfaceUID;
			bool fGraphics;
			bool fCompute;
			bool fTransfer;
			bool fSparse;
			bool fPresent;
			float priority;
			std::vector<zenith::util::nameid> aliases;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vDeviceQueueReqs, It, intType>
		{
		public:
			typedef zenith::vulkan::vDeviceQueueReqs value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vDeviceQueueReqs &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				val.surfaceUID = "UNDEF";
				zenith::util::ioconv::input_named_optional(val.surfaceUID, it, "surfaceUID");
				val.fGraphics = false;
				zenith::util::ioconv::input_named_optional(val.fGraphics, it, "fGraphics");
				val.fCompute = false;
				zenith::util::ioconv::input_named_optional(val.fCompute, it, "fCompute");
				val.fTransfer = false;
				zenith::util::ioconv::input_named_optional(val.fTransfer, it, "fTransfer");
				val.fSparse = false;
				zenith::util::ioconv::input_named_optional(val.fSparse, it, "fSparse");
				val.fPresent = false;
				zenith::util::ioconv::input_named_optional(val.fPresent, it, "fPresent");
				val.priority = 1.0f;
				zenith::util::ioconv::input_named_optional(val.priority, it, "priority");
				zenith::util::ioconv::input_named_multiple(val.aliases, it, "aliases");
			}
			inline static void output(const zenith::vulkan::vDeviceQueueReqs &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.surfaceUID, it.append_value("surfaceUID"));
				zenith::util::ioconv::output_single(val.fGraphics, it.append_value("fGraphics"));
				zenith::util::ioconv::output_single(val.fCompute, it.append_value("fCompute"));
				zenith::util::ioconv::output_single(val.fTransfer, it.append_value("fTransfer"));
				zenith::util::ioconv::output_single(val.fSparse, it.append_value("fSparse"));
				zenith::util::ioconv::output_single(val.fPresent, it.append_value("fPresent"));
				zenith::util::ioconv::output_single(val.priority, it.append_value("priority"));
				zenith::util::ioconv::output_multiple(val.aliases, it, "aliases");
			}
		};


		struct vDeviceReqs
		{
			zenith::util::nameid uid;
			vPhysicalDeviceType physicalType;
			uint64_t memorySize;
			std::vector<vDeviceQueueReqs> queues;
			std::vector<vLayerConfig> enabledLayers;
			std::vector<std::string> requiredExtensions;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vDeviceReqs, It, intType>
		{
		public:
			typedef zenith::vulkan::vDeviceReqs value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vDeviceReqs &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.physicalType, it, "physicalType");
				val.memorySize = 536870912;
				zenith::util::ioconv::input_named_optional(val.memorySize, it, "memorySize");
				zenith::util::ioconv::input_named_multiple(val.queues, it, "queues");
				zenith::util::ioconv::input_named_multiple(val.enabledLayers, it, "enabledLayers");
				zenith::util::ioconv::input_named_multiple(val.requiredExtensions, it, "requiredExtensions");
			}
			inline static void output(const zenith::vulkan::vDeviceReqs &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.physicalType, it.append_value("physicalType"));
				zenith::util::ioconv::output_single(val.memorySize, it.append_value("memorySize"));
				zenith::util::ioconv::output_multiple(val.queues, it, "queues");
				zenith::util::ioconv::output_multiple(val.enabledLayers, it, "enabledLayers");
				zenith::util::ioconv::output_multiple(val.requiredExtensions, it, "requiredExtensions");
			}
		};


		struct vDeviceConfig
		{
			zenith::util::nameid uid;
			std::vector<vDeviceReqs> deviceReqs;
			std::vector<std::string> deviceReqOrder;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vDeviceConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vDeviceConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vDeviceConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_multiple(val.deviceReqs, it, "deviceReqs");
				zenith::util::ioconv::input_named_multiple(val.deviceReqOrder, it, "deviceReqOrder");
			}
			inline static void output(const zenith::vulkan::vDeviceConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_multiple(val.deviceReqs, it, "deviceReqs");
				zenith::util::ioconv::output_multiple(val.deviceReqOrder, it, "deviceReqOrder");
			}
		};


		struct vQueueFamilyProperties
		{
			VkQueueFamilyProperties generalProperties;
			zenith::util::nameid_seq supportedSurfaces;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vQueueFamilyProperties, It, intType>
		{
		public:
			typedef zenith::vulkan::vQueueFamilyProperties value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
		};


		struct vSharingInfo
		{
			vSharingMode mode;
			std::vector<std::string> queues;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSharingInfo, It, intType>
		{
		public:
			typedef zenith::vulkan::vSharingInfo value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSharingInfo &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.mode, it, "mode");
				zenith::util::ioconv::input_named_multiple(val.queues, it, "queues");
			}
			inline static void output(const zenith::vulkan::vSharingInfo &val, It &it)
			{
				zenith::util::ioconv::output_single(val.mode, it.append_value("mode"));
				zenith::util::ioconv::output_multiple(val.queues, it, "queues");
			}
		};


		struct vSwapchainReqs
		{
			zenith::util::nameid uid;
			vSharingInfo sharingInfo;
			vPresentMode presentMode;
			uint32_t imageCount;
			vImageFormat imageFormat;
			std::vector<vImageUsageBit> imageUsage;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSwapchainReqs, It, intType>
		{
		public:
			typedef zenith::vulkan::vSwapchainReqs value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSwapchainReqs &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.sharingInfo, it, "sharingInfo");
				zenith::util::ioconv::input_named_required(val.presentMode, it, "presentMode");
				zenith::util::ioconv::input_named_required(val.imageCount, it, "imageCount");
				zenith::util::ioconv::input_named_required(val.imageFormat, it, "imageFormat");
				zenith::util::ioconv::input_named_multiple(val.imageUsage, it, "imageUsage");
			}
			inline static void output(const zenith::vulkan::vSwapchainReqs &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.sharingInfo, it.append_complex("sharingInfo"));
				zenith::util::ioconv::output_single(val.presentMode, it.append_value("presentMode"));
				zenith::util::ioconv::output_single(val.imageCount, it.append_value("imageCount"));
				zenith::util::ioconv::output_single(val.imageFormat, it.append_value("imageFormat"));
				zenith::util::ioconv::output_multiple(val.imageUsage, it, "imageUsage");
			}
		};


		struct vSwapchainConfig
		{
			zenith::util::nameid uid;
			zenith::util::nameid surfaceUID;
			zenith::util::nameid deviceUID;
			std::vector<vSwapchainReqs> swapchainReqs;
			std::vector<std::string> swapchainReqOrder;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSwapchainConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSwapchainConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSwapchainConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.surfaceUID, it, "surfaceUID");
				zenith::util::ioconv::input_named_required(val.deviceUID, it, "deviceUID");
				zenith::util::ioconv::input_named_multiple(val.swapchainReqs, it, "swapchainReqs");
				zenith::util::ioconv::input_named_multiple(val.swapchainReqOrder, it, "swapchainReqOrder");
			}
			inline static void output(const zenith::vulkan::vSwapchainConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.surfaceUID, it.append_value("surfaceUID"));
				zenith::util::ioconv::output_single(val.deviceUID, it.append_value("deviceUID"));
				zenith::util::ioconv::output_multiple(val.swapchainReqs, it, "swapchainReqs");
				zenith::util::ioconv::output_multiple(val.swapchainReqOrder, it, "swapchainReqOrder");
			}
		};


		struct vSurfaceConfig
		{
			zenith::util::nameid uid;
			zenith::util::nameid windowUID;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSurfaceConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSurfaceConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSurfaceConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.windowUID, it, "windowUID");
			}
			inline static void output(const zenith::vulkan::vSurfaceConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.windowUID, it.append_value("windowUID"));
			}
		};


		struct vMemoryPhysicalChunkConfig
		{
			size_t minAllocSize;
			size_t maxChunkSize;
			size_t defChunkSize;
			size_t minChunkSize;
			size_t align;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryPhysicalChunkConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryPhysicalChunkConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryPhysicalChunkConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.minAllocSize, it, "minAllocSize");
				zenith::util::ioconv::input_named_required(val.maxChunkSize, it, "maxChunkSize");
				zenith::util::ioconv::input_named_required(val.defChunkSize, it, "defChunkSize");
				zenith::util::ioconv::input_named_required(val.minChunkSize, it, "minChunkSize");
				zenith::util::ioconv::input_named_required(val.align, it, "align");
			}
			inline static void output(const zenith::vulkan::vMemoryPhysicalChunkConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.minAllocSize, it.append_value("minAllocSize"));
				zenith::util::ioconv::output_single(val.maxChunkSize, it.append_value("maxChunkSize"));
				zenith::util::ioconv::output_single(val.defChunkSize, it.append_value("defChunkSize"));
				zenith::util::ioconv::output_single(val.minChunkSize, it.append_value("minChunkSize"));
				zenith::util::ioconv::output_single(val.align, it.append_value("align"));
			}
		};


		struct vMemoryLogicalChunkConfig
		{
			size_t maxAllocSize;
			size_t chunkSize;
			size_t blockSize;
			size_t align;
			uint32_t subTypeMask;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryLogicalChunkConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryLogicalChunkConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryLogicalChunkConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.maxAllocSize, it, "maxAllocSize");
				zenith::util::ioconv::input_named_required(val.chunkSize, it, "chunkSize");
				zenith::util::ioconv::input_named_required(val.blockSize, it, "blockSize");
				zenith::util::ioconv::input_named_required(val.align, it, "align");
				zenith::util::ioconv::input_named_required(val.subTypeMask, it, "subTypeMask");
			}
			inline static void output(const zenith::vulkan::vMemoryLogicalChunkConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.maxAllocSize, it.append_value("maxAllocSize"));
				zenith::util::ioconv::output_single(val.chunkSize, it.append_value("chunkSize"));
				zenith::util::ioconv::output_single(val.blockSize, it.append_value("blockSize"));
				zenith::util::ioconv::output_single(val.align, it.append_value("align"));
				zenith::util::ioconv::output_single(val.subTypeMask, it.append_value("subTypeMask"));
			}
		};


		struct vMemoryPoolConfig
		{
			zenith::util::nameid uid;
			vMemoryPhysicalChunkConfig physicalChunk;
			std::vector<vMemoryLogicalChunkConfig> logicalChunk;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryPoolConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryPoolConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryPoolConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.physicalChunk, it, "physicalChunk");
				zenith::util::ioconv::input_named_multiple(val.logicalChunk, it, "logicalChunk");
			}
			inline static void output(const zenith::vulkan::vMemoryPoolConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.physicalChunk, it.append_complex("physicalChunk"));
				zenith::util::ioconv::output_multiple(val.logicalChunk, it, "logicalChunk");
			}
		};


		struct vMemoryTypeConfig
		{
			int8_t typeID;
			zenith::util::ExtendedBitMask isDeviceLocal;
			zenith::util::ExtendedBitMask isHostVisible;
			zenith::util::ExtendedBitMask isHostCoherent;
			zenith::util::ExtendedBitMask isHostCached;
			zenith::util::ExtendedBitMask isLazy;
			zenith::util::nameid poolUID;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryTypeConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryTypeConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryTypeConfig &val, const It &it)
			{
				val.typeID = -1;
				zenith::util::ioconv::input_named_optional(val.typeID, it, "typeID");
				val.isDeviceLocal = zenith::util::ExtendedBitMask::ANY;
				zenith::util::ioconv::input_named_optional(val.isDeviceLocal, it, "isDeviceLocal");
				val.isHostVisible = zenith::util::ExtendedBitMask::ANY;
				zenith::util::ioconv::input_named_optional(val.isHostVisible, it, "isHostVisible");
				val.isHostCoherent = zenith::util::ExtendedBitMask::ANY;
				zenith::util::ioconv::input_named_optional(val.isHostCoherent, it, "isHostCoherent");
				val.isHostCached = zenith::util::ExtendedBitMask::ANY;
				zenith::util::ioconv::input_named_optional(val.isHostCached, it, "isHostCached");
				val.isLazy = zenith::util::ExtendedBitMask::ANY;
				zenith::util::ioconv::input_named_optional(val.isLazy, it, "isLazy");
				zenith::util::ioconv::input_named_required(val.poolUID, it, "poolUID");
			}
			inline static void output(const zenith::vulkan::vMemoryTypeConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.typeID, it.append_value("typeID"));
				zenith::util::ioconv::output_single(val.isDeviceLocal, it.append_value("isDeviceLocal"));
				zenith::util::ioconv::output_single(val.isHostVisible, it.append_value("isHostVisible"));
				zenith::util::ioconv::output_single(val.isHostCoherent, it.append_value("isHostCoherent"));
				zenith::util::ioconv::output_single(val.isHostCached, it.append_value("isHostCached"));
				zenith::util::ioconv::output_single(val.isLazy, it.append_value("isLazy"));
				zenith::util::ioconv::output_single(val.poolUID, it.append_value("poolUID"));
			}
		};


		struct vMemoryUsageOption
		{
			std::vector<vMemoryPropertyBit> memoryPropertyBit;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryUsageOption, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryUsageOption value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryUsageOption &val, const It &it)
			{
				zenith::util::ioconv::input_named_multiple(val.memoryPropertyBit, it, "memoryPropertyBit");
			}
			inline static void output(const zenith::vulkan::vMemoryUsageOption &val, It &it)
			{
				zenith::util::ioconv::output_multiple(val.memoryPropertyBit, it, "memoryPropertyBit");
			}
		};


		struct vMemoryUsageConfig
		{
			vMemoryUsage usage;
			std::vector<vMemoryUsageOption> memoryUsageOption;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryUsageConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryUsageConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryUsageConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.usage, it, "usage");
				zenith::util::ioconv::input_named_multiple(val.memoryUsageOption, it, "memoryUsageOption");
			}
			inline static void output(const zenith::vulkan::vMemoryUsageConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.usage, it.append_value("usage"));
				zenith::util::ioconv::output_multiple(val.memoryUsageOption, it, "memoryUsageOption");
			}
		};


		struct vMemoryManagerConfig
		{
			zenith::util::nameid uid;
			std::vector<vMemoryTypeConfig> memoryType;
			std::vector<vMemoryUsageConfig> memoryUsage;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryManagerConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryManagerConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryManagerConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_multiple(val.memoryType, it, "memoryType");
				zenith::util::ioconv::input_named_multiple(val.memoryUsage, it, "memoryUsage");
			}
			inline static void output(const zenith::vulkan::vMemoryManagerConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_multiple(val.memoryType, it, "memoryType");
				zenith::util::ioconv::output_multiple(val.memoryUsage, it, "memoryUsage");
			}
		};


		struct vMemoryConfig
		{
			zenith::util::nameid uid;
			zenith::util::nameid deviceUID;
			std::vector<vMemoryPoolConfig> memoryPool;
			std::vector<vMemoryManagerConfig> memoryManager;
			std::vector<std::string> memoryManagerReqOrder;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vMemoryConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vMemoryConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vMemoryConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.uid, it, "uid");
				zenith::util::ioconv::input_named_required(val.deviceUID, it, "deviceUID");
				zenith::util::ioconv::input_named_multiple(val.memoryPool, it, "memoryPool");
				zenith::util::ioconv::input_named_multiple(val.memoryManager, it, "memoryManager");
				zenith::util::ioconv::input_named_multiple(val.memoryManagerReqOrder, it, "memoryManagerReqOrder");
			}
			inline static void output(const zenith::vulkan::vMemoryConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.uid, it.append_value("uid"));
				zenith::util::ioconv::output_single(val.deviceUID, it.append_value("deviceUID"));
				zenith::util::ioconv::output_multiple(val.memoryPool, it, "memoryPool");
				zenith::util::ioconv::output_multiple(val.memoryManager, it, "memoryManager");
				zenith::util::ioconv::output_multiple(val.memoryManagerReqOrder, it, "memoryManagerReqOrder");
			}
		};


		struct vSystemConfig
		{
			vInstanceConfig instance;
			std::vector<vDeviceConfig> device;
			std::vector<vMemoryConfig> memory;
			std::vector<vSurfaceConfig> surface;
			std::vector<vSwapchainConfig> swapchain;
		};
		template<class It, zenith::util::ioconv::InternalType intType> class zenith::util::ioconv::io_handler_impl<zenith::vulkan::vSystemConfig, It, intType>
		{
		public:
			typedef zenith::vulkan::vSystemConfig value_type;
			static const zenith::util::ioconv::NodeType node_type = zenith::util::ioconv::NodeType::COMPLEX;
			inline static void input(zenith::vulkan::vSystemConfig &val, const It &it)
			{
				zenith::util::ioconv::input_named_required(val.instance, it, "instance");
				zenith::util::ioconv::input_named_multiple(val.device, it, "device");
				zenith::util::ioconv::input_named_multiple(val.memory, it, "memory");
				zenith::util::ioconv::input_named_multiple(val.surface, it, "surface");
				zenith::util::ioconv::input_named_multiple(val.swapchain, it, "swapchain");
			}
			inline static void output(const zenith::vulkan::vSystemConfig &val, It &it)
			{
				zenith::util::ioconv::output_single(val.instance, it.append_complex("instance"));
				zenith::util::ioconv::output_multiple(val.device, it, "device");
				zenith::util::ioconv::output_multiple(val.memory, it, "memory");
				zenith::util::ioconv::output_multiple(val.surface, it, "surface");
				zenith::util::ioconv::output_multiple(val.swapchain, it, "swapchain");
			}
		};

	}
}


#pragma warning(pop)
