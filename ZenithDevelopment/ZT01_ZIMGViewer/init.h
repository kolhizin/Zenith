#pragma once

#include <glm\glm.hpp>
#include <Graphics\Vulkan\vSystem.h>
#include <Graphics\Vulkan\vBuffer.h>
#include <Graphics\Vulkan\vTexture.h>
#include <Graphics\Vulkan\vSampler.h>
#include <Utils\log_config.h>
#include <Utils\xml_tools.h>
#include <Utils\nameid.h>
#include <Utils\window.h>
#include <Utils\IO\filesystem.h>
#include <Utils\Manager\manager_unique.h>
#include <Utils/FileFormat/ff_img.h>
#include <chrono>
#include <thread>
#include <string>
#include "SimpleCamera.h"

extern std::string fnameVShader, fnameFShader;

extern std::unique_ptr<zenith::util::Window> wnd;
extern zenith::vulkan::vSystem * vSys;
extern zenith::util::io::FileSystem * fs;

extern VkShaderModule vertShader, fragShader;
extern VkPipelineShaderStageCreateInfo shaderStages[2];
extern VkPipelineLayout pipelineLayout;
extern VkRenderPass renderPass;
extern VkPipeline pipeline;
extern VkFramebuffer frameBuffers[3];
extern VkCommandPool comPool;
extern VkCommandBuffer comBuffers[3];
extern VkSemaphore imageAvailableSemaphore, renderFinishedSemaphore;

extern zenith::vulkan::vBufferAutoImpl_ * vertexBuffer, *indexBuffer, *cameraUniformBuffer;

extern zenith::vulkan::vTextureAutoImpl_ * texture;
extern zenith::vulkan::vSamplerImpl_ * sampler;

extern SCamera mainCamera;
extern std::array<bool, 256> keys;
extern VkDescriptorPool descrPool;
extern VkDescriptorSet descrSet;




struct Vertex
{
	glm::vec2 pos;
	glm::vec2 texcoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bd = {};
		bd.binding = 0;
		bd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bd.stride = sizeof(Vertex);
		return bd;
	}
	static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 2> res;
		res[0].binding = 0;
		res[0].location = 0;
		res[0].offset = offsetof(Vertex, pos);
		res[0].format = VK_FORMAT_R32G32_SFLOAT;

		res[1].binding = 0;
		res[1].location = 1;
		res[1].offset = offsetof(Vertex, texcoord);
		res[1].format = VK_FORMAT_R32G32_SFLOAT;
		return res;
	}
};

extern std::vector<Vertex> vertices;

class ImageData
{
	int w_, h_, ch_;
	uint8_t * data_;

	ImageData(const ImageData &) = delete;
	ImageData &operator =(const ImageData &) = delete;
	void clear_();
public:
	inline ImageData() : data_(nullptr), w_(0), h_(0), ch_(0) {}
	inline ImageData(ImageData &&d) : data_(d.data_), w_(d.w_), h_(d.h_), ch_(d.ch_)
	{
		d.data_ = nullptr;
		d.w_ = d.h_ = d.ch_ = 0;
	}
	inline ImageData &operator =(ImageData &&d)
	{
		clear_();
		data_ = d.data_;
		w_ = d.w_;
		h_ = d.h_;
		ch_ = d.ch_;
		d.data_ = nullptr;
		d.w_ = d.h_ = d.ch_ = 0;
		return *this;
	}
	inline ~ImageData() { clear_(); }

	inline void * data() const { return data_; }
	inline size_t width() const { return w_; }
	inline size_t height() const { return h_; }
	inline size_t channels() const { return ch_; }

	static ImageData loadPNG(void * data, size_t sz);
};


inline std::future<zenith::util::io::FileResult> readFile(const char * fname, uint8_t * data, size_t maxSize, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'r', priority);
	auto r = f.read(data, maxSize, priority);
	f.close();
	return r;
}

inline std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>> readFile(const char * fname, float priority = 1.0f)
{
	auto f = fs->createFile();
	f.open(fname, 'r', priority);
	size_t sz = f.size_left(priority).get().size;
	unsigned char * data = new unsigned char[sz];
	std::unique_ptr<unsigned char[]> h(data);
	auto r = f.read(data, sz, priority);
	f.close();
	return std::pair<std::future<zenith::util::io::FileResult>, std::unique_ptr<unsigned char[]>>(std::move(r), std::move(h));
}


uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
void createSemaphore(VkSemaphore &s);
void createCommandBuffers(VkCommandPool &cp, VkCommandBuffer * cbs, size_t numBuff, uint32_t qFamilyIndex);
void createDescriptorPool();
void createVertexBuffer();
void createIndexBuffer();
void writeCommandBuffer(VkCommandBuffer &cb, VkFramebuffer &fb);
void createFramebuffer(VkImageView v, VkFramebuffer &fb);
void createShaderModule(const char * code, size_t codeSize, VkShaderModule &shMod, VkPipelineShaderStageCreateInfo &sci, VkShaderStageFlagBits shaderStage);
void createInputAssembly(VkPipelineVertexInputStateCreateInfo &vis, VkPipelineInputAssemblyStateCreateInfo &ias);
void createViewport(VkPipelineViewportStateCreateInfo &pvs, VkViewport &viewport, VkRect2D &scissor);
void createRasterization(VkPipelineRasterizationStateCreateInfo &prs, VkPipelineMultisampleStateCreateInfo &pms);
void createPipelineLayout(VkPipelineLayout &pl);
void createColorBlend(VkPipelineColorBlendAttachmentState &pcbas, VkPipelineColorBlendStateCreateInfo &pcbs_ci);
void createRenderPass(VkRenderPass &rp);
void init_vulkan();
void stop_vulkan();

VkCommandBuffer beginOneTimeCmdBuffer(VkDevice dev, VkCommandPool pool);
void endOneTimeCmdBuffer(VkCommandBuffer buff);
void submitOneTimeCmdBuffer(VkDevice dev, VkQueue q, VkCommandPool pool, VkCommandBuffer buff);

void cmdTransitionImageLayout(VkCommandBuffer buff, VkImage img, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkAccessFlags srcAccess, VkAccessFlags dstAccess);

//void initTexture(void * data, uint32_t w, uint32_t h, uint32_t channels);
void initTexture(const zenith::util::zfile_format::zImgDescription &img);

void init_descriptors();
void init_input();


void process_input_key_up(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam);
void process_input_key_down(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam);

