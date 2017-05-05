#include <iostream>
#include "init.h"
#include <sstream>
#include "Graphics\Vulkan\vMemoryManager.h"


void init()
{
	const char * windowConf = "../../Resource/Window/WindowConfig.xml";
	//const char * vulkanConf = "../../Resource/Vulkan/VulkanConfig.xml";
	const char * vulkanConf = "ZIMGViewer_Config.xml";
	const char * texFilename = "../../Resource/Texture/heightmap.zimg";
	//const char * texFilename = "../Test12e_TerrainGen1/initialGrid.zimg";
	const size_t BUFF_SIZE_XML = 16384;
	char buff1[BUFF_SIZE_XML], buff2[BUFF_SIZE_XML];

	zenith::util::zLOG::resetOutput("main.log");

	ZLOG_REGULAR("init: creating FileSystem.");
	fs = new zenith::util::io::FileSystem(3);
	ZLOG_REGULAR("init: FileSystem created.");


	pugi::xml_document docV, docW;
	auto resXML_v = readFile(vulkanConf, (uint8_t *)buff1, BUFF_SIZE_XML);
	auto resXML_w = readFile(windowConf, (uint8_t *)buff2, BUFF_SIZE_XML);
	auto resIMG0 = readFile(texFilename);

	auto resXMLd_w = resXML_w.get();
	docW.load_buffer_inplace(resXMLd_w.data, resXMLd_w.size);

	zenith::util::WndConfig wndConf;

	{
		zenith::util::ObjectMap<char, char> omW;
		zenith::util::xml::xml2objmap(docW.root().child("Window"), omW);
		zenith::util::from_objmap(wndConf, omW);
	}


	auto resXMLd_v = resXML_v.get();
	docV.load_buffer_inplace(resXMLd_v.data, resXMLd_v.size);

	zenith::vulkan::vSystemConfig vConf;

	{
		zenith::util::ObjectMap<char, char> omV;

		zenith::util::xml::xml2objmap(docV.root().child("VulkanSystem"), omV);
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

	init_vulkan();

	auto resIMG0r = resIMG0.first.get();
	auto imgData = zenith::util::zfile_format::zimg_from_mem(resIMG0r.data, resIMG0r.size);
	//auto texImageData = ImageData::loadPNG(resPNG0r.data, resPNG0r.size);

	initTexture(imgData);
	//initTexture(texImageData.data(), texImageData.width(), texImageData.height(), texImageData.channels());

	init_descriptors();

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
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	/*
	zenith::util::memory::MemAllocInfo highLevel;
	zenith::util::memory::MemAllocInfo lowLevelSmall, lowLevelLarge;

	highLevel.allocMinSize = highLevel.allocMaxSize = 128;
	highLevel.left = &lowLevelSmall;
	highLevel.right = &lowLevelLarge;

	lowLevelSmall.allocNumBlock = 1024;
	lowLevelSmall.allocMinSize = 16;
	lowLevelSmall.poolAlign = 16;
	lowLevelSmall.poolSize = 1024 * 16;
	lowLevelSmall.poolOffset = nullptr;

	lowLevelLarge.allocNumBlock = 1024;
	lowLevelLarge.allocMinSize = 256;
	lowLevelLarge.poolAlign = 256;
	lowLevelLarge.poolSize = 1024 * 256;
	lowLevelLarge.poolOffset = reinterpret_cast<void *>(lowLevelSmall.poolSize);

	zenith::vulkan::MainAllocator alloc(&highLevel);
	*/

	/*
	zenith::util::memory::MemAllocInfo listMAI;
	listMAI.allocMinSize = 128; listMAI.allocMaxSize = 16384;
	listMAI.poolAlign = 16; listMAI.poolSize = 1024 * 1024; listMAI.poolOffset = nullptr;
	zenith::vulkan::MainAllocator alloc(&listMAI);

	auto blk1 = alloc.allocate(986);
	auto blk2 = alloc.allocate(986);
	auto blk3 = alloc.allocate(57);
	alloc.deallocate(blk2);
	auto blk4 = alloc.allocate(854);
	alloc.deallocate(blk3);
	*/



	init();

	wnd->loop([]() {mainLoop();});

	deinit();

	std::cout << "Resources freed.\n";
	std::cout << "Closing console in 0.5 second.\n";

	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	return 0;
}