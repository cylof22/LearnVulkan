#include "pch.h"
#include "VulkanGraphicContext.h"
#include "VulkanRenderWindow.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VulkanRenderWindow::VulkanRenderWindow(ANativeWindow* pWnd, VulkanGraphicContext* graphicCotext) 
	: m_pWnd(pWnd)
{

}
#elif VK_USE_PLATFORM_WIN32_KHR
VVulkanRenderWindow::ulkanRenderWindow(HWND pWnd, VulkanGraphicContext* graphicCotext)
{

}
#endif

VulkanRenderWindow::~VulkanRenderWindow()
{
}

uint32_t VulkanRenderWindow::getWidth() const
{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	return ANativeWindow_getWidth(m_pWnd);
#elif VK_USE_PLATFORM_WIN32_KHR
	return 0;
#endif
}

uint32_t VulkanRenderWindow::getHeight() const
{
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	return ANativeWindow_getHeight(m_pWnd);
#elif VK_USE_PLATFORM_WIN32_KHR
	return 0;
#endif
}

std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> VulkanRenderWindow::getColorBits() const
{
	return std::make_tuple(m_displayAttribs.redBits, m_displayAttribs.blueBits, m_displayAttribs.greenBits, m_displayAttribs.alphaBits);
}

std::pair<uint32_t, uint32_t> VulkanRenderWindow::getDepthStencilBits() const
{
	return std::pair<uint32_t, uint32_t>(m_displayAttribs.depthBPP, m_displayAttribs.stencilBPP);
}

VsyncMode VulkanRenderWindow::getSyncMode() const
{
	return m_displayAttribs.vsyncMode;
}