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

	bool init();

private:
	void getSupportedFormats();
	VkResult createSurface();
	uint32_t getGraphicsQueueWithPresentation();
	void getSurfaceCapabilityAndPresentMode();
	void managePresentMode();

	VkResult createSwapChain();
	VkResult createColorBuffers();
	VkResult getColorImages();
	VkResult createColorImageViews();

private:
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapchain;
	uint32_t m_activeColorBuffer;
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

	struct ColorBuffer
	{
		VkImage image;
		VkImageView view;
	};

	std::vector<ColorBuffer> m_colorBuffers;

	VulkanInstance* m_pInstance;
	VulkanDevice* m_pDevice;
};