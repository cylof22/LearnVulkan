#pragma once
#include <string>
#include "common\vulkan_wrapper.h"

class VulkanRenderWindow;

constexpr uint32_t MaxSwapChains = 3;

class VulkanGraphicContext
{
public:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	VulkanGraphicContext(ANativeWindow* pWnd);
#elif VK_USE_PLATFORM_WIN32_KHR
	VulkanGraphicContext(HWND pWnd)
#endif
	~VulkanGraphicContext();

	virtual bool init();

	virtual void release();

	virtual void waitIdle();

	std::string getInfo() const { return "Vulkan"; }

	//Todo: Maybe more suitable for the renderwindow
	const VkFormat getDepthStencilImageFormat()const { return VK_FORMAT_R8G8B8A8_SNORM; };

	VkQueue getMainQueue() { return m_universalQueue; }

	VkDevice getDevice() { return m_device; }
	VkPhysicalDevice getGPU() { return m_physicalDevice; }
private:
	bool initVkInstance();
	bool initDebugCallbacks();
	bool initPhysicalDevice();
	bool initLogicalDevice();
	bool initSurface();
	bool initSwapChain(bool hasDepth, bool hasStencil, uint32_t& swapChainLength);

private:
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	ANativeWindow* m_pWnd;
#elif VK_USE_PLATFORM_WIN32_KHR
	HWND m_pWnd;
#endif 

	VkPhysicalDevice m_physicalDevice;
	VkDevice m_device;
	VkInstance m_instance;
	VkSurfaceKHR m_surface;
	VkDisplayKHR m_display;
	VkSwapchainKHR m_swapChain;

	uint32_t m_displayWidth;
	uint32_t m_displayHeight;

	uint32_t m_universalQueueFamily;
	uint32_t m_universalQueueCount;
	VkQueue  m_universalQueue;

	uint32_t m_transferQueueFamily;
	uint32_t m_transferQueueCount;
	VkQueue m_transferQueue;

	uint32_t m_sparseQueueFamily;
	uint32_t m_sparseQueueCount;
	VkQueue m_sparseQueue;
	
	VkPhysicalDeviceMemoryProperties    m_deviceMemProperties;

	VkCommandPool m_universalCommandPool;

	VkDebugReportCallbackEXT            m_debugReportCallback;
	bool                                m_supportsDebugReport;

    
	VkFence                             m_fenceAcquire[MaxSwapChains + 1];
	VkFence                             m_fencePrePresent[MaxSwapChains + 1];
	VkFence                             m_fenceRender[MaxSwapChains];
	VkCommandBuffer                     m_acquireBarrierCommandBuffersRenderQueue[MaxSwapChains];
	VkCommandBuffer                     m_presentBarrierCommandBuffersRenderQueue[MaxSwapChains];
	VkSemaphore                         m_semaphoreFinishedRendering[MaxSwapChains];
	VkSemaphore                         m_semaphoreCanPresent[MaxSwapChains];
	VkSemaphore                         m_semaphoreImageAcquired[MaxSwapChains + 1];
	VkSemaphore                         m_semaphoreCanBeginRendering[MaxSwapChains];

	uint32_t m_currentImageAcqSem;
	uint32_t m_swapIndex;

	VulkanRenderWindow* m_pWindow;
};

