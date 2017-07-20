#include "vPipelineUtils.h"

using namespace zenith::gengraphics;
using namespace zenith::vulkan::util;



void zenith::vulkan::util::fillPipelineSetLayoutBinding(VkDescriptorSetLayoutBinding * slb, const zenith::gengraphics::ggPipelineResource &res)
{
	slb->binding = res.binding_location();
	slb->descriptorCount = 1; //should be updated, but not critical
	slb->pImmutableSamplers = nullptr; //should be updated, but not critical
	slb->descriptorType = convertDescriptorType(res.type(), res.is_dynamic());
	slb->stageFlags = convertShaderStageFlags(res.stages());
}

VkDescriptorSetLayout zenith::vulkan::util::createPipelineSetLayout(VkDevice dev, const zenith::gengraphics::ggPipelineLayoutGroup &group, VkAllocationCallbacks * pCallbacks)
{
	/*
		should be thread-safe, but currently will solve simply by dynamic memory allocation
		better solution involve mutexes and ring-buffer
		even better solution includes special atomic operations on top/bottom pointers
	*/
	std::vector<VkDescriptorSetLayoutBinding> slb;
	slb.reserve(group.size());
	for (const auto &r : group)
	{
		slb.emplace_back();
		fillPipelineSetLayoutBinding(&slb.back(), r);
	}
	VkDescriptorSetLayoutCreateInfo ci;
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;
	ci.bindingCount = group.size();
	ci.pBindings = &slb[0];
	VkDescriptorSetLayout dsl;
	VkResult res = vkCreateDescriptorSetLayout(dev, &ci, pCallbacks, &dsl);
	if (res != VK_SUCCESS)
		throw VulkanException(res, "createPipelineSetLayout(): failed during vulkan-call to vkCreateDescriptorSetLayout.");
	return dsl;
}

VkPipelineLayout zenith::vulkan::util::createPipelineLayout(VkDevice dev, VkDescriptorSetLayout * pDSL, uint32_t numDSL, VkAllocationCallbacks * pCallbacks)
{
	VkPipelineLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.setLayoutCount = numDSL;
	ci.pSetLayouts = pDSL;

	ci.pushConstantRangeCount = 0;
	ci.pPushConstantRanges = nullptr;

	VkPipelineLayout pl;
	VkResult res = vkCreatePipelineLayout(dev, &ci, pCallbacks, &pl);
	if(res != VK_SUCCESS)
		throw VulkanException(res, "createPipelineLayout(): failed during vulkan-call to vkCreatePipelineLayout.");
	return pl;
}

VkPipelineLayout zenith::vulkan::util::createPipelineLayout(VkDevice dev, const zenith::gengraphics::ggPipelineLayout &layout, VkAllocationCallbacks * pCallbacks)
{
	for(uint32_t i = 0; i < layout.num_groups(); i++)
		if(layout.at(i).binding_group() != i)
			throw VulkanException("createPipelineLayout(): groups are not consecutive and starting from zero.");
	//destroy on exception and after execution
	zenith::vulkan::vAutoArray<VkDescriptorSetLayout> layouts(layout.num_groups(), vkDestroyDescriptorSetLayout, dev, pCallbacks);
	for (uint32_t i = 0; i < layouts.size(); i++)
		layouts[i] = createPipelineSetLayout(dev, layout.at(i), pCallbacks);

	return createPipelineLayout(dev, &layouts[0], layouts.size(), pCallbacks);
}

VkDescriptorPool zenith::vulkan::util::createPipelineDescriptorPool(VkDevice dev, const zenith::gengraphics::ggPipelineLayout & layout, uint32_t maxSets, VkAllocationCallbacks * pCallbacks)
{
	std::vector<uint32_t> typeNums(VK_DESCRIPTOR_TYPE_RANGE_SIZE, 0);
	
	for (const auto &r : layout)
	{
		VkDescriptorType dt = convertDescriptorType(r.type(), r.is_dynamic());
		typeNums[dt - VK_DESCRIPTOR_TYPE_BEGIN_RANGE]++;
	}
	uint32_t numDesc = 0;
	for (uint32_t i = 0; i < typeNums.size(); i++)
		if (typeNums[i])
			numDesc++;

	std::vector<VkDescriptorPoolSize> psz(numDesc);

	for (uint32_t i = 0, j = 0; i < typeNums.size(); i++)
	{
		if (typeNums[i] == 0)
			continue;
		psz[j].type = static_cast<VkDescriptorType>(i + VK_DESCRIPTOR_TYPE_BEGIN_RANGE);
		psz[j].descriptorCount = typeNums[i];
		j++;
	}

	VkDescriptorPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.maxSets = maxSets;
	ci.poolSizeCount = static_cast<uint32_t>(psz.size());
	ci.pPoolSizes = &psz[0];
	VkDescriptorPool dp;
	VkResult res = vkCreateDescriptorPool(dev, &ci, pCallbacks, &dp);
	if (res != VK_SUCCESS)
		throw VulkanException(res, "createPipelineDescriptorPool(): failed during vulkan-call to vkCreateDescriptorPool.");
	return dp;
}

void zenith::vulkan::util::fillPipelineProgramDepthStencil(VkPipelineDepthStencilStateCreateInfo & ci, const zenith::gengraphics::ggPipelineProgramDepth & d, const zenith::gengraphics::ggPipelineProgramStencil & s)
{
	ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.depthTestEnable = (d.is_testable() ? VK_TRUE : VK_FALSE);
	ci.depthWriteEnable = (d.is_writable() ? VK_TRUE : VK_FALSE);
	if (d.is_testable())
		ci.depthCompareOp = convertCompareOp(d.compare_op());
	else
		ci.depthCompareOp = VK_COMPARE_OP_NEVER;

	ci.depthBoundsTestEnable = VK_FALSE;
	ci.minDepthBounds = 0.0f;
	ci.maxDepthBounds = 1.0f;

	ci.stencilTestEnable = (s.is_enabled() ? VK_TRUE : VK_FALSE);
	ci.front = {};
	ci.back = {};
}

void zenith::vulkan::util::fillPipelineProgramRasterize(VkPipelineRasterizationStateCreateInfo & ci, const zenith::gengraphics::ggPipelineProgramRasterize & r)
{
	ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.depthClampEnable = VK_FALSE;

	ci.depthBiasEnable = VK_FALSE;
	ci.depthBiasClamp = 0.0f;
	ci.depthBiasSlopeFactor = 0.0f;
	ci.depthBiasConstantFactor = 0.0f;

	ci.rasterizerDiscardEnable = (r.is_discardable() ? VK_TRUE : VK_FALSE);
	
	ci.polygonMode = VK_POLYGON_MODE_FILL;
	ci.lineWidth = 1.0f;
	ci.cullMode = convertCullMode(r.cull_mode());

	if (r.cull_mode() == zenith::gengraphics::ggCullMode::NONE)
		ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
	else
		ci.frontFace = convertFaceDef(r.face_def());
}

void zenith::vulkan::util::fillPipelineProgramMultisample(VkPipelineMultisampleStateCreateInfo & ci, const zenith::gengraphics::ggPipelineProgramMultisample & m)
{
	ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.sampleShadingEnable = VK_FALSE;
	ci.rasterizationSamples = static_cast<VkSampleCountFlagBits>(m.num_samples());
	ci.minSampleShading = 1.0f;
	ci.pSampleMask = nullptr;
	ci.alphaToCoverageEnable = VK_FALSE;
	ci.alphaToOneEnable = VK_FALSE;
}
