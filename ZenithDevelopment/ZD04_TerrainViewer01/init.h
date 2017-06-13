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


class SCamera
{
	glm::mat4x4 view_, proj_;
public:
	glm::vec3 pos, dir, up;
	float fovAngle = 90.0f, aspectRatio = 4.0f / 3.0f, nearClip = 1e-1f, farClip = 1e3f;

	inline const glm::mat4x4 &getView() const { return view_; }
	inline const glm::mat4x4 &getProj() const { return proj_; }

	inline void * dataViewProj() { return &view_; }
	inline size_t sizeViewProj() const { return sizeof(view_) + sizeof(proj_); }

	inline void updateView()
	{
		glm::vec3 x, y, z;
		z = dir;
		x = glm::cross(up, z);
		y = glm::normalize(glm::cross(z, x));
		x = glm::normalize(x);
		z = glm::normalize(z);

		float px = -glm::dot(pos, x);
		float py = -glm::dot(pos, y);
		float pz = -glm::dot(pos, z);


		view_ = glm::mat4x4(x.x, y.x, z.x, 0.0f,
			x.y, y.y, z.y, 0.0f,
			x.z, y.z, z.z, 0.0f,
			px, py, pz, 1.0f);

		//view_ = glm::mat4x4(1.0f, 0.0f, 0.0f, 0.0f,		0.0f, 1.0f, 0.0f, 0.0f,		0.0f, 0.0f, 1.0f, 0.0f,  	0.0f, 0.0f, 0.0f, 1.0f);
	}
	inline void updateProj()
	{
		float h = 1.0f / glm::tan(glm::radians(fovAngle) * 0.5f);
		float w = h / aspectRatio;
		float zn = nearClip;
		float zf = farClip;
		float q = zf / (zf - zn);
		proj_ = glm::mat4x4(w, 0.0f, 0.0f, 0.0f,
			0.0f, h, 0.0f, 0.0f,
			0.0f, 0.0f, q, 1.0f,
			0.0f, 0.0f, -q*zn, 0.0f);
	}
};

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

extern std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> texture, depthMap;
extern zenith::vulkan::vSamplerImpl_ * sampler;

extern SCamera mainCamera;
extern std::array<bool, 256> keys;
extern VkDescriptorPool descrPool;
extern VkDescriptorSet descrSet;




struct Vertex
{
	glm::vec4 pos;
	glm::vec3 norm;
	glm::vec2 texcoord;

	static VkVertexInputBindingDescription getBindingDescription()
	{
		VkVertexInputBindingDescription bd = {};
		bd.binding = 0;
		bd.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		bd.stride = sizeof(Vertex);
		return bd;
	}
	static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescription()
	{
		std::array<VkVertexInputAttributeDescription, 3> res;
		res[0].binding = 0;
		res[0].location = 0;
		res[0].offset = offsetof(Vertex, pos);
		res[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;

		res[1].binding = 0;
		res[1].location = 1;
		res[1].offset = offsetof(Vertex, norm);
		res[1].format = VK_FORMAT_R32G32B32_SFLOAT;

		res[2].binding = 0;
		res[2].location = 2;
		res[2].offset = offsetof(Vertex, texcoord);
		res[2].format = VK_FORMAT_R32G32_SFLOAT;
		return res;
	}
};

extern std::vector<Vertex> vertices;
extern std::vector<uint32_t> indices;

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
void createCommandPool(VkCommandPool & cp, uint32_t qFamilyIndex);
void createCommandBuffers(VkCommandPool &cp, VkCommandBuffer * cbs, size_t numBuff);
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

std::unique_ptr<zenith::vulkan::vTextureAutoImpl_> initTexture(const zenith::util::zfile_format::zImgDescription &img);

void init_input();


void process_input_key_up(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam);
void process_input_key_down(zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam);


void init_camera();
void update_camera();