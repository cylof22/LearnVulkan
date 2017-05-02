#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>
class VulkanInstance;
class VulkanDevice;

class VulkanSwapChain
{
public:
	VulkanSwapChain(VulkanInstance* pInstance, VulkanDevice* pDevice);
	~VulkanSwapChain();

	bool init(ANativeWindow* pWnd);

	struct ColorBuffer
	{
		VkImage image;
		VkImageView view;
	};

	VkFormat getColorFormat() const { return m_colorFormat; }

	const std::vector<ColorBuffer>& getColorBuffers() const { return m_colorBuffers; }

	const VkSwapchainKHR* getSwapChain() const { return &m_swapchain; }
private:
	void getSupportedFormats();
	VkResult createSurface(ANativeWindow* pWnd);
	uint32_t getGraphicsQueueWithPresentation();
	void getSurfaceCapabilityAndPresentMode(ANativeWindow* pWnd);
	void managePresentMode();

	VkResult createSwapChain(ANativeWindow* pWnd);
	VkResult createColorBuffers();
	VkResult getColorImages();
	VkResult createColorImageViews();

private:
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapchain;
	VkFormat m_colorFormat;

	VkSurfaceCapabilitiesKHR m_surfaceCapabilities;
	std::vector<VkPresentModeKHR> m_presentModes;
	VkExtent2D m_swapChainSize;
	uint32_t m_numberOfSwapChainImages;
	VkSurfaceTransformFlagBitsKHR m_surfaceTransformFlags;
	VkPresentModeKHR m_swapChainMode;
	// only support double buffer, use array
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
	std::vector<ColorBuffer> m_colorBuffers;
	
	VulkanInstance* m_pInstance;
	VulkanDevice* m_pDevice;
};