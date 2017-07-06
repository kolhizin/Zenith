#include "init.h"
#include <Utils\FileFormat\ff_util_vulkan.h>

std::unique_ptr<zenith::util::Window> wnd;
zenith::vulkan::vSystem * vSys = nullptr;
zenith::util::io::FileSystem * fs = nullptr;

std::string fnameVShader, fnameFShader;

VkShaderModule vertShader, fragShader;
VkPipelineShaderStageCreateInfo shaderStages[2];
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;
VkFramebuffer frameBuffers[3];
VkCommandPool comPool;
VkCommandBuffer comBuffers[3];
VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;
zenith::vulkan::vBufferAutoImpl_ * vertexBuffer, *indexBuffer, *cameraUniformBuffer;
std::array<bool, 256> keys;
VkDescriptorPool descrPool;
VkDescriptorSet descrSet;
VkDescriptorSetLayout descriptorSetLayout;
SCamera mainCamera;
std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> texture, depthMap;
zenith::vulkan::vSamplerImpl_ * sampler;
zenith::vulkan::vTextureViewRawImpl_ * textureView;


std::vector<Vertex> vertices = {
	{ { -1.f, -1.f, -1.f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 0.0f } },
	{ { -1.f,  1.f, -1.f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
	{ { 1.f,  1.f, -1.f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 1.0f } },
	{ { 1.f, -1.f, -1.f, 1.0f },{ 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f } },

	{ { -1.f, -1.f, 1.f, 1.0f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 0.0f } },
	{ { -1.f,  1.f, 1.f, 1.0f },{ 0.0f, 0.0f, -1.0f },{ 0.0f, 1.0f } },
	{ { 1.f,  1.f, 1.f, 1.0f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 1.0f } },
	{ { 1.f, -1.f, 1.f, 1.0f },{ 0.0f, 0.0f, -1.0f },{ 1.0f, 0.0f } },

	{ { 1.f, -1.f, -1.f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 1.f,  1.f, -1.f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
	{ { 1.f,  1.f,  1.f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } },
	{ { 1.f, -1.f,  1.f, 1.0f },{ 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },

	{ { -1.f, -1.f, -1.f, 1.0f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { -1.f,  1.f, -1.f, 1.0f },{ -1.0f, 0.0f, 0.0f },{ 0.0f, 1.0f } },
	{ { -1.f,  1.f,  1.f, 1.0f },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 1.0f } },
	{ { -1.f, -1.f,  1.f, 1.0f },{ -1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },

	{ { -1.f, 1.f, -1.f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 1.f, 1.f, -1.f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 0.0f, 1.0f } },
	{ { 1.f, 1.f,  1.f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 1.0f } },
	{ { -1.f, 1.f,  1.f, 1.0f },{ 0.0f, 1.0f, 0.0f },{ 1.0f, 0.0f } },

	{ { -1.f, -1.f, -1.f, 1.0f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f } },
	{ { 1.f, -1.f, -1.f, 1.0f },{ 0.0f, -1.0f, 0.0f },{ 0.0f, 1.0f } },
	{ { 1.f, -1.f,  1.f, 1.0f },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 1.0f } },
	{ { -1.f, -1.f,  1.f, 1.0f },{ 0.0f, -1.0f, 0.0f },{ 1.0f, 0.0f } },

};

std::vector<uint32_t> indices = {
	0, 1, 2, 0, 2, 3,
	6, 5, 4, 7, 6, 4,
	8, 9, 10, 8, 10, 11,
	14, 13, 12, 15, 14, 12,
	18, 17, 16, 19, 18, 16,
	20, 21, 22, 20, 22, 23
};


void createSemaphore(VkSemaphore &s)
{
	VkSemaphoreCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	if (vkCreateSemaphore(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &s) != VK_SUCCESS)
		throw std::runtime_error("Failed to create semaphore!");
}
void createCommandPool(VkCommandPool & cp, uint32_t qFamilyIndex)
{
	VkCommandPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;
	ci.queueFamilyIndex = qFamilyIndex;

	if (vkCreateCommandPool(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &cp) != VK_SUCCESS)
		throw std::runtime_error("Failed to create command pool!");
}
void createCommandBuffers(VkCommandPool &cp, VkCommandBuffer * cbs, size_t numBuff)
{
	VkCommandBufferAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.pNext = nullptr;
	ai.commandPool = cp;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = numBuff;
	if (vkAllocateCommandBuffers(vSys->getDevice("vdevice-main").getDevice(), &ai, cbs) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffers!");
}

void createDescriptorPool()
{
	VkDescriptorPoolCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	VkDescriptorPoolSize psz[2];
	psz[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	psz[0].descriptorCount = 1;
	psz[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	psz[1].descriptorCount = 1;

	ci.maxSets = 2;
	ci.poolSizeCount = 2;
	ci.pPoolSizes = &psz[0];

	if (vkCreateDescriptorPool(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &descrPool) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");

}

void createVertexBuffer()
{
	size_t sz = sizeof(vertices[0]) * vertices.size();
	vertexBuffer = new zenith::vulkan::vBufferAutoImpl_(const_cast<zenith::vulkan::vDeviceImpl_*>(vSys->getDevice("vdevice-main").rawImpl()),
		sz,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		zenith::vulkan::vMemoryUsage::STAGING,
		zenith::vulkan::vObjectSharingInfo());


	void * data = vertexBuffer->memoryBlock().map();
	memcpy(data, vertices.data(), sz);
	vertexBuffer->memoryBlock().unmap();
}
void createIndexBuffer()
{
	size_t sz = sizeof(uint32_t) * indices.size();
	indexBuffer = new zenith::vulkan::vBufferAutoImpl_(const_cast<zenith::vulkan::vDeviceImpl_*>(vSys->getDevice("vdevice-main").rawImpl()),
		sz,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		zenith::vulkan::vMemoryUsage::STAGING,
		zenith::vulkan::vObjectSharingInfo());


	void * data = indexBuffer->memoryBlock().map();
	memcpy(data, indices.data(), sz);
	indexBuffer->memoryBlock().unmap();
}
void writeCommandBuffer(VkCommandBuffer &cb, VkFramebuffer &fb)
{
	VkCommandBufferBeginInfo s_begin = {}; s_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; s_begin.pNext = nullptr; s_begin.pInheritanceInfo = nullptr;
	s_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	if (vkBeginCommandBuffer(cb, &s_begin) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin recording command buffer!");

	VkClearValue clearValues[2];
	clearValues[0].color = { 0.7f,0.7f,0.7f,1.0f };
	clearValues[1].depthStencil = { 1.0f,0 };
	VkRenderPassBeginInfo s_renderpass = {}; s_renderpass.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; s_renderpass.pNext = nullptr;
	s_renderpass.renderPass = renderPass;
	s_renderpass.framebuffer = fb;

	s_renderpass.clearValueCount = 2;
	s_renderpass.pClearValues = &clearValues[0];

	s_renderpass.renderArea.offset = { 0, 0 };
	s_renderpass.renderArea.extent.width = wnd->getSize().width;
	s_renderpass.renderArea.extent.height = wnd->getSize().height;

	vkCmdBeginRenderPass(cb, &s_renderpass, VK_SUBPASS_CONTENTS_INLINE);
	{
		vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descrSet, 0, nullptr);
		VkDeviceSize offset = 0;
		VkBuffer bf = vertexBuffer->handle();
		vkCmdBindVertexBuffers(cb, 0, 1, &bf, &offset);
		vkCmdBindIndexBuffer(cb, indexBuffer->handle(), 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(cb, indices.size(), 1, 0, 0, 0);
		//vkCmdDraw(cb, vertices.size(), 1, 0, 0);
	}
	vkCmdEndRenderPass(cb);

	if (vkEndCommandBuffer(cb) != VK_SUCCESS)
		throw std::runtime_error("Failed to record command buffer!");
}

void createFramebuffer(VkImageView v, VkFramebuffer &fb)
{
	VkImageView ivs[] = { v, depthMap->mainView().handle() };

	VkFramebufferCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.renderPass = renderPass;
	ci.attachmentCount = 2;
	ci.pAttachments = ivs;
	ci.width = wnd->getSize().width;
	ci.height = wnd->getSize().height;
	ci.layers = 1;

	VkResult res = vkCreateFramebuffer(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &fb);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create framebuffer!");
}

void createShaderModule(const char * code, size_t codeSize, VkShaderModule &shMod, VkPipelineShaderStageCreateInfo &sci, VkShaderStageFlagBits shaderStage)
{
	VkShaderModuleCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;
	ci.pCode = (const uint32_t *)code;
	ci.codeSize = codeSize;

	auto res = vkCreateShaderModule(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &shMod);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create shader module!");


	sci.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	sci.flags = 0;
	sci.pNext = nullptr;

	sci.stage = shaderStage;
	sci.module = shMod;
	sci.pName = "main";

	sci.pSpecializationInfo = nullptr;
}
void createInputAssembly(VkPipelineVertexInputStateCreateInfo &vis, VkPipelineInputAssemblyStateCreateInfo &ias)
{
	vis.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vis.flags = 0;
	vis.pNext = nullptr;
	vis.vertexAttributeDescriptionCount = 0;
	vis.pVertexAttributeDescriptions = nullptr;
	vis.vertexBindingDescriptionCount = 0;
	vis.pVertexBindingDescriptions = 0;

	ias.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ias.flags = 0;
	ias.pNext = nullptr;
	ias.primitiveRestartEnable = VK_TRUE;//VK_FALSE;
	ias.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
}
void createViewport(VkPipelineViewportStateCreateInfo &pvs, VkViewport &viewport, VkRect2D &scissor)
{
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = wnd->getSize().width;
	scissor.extent.height = wnd->getSize().height;

	viewport.width = (float)scissor.extent.width;
	viewport.height = (float)scissor.extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	pvs.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pvs.flags = 0;
	pvs.pNext = nullptr;
	pvs.viewportCount = 1;
	pvs.pViewports = &viewport;
	pvs.scissorCount = 1;
	pvs.pScissors = &scissor;
}
void createDepthStencilState(VkPipelineDepthStencilStateCreateInfo &ci)
{
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ci.pNext = nullptr;
	ci.flags = 0;
	ci.depthTestEnable = VK_TRUE;
	ci.depthWriteEnable = VK_TRUE;
	ci.depthCompareOp = VK_COMPARE_OP_LESS;
	ci.depthBoundsTestEnable = VK_FALSE;
	ci.minDepthBounds = 0.0f;
	ci.maxDepthBounds = 1.0f;

	ci.stencilTestEnable = VK_FALSE;
	ci.front = {};
	ci.back = {};
}
void createRasterization(VkPipelineRasterizationStateCreateInfo &prs, VkPipelineMultisampleStateCreateInfo &pms)
{
	prs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	prs.flags = 0;
	prs.pNext = nullptr;

	prs.depthClampEnable = VK_FALSE;

	prs.depthBiasEnable = VK_FALSE;
	prs.depthBiasClamp = 0.0f;
	prs.depthBiasSlopeFactor = 0.0f;
	prs.depthBiasConstantFactor = 0.0f;

	prs.rasterizerDiscardEnable = VK_FALSE;

	prs.cullMode = VK_CULL_MODE_BACK_BIT;
	prs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	prs.polygonMode = VK_POLYGON_MODE_FILL;
	prs.lineWidth = 1.0f;


	pms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pms.flags = 0;
	pms.pNext = nullptr;

	pms.sampleShadingEnable = VK_FALSE;
	pms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pms.minSampleShading = 1.0f;
	pms.pSampleMask = nullptr;
	pms.alphaToCoverageEnable = VK_FALSE;
	pms.alphaToOneEnable = VK_FALSE;
}
void createPipelineLayout(VkPipelineLayout &pl)
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.pImmutableSamplers = nullptr;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


	VkDescriptorSetLayoutBinding binds[] = { samplerLayoutBinding, uboLayoutBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 2;
	layoutInfo.pBindings = &binds[0];



	auto res = vkCreateDescriptorSetLayout(vSys->getDevice("vdevice-main").getDevice(), &layoutInfo, nullptr, &descriptorSetLayout);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor set layout!");

	VkPipelineLayoutCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;

	ci.setLayoutCount = 1;
	ci.pSetLayouts = &descriptorSetLayout;

	ci.pushConstantRangeCount = 0;
	ci.pPushConstantRanges = nullptr;

	res = vkCreatePipelineLayout(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &pl);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline layout!");
}

void createColorBlend(VkPipelineColorBlendAttachmentState &pcbas, VkPipelineColorBlendStateCreateInfo &pcbs_ci)
{
	pcbas.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pcbas.blendEnable = VK_FALSE;
	pcbas.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	pcbas.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	pcbas.colorBlendOp = VK_BLEND_OP_ADD;
	pcbas.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	pcbas.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	pcbas.alphaBlendOp = VK_BLEND_OP_ADD;

	pcbs_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pcbs_ci.flags = 0;
	pcbs_ci.pNext = nullptr;

	pcbs_ci.logicOpEnable = VK_FALSE;
	pcbs_ci.logicOp = VK_LOGIC_OP_COPY;
	pcbs_ci.attachmentCount = 1;
	pcbs_ci.pAttachments = &pcbas;
	pcbs_ci.blendConstants[0] = 0.0f;
	pcbs_ci.blendConstants[1] = 0.0f;
	pcbs_ci.blendConstants[2] = 0.0f;
	pcbs_ci.blendConstants[3] = 0.0f;
}

void createRenderPass(VkRenderPass &rp)
{
	std::array<VkAttachmentDescription, 2> ads;
	ads[0].flags = 0;
	ads[0].format = static_cast<VkFormat>(vSys->getDevice("vdevice-main").getSwapchain("vswapchain-main").getFormat());
	ads[0].samples = VK_SAMPLE_COUNT_1_BIT;
	ads[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ads[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	ads[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ads[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ads[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	ads[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	ads[1].flags = 0;
	ads[1].format = VK_FORMAT_D32_SFLOAT;
	ads[1].samples = VK_SAMPLE_COUNT_1_BIT;
	ads[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	ads[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ads[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	ads[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	ads[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	ads[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference ar = {};
	ar.attachment = 0;
	ar.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference arDepth = {};
	arDepth.attachment = 1;
	arDepth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription sd = {};
	sd.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	sd.colorAttachmentCount = 1;
	sd.pColorAttachments = &ar;
	sd.inputAttachmentCount = 0;
	sd.preserveAttachmentCount = 0;
	sd.pDepthStencilAttachment = &arDepth;

	VkSubpassDependency dep = {};
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;
	dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask = 0;
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	ci.flags = 0;
	ci.pNext = nullptr;
	ci.attachmentCount = ads.size();
	ci.pAttachments = ads.data();
	ci.subpassCount = 1;
	ci.pSubpasses = &sd;
	ci.dependencyCount = 1;
	ci.pDependencies = &dep;
	//ci.dependencyCount = 0;

	auto res = vkCreateRenderPass(vSys->getDevice("vdevice-main").getDevice(), &ci, nullptr, &rp);
	if (res != VK_SUCCESS)
		throw std::runtime_error("Failed to create render-pass!");

}

void stop_vulkan()
{
	vSys->getDevice("vdevice-main").wait();

	texture.reset();
	depthMap.reset();

	vkDestroyPipelineLayout(vSys->getDevice("vdevice-main").getDevice(), pipelineLayout, nullptr);
	vkDestroyShaderModule(vSys->getDevice("vdevice-main").getDevice(), vertShader, nullptr);
	vkDestroyShaderModule(vSys->getDevice("vdevice-main").getDevice(), fragShader, nullptr);

	vkDestroyPipeline(vSys->getDevice("vdevice-main").getDevice(), pipeline, nullptr);
	vkDestroyRenderPass(vSys->getDevice("vdevice-main").getDevice(), renderPass, nullptr);

	delete vertexBuffer;

	vkDestroySemaphore(vSys->getDevice("vdevice-main").getDevice(), renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(vSys->getDevice("vdevice-main").getDevice(), imageAvailableSemaphore, nullptr);

	vkDestroyFramebuffer(vSys->getDevice("vdevice-main").getDevice(), frameBuffers[0], nullptr);
	vkDestroyFramebuffer(vSys->getDevice("vdevice-main").getDevice(), frameBuffers[1], nullptr);
	vkDestroyFramebuffer(vSys->getDevice("vdevice-main").getDevice(), frameBuffers[2], nullptr);

	vkDestroyCommandPool(vSys->getDevice("vdevice-main").getDevice(), comPool, nullptr);

	delete vSys;
}

VkCommandBuffer beginOneTimeCmdBuffer(VkDevice dev, VkCommandPool pool)
{
	VkCommandBuffer cmdBuff;
	VkCommandBufferAllocateInfo ai;
	ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	ai.pNext = nullptr;
	ai.commandPool = pool;
	ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	ai.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(dev, &ai, &cmdBuff) != VK_SUCCESS)
		throw std::runtime_error("Failed to allocate command buffer for image-layout transition");

	VkCommandBufferBeginInfo bi;
	bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	bi.pNext = nullptr;
	bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	bi.pInheritanceInfo = nullptr;

	if (vkBeginCommandBuffer(cmdBuff, &bi) != VK_SUCCESS)
		throw std::runtime_error("Failed to begin command buffer for image-layout transition");
	return cmdBuff;
}

void endOneTimeCmdBuffer(VkCommandBuffer buff)
{
	if (vkEndCommandBuffer(buff) != VK_SUCCESS)
		throw std::runtime_error("Failed to end command buffer for image-layout transition");
}

void submitOneTimeCmdBuffer(VkDevice dev, VkQueue q, VkCommandPool pool, VkCommandBuffer buff)
{
	VkSubmitInfo si;
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.pNext = nullptr;
	si.commandBufferCount = 1;
	si.pCommandBuffers = &buff;
	si.signalSemaphoreCount = 0;
	si.waitSemaphoreCount = 0;
	si.pWaitDstStageMask = nullptr;

	if (vkQueueSubmit(q, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit command buffer for image-layout transition");

	if (vkQueueWaitIdle(q) != VK_SUCCESS)
		throw std::runtime_error("Failed to wait for command buffer execution");

	vkFreeCommandBuffers(dev, pool, 1, &buff);
}

void cmdTransitionImageLayout(VkCommandBuffer buff, VkImage img, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
	VkImageMemoryBarrier ibar;
	ibar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	ibar.pNext = nullptr;
	ibar.image = img;
	ibar.dstQueueFamilyIndex = ibar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	ibar.oldLayout = oldLayout;
	ibar.newLayout = newLayout;

	ibar.srcAccessMask = srcAccess;
	ibar.dstAccessMask = dstAccess;

	ibar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	ibar.subresourceRange.baseArrayLayer = 0;
	ibar.subresourceRange.layerCount = 1;
	ibar.subresourceRange.baseMipLevel = 0;
	ibar.subresourceRange.levelCount = 1;

	vkCmdPipelineBarrier(buff, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr, //mem
		0, nullptr, //buff
		1, &ibar	//img
	);
}

std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> initDepthTexture(size_t w, size_t h)
{
	VkFormat format = VK_FORMAT_D32_SFLOAT;

	std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> res = std::make_unique<zenith::vulkan::vTextureAutoImpl_>(const_cast<zenith::vulkan::vDeviceImpl_ *>(vSys->getDevice("vdevice-main").rawImpl()),
		zenith::vulkan::vTextureDimensions::SingleTextureWithoutMip(w, h), format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		zenith::vulkan::vMemoryUsage::GPU_STATIC, zenith::vulkan::vObjectSharingInfo(),
		VK_IMAGE_LAYOUT_UNDEFINED, true);

	VkCommandBuffer cmdBuff = beginOneTimeCmdBuffer(vSys->getDevice("vdevice-main").getDevice(), comPool);
	cmdTransitionImageLayout(cmdBuff, res->handle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

	endOneTimeCmdBuffer(cmdBuff);
	submitOneTimeCmdBuffer(vSys->getDevice("vdevice-main").getDevice(), vSys->getDevice("vdevice-main").getQueue("vqueue-graphics"), comPool, cmdBuff);

	vkDeviceWaitIdle(vSys->getDevice("vdevice-main").getDevice());
	return res;
}

std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> initTexture(const zenith::util::zfile_format::zImgDescription &img)
{

	VkFormat format = zenith::util::zfile_format::ImageFormat2Vulkan(img.imageFormat);
	size_t actNumChan = zenith::util::zfile_format::getNumChannels(img.imageFormat);
	size_t actChanSize = zenith::util::zfile_format::getChannelSize(img.imageFormat) >> 3;
	bool needFormatTransform = false;

	if (format == VK_FORMAT_UNDEFINED)
		throw std::runtime_error("initTexture: unsupported texture format!");

	std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> res(new zenith::vulkan::vTextureAutoImpl_(const_cast<zenith::vulkan::vDeviceImpl_ *>(vSys->getDevice("vdevice-main").rawImpl()),
		zenith::vulkan::vTextureDimensions::SingleTextureWithoutMip(img.width, img.height), format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		zenith::vulkan::vMemoryUsage::GPU_STATIC, zenith::vulkan::vObjectSharingInfo(),
		VK_IMAGE_LAYOUT_UNDEFINED, true));


	zenith::vulkan::vBufferAutoImpl_ stagingBuff(const_cast<zenith::vulkan::vDeviceImpl_ *>(vSys->getDevice("vdevice-main").rawImpl()),
		img.width * img.height * actNumChan * actChanSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, zenith::vulkan::vMemoryUsage::STAGING, zenith::vulkan::vObjectSharingInfo());

	uint8_t * dst = static_cast<uint8_t *>(stagingBuff.memoryBlock().map());
	if (!needFormatTransform)
		memcpy(dst, img.levelData[0], img.width * img.height * actNumChan * actChanSize);
	stagingBuff.memoryBlock().unmap();



	VkBufferImageCopy copyRegion;
	copyRegion.imageOffset.x = 0;
	copyRegion.imageOffset.y = 0;
	copyRegion.imageOffset.z = 0;
	copyRegion.imageExtent.width = res->getDimensions().width();
	copyRegion.imageExtent.height = res->getDimensions().height();
	copyRegion.imageExtent.depth = res->getDimensions().depth();
	copyRegion.imageSubresource = zenith::vulkan::vTextureSubresource::Full(res->getDimensions()).vkImageSubresourceLayers(VK_IMAGE_ASPECT_COLOR_BIT);
	copyRegion.bufferOffset = 0;
	copyRegion.bufferRowLength = img.width;
	copyRegion.bufferImageHeight = img.height;


	VkCommandBuffer cmdBuff = beginOneTimeCmdBuffer(vSys->getDevice("vdevice-main").getDevice(), comPool);
	cmdTransitionImageLayout(cmdBuff, res->handle(), format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT);

	vkCmdCopyBufferToImage(cmdBuff, stagingBuff.handle(), res->handle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	cmdTransitionImageLayout(cmdBuff, res->handle(), format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	endOneTimeCmdBuffer(cmdBuff);
	submitOneTimeCmdBuffer(vSys->getDevice("vdevice-main").getDevice(), vSys->getDevice("vdevice-main").getQueue("vqueue-graphics"), comPool, cmdBuff);

	vkDeviceWaitIdle(vSys->getDevice("vdevice-main").getDevice());

	return res;
}

void init_vulkan()
{

	//const char * shdrVert = "../../Resource/Shaders/Test12a/vert.spv";
	//const char * shdrFrag = "../../Resource/Shaders/Test12a/frag.spv";

	const size_t BUFF_SIZE_XML = 16384;
	const size_t BUFF_SIZE_SHDR = 4096;

	char buff1[BUFF_SIZE_XML], buff2[BUFF_SIZE_XML], buff3[BUFF_SIZE_SHDR], buff4[BUFF_SIZE_SHDR];

	auto resSHDR_v = readFile(fnameVShader.c_str(), (uint8_t *)buff3, BUFF_SIZE_SHDR);
	auto resSHDR_f = readFile(fnameFShader.c_str(), (uint8_t *)buff4, BUFF_SIZE_SHDR);

	auto shdr_code_v = resSHDR_v.get();
	auto shdr_code_f = resSHDR_f.get();

	createShaderModule((const char *)shdr_code_v.data, shdr_code_v.size, vertShader, shaderStages[0], VK_SHADER_STAGE_VERTEX_BIT);
	createShaderModule((const char *)shdr_code_f.data, shdr_code_f.size, fragShader, shaderStages[1], VK_SHADER_STAGE_FRAGMENT_BIT);

	createCommandPool(comPool, vSys->getDevice("vdevice-main").getQueueFamilyIndex("vqueue-graphics"));
	depthMap = initDepthTexture(wnd->getSize().width, wnd->getSize().height);

	VkPipelineInputAssemblyStateCreateInfo pias_ci;
	VkPipelineVertexInputStateCreateInfo pvis_ci;
	createInputAssembly(pvis_ci, pias_ci);

	auto bd = Vertex::getBindingDescription();
	auto ad = Vertex::getAttributeDescription();
	pvis_ci.vertexAttributeDescriptionCount = ad.size();
	pvis_ci.pVertexAttributeDescriptions = &ad[0];
	pvis_ci.vertexBindingDescriptionCount = 1;
	pvis_ci.pVertexBindingDescriptions = &bd;

	VkRect2D scissor;
	VkViewport viewport;
	VkPipelineViewportStateCreateInfo pvs_ci;
	createViewport(pvs_ci, viewport, scissor);

	VkPipelineRasterizationStateCreateInfo prs_ci;
	VkPipelineMultisampleStateCreateInfo pms_ci;
	createRasterization(prs_ci, pms_ci);

	VkPipelineColorBlendAttachmentState pcbas;
	VkPipelineColorBlendStateCreateInfo pcbs_ci;
	createColorBlend(pcbas, pcbs_ci);

	VkPipelineDepthStencilStateCreateInfo dss_ci;
	createDepthStencilState(dss_ci);


	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH };
	VkPipelineDynamicStateCreateInfo pds_ci;
	pds_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	pds_ci.flags = 0;
	pds_ci.pNext = nullptr;
	pds_ci.dynamicStateCount = 2;
	pds_ci.pDynamicStates = dynamicStates;

	createPipelineLayout(pipelineLayout);
	createRenderPass(renderPass);

	VkGraphicsPipelineCreateInfo pci = {};
	pci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pci.stageCount = 2;
	pci.pStages = shaderStages;
	pci.pVertexInputState = &pvis_ci;
	pci.pInputAssemblyState = &pias_ci;
	pci.pViewportState = &pvs_ci;
	pci.pRasterizationState = &prs_ci;
	pci.pMultisampleState = &pms_ci;
	pci.pDepthStencilState = &dss_ci;
	pci.pColorBlendState = &pcbs_ci;
	pci.pDynamicState = nullptr;

	pci.layout = pipelineLayout;
	pci.renderPass = renderPass;
	pci.subpass = 0;

	pci.basePipelineHandle = VK_NULL_HANDLE;
	pci.basePipelineIndex = -1;

	VkResult vkres = vkCreateGraphicsPipelines(vSys->getDevice("vdevice-main").getDevice(), VK_NULL_HANDLE, 1, &pci, nullptr, &pipeline);
	if (vkres != VK_SUCCESS)
		throw std::runtime_error("Failed to create pipeline!");

	ZLOG_REGULAR("Created pipeline. Creating command buffers and framebuffers.");
	createVertexBuffer();
	createIndexBuffer();
	createCommandBuffers(comPool, comBuffers, 3);
	for (size_t i = 0; i < 3; i++)
		createFramebuffer(vSys->getDevice("vdevice-main").getSwapchain("vswapchain-main").getImageView(i), frameBuffers[i]);
	ZLOG_REGULAR("Frambuffers and command buffers created.");

	createDescriptorPool();

	createSemaphore(imageAvailableSemaphore);
	createSemaphore(renderFinishedSemaphore);

	std::cout << "Init finished.\n";
}


void init_input()
{
	for (bool &v : keys)
		v = false;
}

void process_input_key_up(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam)
{
	keys[wparam] = false;
}
void process_input_key_down(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam)
{
	keys[wparam] = true;
}


void init_camera()
{
	mainCamera.pos = glm::vec3(0.0f, 0.0f, 0.0f);
	mainCamera.dir = glm::vec3(0.5f, 0.5f, 0.14f);
	mainCamera.up = glm::vec3(0.0f, 0.0f, 1.0f);
	mainCamera.updateProj();
	mainCamera.updateView();

	size_t sz = mainCamera.sizeViewProj();
	cameraUniformBuffer = new zenith::vulkan::vBufferAutoImpl_(const_cast<zenith::vulkan::vDeviceImpl_*>(vSys->getDevice("vdevice-main").rawImpl()),
		sz,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		zenith::vulkan::vMemoryUsage::STAGING,
		zenith::vulkan::vObjectSharingInfo());


	void * data = cameraUniformBuffer->memoryBlock().map();
	memcpy(data, mainCamera.dataViewProj(), sz);
	cameraUniformBuffer->memoryBlock().unmap();

	VkDescriptorSetLayout layouts[] = { descriptorSetLayout };
	VkDescriptorSetAllocateInfo ai = {};
	ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	ai.pNext = nullptr;
	ai.descriptorPool = descrPool;
	ai.descriptorSetCount = 1;
	ai.pSetLayouts = layouts;

	if (vkAllocateDescriptorSets(vSys->getDevice("vdevice-main").getDevice(), &ai, &descrSet) != VK_SUCCESS)
		throw std::runtime_error("Failed to create descriptor pool!");

	VkDescriptorBufferInfo bi = {};
	bi.buffer = cameraUniformBuffer->handle();
	bi.offset = 0;
	bi.range = mainCamera.sizeViewProj();

	VkDescriptorImageInfo imageInfo = {};
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView = texture->mainView().handle();
	imageInfo.sampler = sampler->handle();

	VkWriteDescriptorSet wi = {};
	wi.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wi.pNext = nullptr;
	wi.descriptorCount = 1;
	wi.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	wi.pBufferInfo = &bi;
	wi.pImageInfo = nullptr;
	wi.pTexelBufferView = nullptr;

	wi.dstArrayElement = 0;
	wi.dstBinding = 0;
	wi.dstSet = descrSet;

	VkWriteDescriptorSet wi2 = {};
	wi2.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	wi2.pNext = nullptr;
	wi2.descriptorCount = 1;
	wi2.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	wi2.pBufferInfo = nullptr;
	wi2.pImageInfo = &imageInfo;
	wi2.pTexelBufferView = nullptr;

	wi2.dstArrayElement = 0;
	wi2.dstBinding = 1;
	wi2.dstSet = descrSet;

	VkWriteDescriptorSet wis[] = { wi, wi2 };


	vkUpdateDescriptorSets(vSys->getDevice("vdevice-main").getDevice(), 2, &wis[0], 0, nullptr);
}


void update_camera()
{
	static bool enableMouse = true;

	static std::chrono::system_clock::time_point tpPrev = std::chrono::system_clock::now(), tpNow = std::chrono::system_clock::now();
	tpNow = std::chrono::system_clock::now();
	std::chrono::duration<double> dur = (tpNow - tpPrev);
	double dTime = dur.count();
	tpPrev = tpNow;

	if (dTime > 1.0f)
		dTime = 0.1f;


	if (keys[0x57])//W
		mainCamera.pos += mainCamera.dir * float(dTime) * 10.0f;
	if (keys[0x41])//A
		mainCamera.pos += glm::cross(mainCamera.dir, mainCamera.up) * float(dTime) * 25.0f;
	if (keys[0x53])//S
		mainCamera.pos -= mainCamera.dir * float(dTime) * 10.0f;
	if (keys[0x44])//D
		mainCamera.pos -= glm::cross(mainCamera.dir, mainCamera.up) * float(dTime) * 25.0f;

	if (keys[VK_F1])
		enableMouse = true;
	if (keys[VK_F2])
		enableMouse = false;

	if (enableMouse)
	{
		POINT p;
		GetCursorPos(&p);
		static double a = 0.0, b = 0.0;
		static int prevX = 0, prevY = 0;
		int xPos = p.x - 640;
		int yPos = p.y - 480;
		prevX = p.x;//LOWORD(lParam);
		prevY = p.y;//HIWORD(lParam);
		a += double(xPos) * 0.1;// * dTime * 50;
		b -= double(yPos) * 0.1;// * dTime * 50;
		while (a > 360.0)
			a -= 360.0;
		while (a < 0.0)
			a += 360.0;
		while (b < -90.0)
			b += 180.0;
		while (b > 90.0)
			b -= 180.0;
		SetCursorPos(640, 480);
		double y_ = glm::sin(glm::radians(a)) * glm::cos(-glm::radians(b));
		double x_ = glm::cos(glm::radians(a)) * glm::cos(-glm::radians(b));
		double z_ = glm::sin(-glm::radians(b));
		mainCamera.dir = glm::vec4(x_, y_, z_, 0.0f);
	}

	mainCamera.updateView();

	size_t sz = mainCamera.sizeViewProj();

	void * data = cameraUniformBuffer->memoryBlock().map();
	memcpy(data, mainCamera.dataViewProj(), sz);
	cameraUniformBuffer->memoryBlock().unmap();
}
