#include <iostream>
#include "init.h"
#include <sstream>
#include "Graphics\Vulkan\vMemoryManager.h"

void init(const std::string &moduleDir)
{
	const char * appConf = "AppConfig.xml";
	std::string appConfFilename = moduleDir + appConf;
	//const char * texFilename = "../Test12e_TerrainGen1/initialGrid.zimg";
	const size_t BUFF_SIZE_XML = 16384;
	char buff1[BUFF_SIZE_XML], buff2[BUFF_SIZE_XML];

	std::string fnameVShader, fnameFShader, fnameTexture;

	ZLOG_REGULAR("init: creating FileSystem.");
	fs = new zenith::util::io::FileSystem(3);
	ZLOG_REGULAR("init: FileSystem created.");

	pugi::xml_document docA;
	auto resXML = readFile(appConfFilename.c_str(), (uint8_t *)buff1, BUFF_SIZE_XML);
	
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

	auto xSettings = docA.root().child("AppConfig").child("Settings");
	auto xWindow = docA.root().child("AppConfig").child("Window");
	auto xVulkan = docA.root().child("AppConfig").child("VulkanSystem");

	auto xSettingsLog = xSettings.child("log");
	auto xSettingsShaders = xSettings.child("shaders");
	auto xSettingsTexture = xSettings.child("texture");

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

	if (xSettingsTexture.empty() || xSettingsTexture.attribute("fname").empty())
	{
		ZLOG_FATAL("Texture location not present in config file!");
		throw std::runtime_error("Failed to find texture location in config file!");
	}
	else
	{
		fnameTexture = xSettingsTexture.attribute("fname").as_string();
		if (fnameTexture.find('\\') == std::string::npos)
			fnameTexture = moduleDir + fnameTexture;
	}
	auto resIMG0 = readFile(fnameTexture.c_str());



	zenith::util::WndConfig wndConf;

	{
		zenith::util::ObjectMap<char, char> omW;
		zenith::util::xml::xml2objmap(xWindow, omW);
		zenith::util::from_objmap(wndConf, omW);
	}


	zenith::vulkan::vSystemConfig vConf;

	{
		zenith::util::ObjectMap<char, char> omV;

		zenith::util::xml::xml2objmap(xVulkan, omV);
		zenith::vulkan::from_objmap(vConf, omV);
	}



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

	init_vulkan_app(fnameVShader.c_str(), fnameFShader.c_str());

	auto resIMG0r = resIMG0.first.get();
	auto imgData = zenith::util::zfile_format::zimg_from_mem(resIMG0r.data, resIMG0r.size);

	init_texture(imgData);

	init_camera();

	for (size_t i = 0; i < 3; i++)
		writeCommandBuffer(comBuffers[i], frameBuffers[i]);

}


void deinit()
{
	//delete vSys;
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
int main()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string moduleDir = "";//buffer;
	//moduleDir.erase(moduleDir.find_last_of('\\') + 1, moduleDir.size());

	init(moduleDir);

	wnd->loop([]() {mainLoop();});

	deinit();

	std::cout << "Resources freed.\n";
	std::cout << "Closing console in 0.5 second.\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 0;
}