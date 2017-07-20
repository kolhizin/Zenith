#pragma once
#include "vulkan_general.h"
#include "../General/ggPipelineLayout.h"
#include "../General/ggPipelineFixedFunction.h"

namespace zenith
{
	namespace vulkan
	{
		namespace util
		{
			void fillPipelineSetLayoutBinding(VkDescriptorSetLayoutBinding * slb, const zenith::gengraphics::ggPipelineResource &res);
			VkDescriptorSetLayout createPipelineSetLayout(VkDevice dev, const zenith::gengraphics::ggPipelineLayoutGroup &group, VkAllocationCallbacks * pCallbacks = nullptr);
			VkPipelineLayout createPipelineLayout(VkDevice dev, VkDescriptorSetLayout * pDSL, uint32_t numDSL, VkAllocationCallbacks * pCallbacks = nullptr);
			VkPipelineLayout createPipelineLayout(VkDevice dev, const zenith::gengraphics::ggPipelineLayout &layout, VkAllocationCallbacks * pCallbacks = nullptr);
			VkDescriptorPool createPipelineDescriptorPool(VkDevice dev, const zenith::gengraphics::ggPipelineLayout &layout, uint32_t maxSets, VkAllocationCallbacks * pCallbacks = nullptr);

			void fillPipelineProgramDepthStencil(VkPipelineDepthStencilStateCreateInfo &ci, const zenith::gengraphics::ggPipelineProgramDepth &d, const zenith::gengraphics::ggPipelineProgramStencil &s);
			void fillPipelineProgramRasterize(VkPipelineRasterizationStateCreateInfo &ci, const zenith::gengraphics::ggPipelineProgramRasterize &r);
			void fillPipelineProgramMultisample(VkPipelineMultisampleStateCreateInfo &ci, const zenith::gengraphics::ggPipelineProgramMultisample &m);

			inline VkCullModeFlags convertCullMode(zenith::gengraphics::ggCullMode cull)
			{
				switch (cull)
				{
				case zenith::gengraphics::ggCullMode::NONE: return VK_CULL_MODE_NONE;
				case zenith::gengraphics::ggCullMode::FRONT: return VK_CULL_MODE_FRONT_BIT;
				case zenith::gengraphics::ggCullMode::BACK: return VK_CULL_MODE_BACK_BIT;
				case zenith::gengraphics::ggCullMode::FRONT_AND_BACK: return VK_CULL_MODE_FRONT_AND_BACK;
				}
				throw zenith::vulkan::VulkanException("convertCullMode(): unknown or undefined pipeline cull mode.");
			}
			inline VkCompareOp convertCompareOp(zenith::gengraphics::ggCompareOp op)
			{
				switch (op)
				{
				case zenith::gengraphics::ggCompareOp::LESS: return VK_COMPARE_OP_LESS;
				case zenith::gengraphics::ggCompareOp::LEQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
				case zenith::gengraphics::ggCompareOp::GREATER: return VK_COMPARE_OP_GREATER;
				case zenith::gengraphics::ggCompareOp::GEQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
				case zenith::gengraphics::ggCompareOp::EQUAL: return VK_COMPARE_OP_EQUAL;
				case zenith::gengraphics::ggCompareOp::NEQUAL: return VK_COMPARE_OP_NOT_EQUAL;
				}
				throw zenith::vulkan::VulkanException("convertCompareOp(): unknown or undefined pipeline depth compare op.");
			}
			inline VkFrontFace convertFaceDef(zenith::gengraphics::ggFaceDef def)
			{
				switch (def)
				{
				case zenith::gengraphics::ggFaceDef::CLOCKWISE: return VK_FRONT_FACE_CLOCKWISE;
				case zenith::gengraphics::ggFaceDef::COUNTER_CLOCKWISE: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
				}
				throw zenith::vulkan::VulkanException("convertFaceDef(): unknown or undefined pipeline face definition.");
			}

			inline VkDescriptorType convertDescriptorType(zenith::gengraphics::ggPipelineResourceType r, bool dynamic = false)
			{
				switch (r)
				{
				case zenith::gengraphics::ggPipelineResourceType::TEXTURE: return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
				case zenith::gengraphics::ggPipelineResourceType::SAMPLER: return VK_DESCRIPTOR_TYPE_SAMPLER;
				case zenith::gengraphics::ggPipelineResourceType::COMBINED_TEXTURE_SAMPLER: return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				case zenith::gengraphics::ggPipelineResourceType::UNIFORM_BUFFER: return (dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
				case zenith::gengraphics::ggPipelineResourceType::STORAGE_BUFFER: return (dynamic ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
				default:
					break;
				}
				throw zenith::vulkan::VulkanException("convertDescriptorType(): unknown or undefined pipeline resource type.");
			}

			inline VkShaderStageFlags convertShaderStageFlags(zenith::util::bitenum<zenith::gengraphics::ggPipelineStage> stages)
			{
				//should be updated to include all stages
				VkShaderStageFlags res = 0;
				if (stages.includes(zenith::gengraphics::ggPipelineStage::VERTEX_SHADER))
					res |= VK_SHADER_STAGE_VERTEX_BIT;
				if (stages.includes(zenith::gengraphics::ggPipelineStage::PIXEL_SHADER))
					res |= VK_SHADER_STAGE_FRAGMENT_BIT;
				return res;
			}
		}
	}
}