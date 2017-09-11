#pragma once
#include <string>
#include <vector>
#include <tuple>
#include "common\vulkan_wrapper.h"

enum class VsyncMode {
	Off,//!<The application does not synchronizes with the vertical sync. If application renders faster than the display refreshes, frames are wasted and tearing may be observed. FPS is uncapped. Maximum power consumption. If unsupported, "ON" value will be used instead. Minimum latency.
	On,//!<The application is always syncrhonized with the vertical sync. Tearing does not happen. FPS is capped to the display's refresh rate. For fast applications, battery life is improved. Always supported.
	Relaxed,//!<The application synchronizes with the vertical sync, but only if the application rendering speed is greater than refresh rate. Compared to OFF, there is no tearing. Compared to ON, the FPS will be improved for "slower" applications. If unsupported, "ON" value will be used instead. Recommended for most applications. Default if supported.
	Mailbox, //!<The presentation engine will always use the latest fully rendered image. Compared to OFF, no tearing will be observed. Compared to ON, battery power will be worse, especially for faster applications. If unsupported,  "OFF" will be attempted next.
	Half, //!<The application is capped to using half the vertical sync time. FPS artificially capped to Half the display speed (usually 30fps) to maintain battery. Best possible battery savings. Worst possibly performance. Recommended for specific applications where battery saving is critical.
};

struct DisplayAttributes
{
	std::string  windowTitle;

	uint32_t width;
	uint32_t height;
	uint32_t x;
	uint32_t y;

	uint32_t depthBPP;
	uint32_t stencilBPP;

	uint32_t redBits;
	uint32_t greenBits;
	uint32_t blueBits;
	uint32_t alphaBits;

	uint32_t aaSamples;

	VsyncMode vsyncMode;

	bool fullscreen;
	bool forceColorBPP;
	uint32_t swapLength;
};

struct ANativeWindow;
class VulkanGraphicContext;
class VulkanFrameBuffer;

class VulkanRenderWindow
{
public:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	VulkanRenderWindow(ANativeWindow* pWnd, VulkanGraphicContext* graphicCotext);
#elif VK_USE_PLATFORM_WIN32_KHR
	VulkanRenderWindow(HWND pWnd, VulkanGraphicContext* graphicContext)
#endif
	~VulkanRenderWindow();

	uint32_t getWidth() const;
	uint32_t getHeight() const;

	std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> getColorBits() const;
	std::pair<uint32_t, uint32_t> getDepthStencilBits() const;
	VsyncMode getSyncMode() const;

	VulkanFrameBuffer* getScreenFbo() { return m_onScreenFbo; }

private:
	DisplayAttributes m_displayAttribs;

	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapChain;

	VulkanFrameBuffer* m_onScreenFbo;

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	ANativeWindow* m_pWnd;
#elif
	HWND m_pWnd;
#endif
};

