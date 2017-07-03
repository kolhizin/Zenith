#include <iostream>
#include "init.h"
#include <sstream>
#include "Graphics\Vulkan\vMemoryManager.h"

std::string texFilename, moduleDir;

float getHeight_R8(const zenith::util::zfile_format::zImgDescription &d, uint32_t i, uint32_t j)
{
	return float(*(static_cast<const uint8_t *>(d.levelData[0]) + d.levelDescr[0]->getRowPitch() * i + j)) / 255.0f;
}

float getHeight_R32F(const zenith::util::zfile_format::zImgDescription &d, uint32_t i, uint32_t j)
{
	return *reinterpret_cast<const float *>(static_cast<const uint8_t *>(d.levelData[0]) + d.levelDescr[0]->getRowPitch() * i + sizeof(float)*j);
}

inline glm::vec3 getNormal(const glm::vec4 &p1, const glm::vec4 &p2, const glm::vec4 &p3)
{
	glm::vec3 v1 = glm::vec3(p2) - glm::vec3(p1);
	glm::vec3 v2 = glm::vec3(p3) - glm::vec3(p1);
	return glm::normalize(glm::cross(v1, v2));
}
void initTerrain(const zenith::util::zfile_format::zImgDescription &d, std::vector<Vertex> &va, std::vector<uint32_t> &ia)
{
	const float xyScale = 10.0f, zScale = 10.0f;
	uint32_t w = d.width;
	uint32_t h = d.height;
	va.clear(); va.reserve(w * h);
	ia.clear(); ia.reserve(w * h * 2);

	auto pfnHeight = getHeight_R8;
	if(d.imageFormat == zenith::util::zfile_format::ImageFormat::R32F)
		pfnHeight = getHeight_R32F;

	float iw = 1.0f / float(w - 1);
	float ih = 1.0f / float(h - 1);

	for(uint32_t i = 0; i < h; i++)
		for (uint32_t j = 0; j < w; j++)
		{
			float h = pfnHeight(d, i, j);
			float x = float(j) * iw;
			float y = float(i) * ih;

			Vertex v;
			v.texcoord = glm::vec2(x, y);
			v.pos = glm::vec4(x * xyScale, y * xyScale, h * zScale, 1.0f);
			v.norm = glm::vec3(0.0f, 0.0f, 0.0f);
			va.push_back(v);
		}

	for (uint32_t i = 0; i < h - 1; i++)
	{
		for (uint32_t j = 0; j < w; j++)
		{
			ia.push_back(i*w+j);
			ia.push_back((i+1)*w + j);

			if (j + 1 < w)
			{
				glm::vec3 n1 = getNormal(va[i*w + j].pos, va[(i + 1)*w + j].pos, va[i*w + j + 1].pos);
				glm::vec3 n2 = getNormal(va[i*w + j + 1].pos, va[(i + 1)*w + j].pos, va[(i + 1)*w + j + 1].pos);

				va[i*w + j].norm += n1;
				va[(i + 1)*w + j].norm += n1;
				va[i*w + j + 1].norm += n1;

				va[(i + 1)*w + j + 1].norm += n2;
				va[(i + 1)*w + j].norm += n2;
				va[i*w + j + 1].norm += n2;
			}
		}
		if(i+2 < h)
			ia.push_back(0xFFFFFFFF); //primitive restart
	}
	for (auto i = 0; i < va.size(); i++)
		va[i].norm = glm::normalize(va[i].norm);
}

void init()
{
	const char * appConf = "TerrainViewer01_Config.xml";
	std::string appConfFilename = moduleDir + appConf;
	//const char * texFilename = "../Test12e_TerrainGen1/initialGrid.zimg";
	const size_t BUFF_SIZE_XML = 16384;
	char buff1[BUFF_SIZE_XML], buff2[BUFF_SIZE_XML];

	ZLOG_REGULAR("init: creating FileSystem.");
	fs = new zenith::util::io::FileSystem(3);
	ZLOG_REGULAR("init: FileSystem created.");

	pugi::xml_document docA;

	auto resXML = readFile(appConfFilename.c_str(), (uint8_t *)buff1, BUFF_SIZE_XML);
	auto resIMG0 = readFile(texFilename.c_str());

	zenith::util::io::FileResult resXMLd;
	try {
		resXMLd = resXML.get();
	}
	catch (...)
	{
		ZLOG_FATAL("Application config not found.");
		terminate();
	}


	docA.load_buffer_inplace(resXMLd.data, resXMLd.size);

	auto xSettings = docA.root().child("TerrainViewer01").child("Settings");
	auto xWindow = docA.root().child("TerrainViewer01").child("Window");
	auto xVulkan = docA.root().child("TerrainViewer01").child("VulkanSystem");

	auto xSettingsLog = xSettings.child("log");
	auto xSettingsShaders = xSettings.child("shaders");

	if (!xSettingsLog.empty())
	{
		if (!xSettingsLog.attribute("enable").empty() && !xSettingsLog.attribute("enable").as_bool())
			zenith::util::zLOG::close();
		else
		{
			if (!xSettingsLog.attribute("filename").empty())
			{
				std::string fname = xSettingsLog.attribute("filename").as_string();
				if (fname.find('\\') == std::string::npos)
					fname = moduleDir + fname;
				zenith::util::zLOG::resetOutput(fname.c_str());
			}
		}
	}
	if (xSettingsShaders.empty() || xSettingsShaders.attribute("fragment").empty() || xSettingsShaders.attribute("vertex").empty())
	{
		ZLOG_FATAL("Shader location not present in config file!");
		throw std::runtime_error("Failed to find shader location in config file!");
	}
	else
	{
		fnameVShader = xSettingsShaders.attribute("vertex").as_string();
		fnameFShader = xSettingsShaders.attribute("fragment").as_string();
		if (fnameVShader.find('\\') == std::string::npos)
			fnameVShader = moduleDir + fnameVShader;
		if (fnameFShader.find('\\') == std::string::npos)
			fnameFShader = moduleDir + fnameFShader;
	}



	zenith::util::WndConfig wndConf;
	zenith::util::ioconv::input(wndConf, zenith::util::ioconv::input_xml_root(xWindow));

	zenith::vulkan::vSystemConfig vConf;
	zenith::util::ioconv::input(vConf, zenith::util::ioconv::input_xml_root(xVulkan));

	wnd = std::make_unique<zenith::util::Window>(wndConf);


	wnd->addHandler(WM_CLOSE, "closeHandler", zenith::util::makeWindowMessageHandler([](zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam)
	{stop_vulkan(); wnd->destroy();}
	));


	wnd->addHandler(WM_KEYDOWN, "escapeHandler", zenith::util::makeWindowMessageHandler([](zenith::util::Window * wnd, WPARAM wparam, LPARAM lparam)
	{if (wparam == VK_ESCAPE) { stop_vulkan(); wnd->destroy(); }
	else if (wparam == VK_RETURN) SetWindowPos(wnd->hwnd(), 0, 10, 10, 300, 300, 0);}
	));

	wnd->addHandler(WM_KEYDOWN, "keyDownHandler", zenith::util::makeWindowMessageHandler(&process_input_key_down));
	wnd->addHandler(WM_KEYUP, "keyUpHandler", zenith::util::makeWindowMessageHandler(&process_input_key_up));

	zenith::vulkan::vSystemWindowInfo wndInfo;
	wndInfo.pWnd = wnd.get();
	wndInfo.pfnGetWndSize = zenith::util::Window::getWndSize;
	wndInfo.pUserData = nullptr;
	wndInfo.uid = "wnd-main";


	vSys = new zenith::vulkan::vSystem(vConf, wndInfo);

	zenith::vulkan::vObjectSharingInfo objsi{ false };


	auto resIMG0r = resIMG0.first.get();
	auto imgData = zenith::util::zfile_format::zimg_from_mem(resIMG0r.data, resIMG0r.size);
	initTerrain(imgData, vertices, indices);

	init_vulkan();


	texture = initTexture(imgData);
	sampler = new zenith::vulkan::vSamplerImpl_(const_cast<zenith::vulkan::vDeviceImpl_ *>(vSys->getDevice("vdevice-main").rawImpl()),
		zenith::vulkan::vSamplerFiltering(), zenith::vulkan::vSamplerAddressing(), zenith::vulkan::vSamplerLOD());
	//initTexture(texImageData.data(), texImageData.width(), texImageData.height(), texImageData.channels());

	//init_descriptors();
	init_camera();

	for (size_t i = 0; i < 3; i++)
		writeCommandBuffer(comBuffers[i], frameBuffers[i]);

}


void deinit()
{
	wnd.release();
	delete fs;
}
void mainLoop()
{
	update_camera();

	uint32_t imgInd = vSys->getDevice("vdevice-main").getSwapchain("vswapchain-main").acquireNextImage(imageAvailableSemaphore, VK_NULL_HANDLE);


	VkSubmitInfo si = {};
	si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	si.pNext = nullptr;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
	VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	si.waitSemaphoreCount = 1; si.pWaitSemaphores = waitSemaphores;
	si.pWaitDstStageMask = waitStages;

	si.commandBufferCount = 1;
	si.pCommandBuffers = &comBuffers[imgInd];

	si.signalSemaphoreCount = 1;
	si.pSignalSemaphores = signalSemaphores;


	if (vkQueueSubmit(vSys->getDevice("vdevice-main").getQueue("vqueue-graphics"), 1, &si, VK_NULL_HANDLE) != VK_SUCCESS)
		throw std::runtime_error("Failed to submit queue!");

	vSys->getDevice("vdevice-main").getSwapchain("vswapchain-main").queuePresent(vSys->getDevice("vdevice-main").getQueue("vqueue-present"), imgInd, renderFinishedSemaphore);
}

void init_args(LPSTR lpCmdLine)
{
	/*
	texFilename = lpCmdLine;
	if (texFilename.size() > 0)
	{
		if (texFilename.front() == '\"')
			texFilename.erase(0, 1);
		if (texFilename.back() == '\"')
			texFilename.erase(texFilename.size() - 1, 1);
	}
	

	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	moduleDir = buffer;
	moduleDir.erase(moduleDir.find_last_of('\\') + 1, moduleDir.size());
	*/
	texFilename = "initialGrid.zimg";
	moduleDir = "";
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	init_args(lpCmdLine);
	init();

	wnd->loop([]() {mainLoop();});

	deinit();

	std::cout << "Resources freed.\n";
	std::cout << "Closing console in 0.5 second.\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 0;
}