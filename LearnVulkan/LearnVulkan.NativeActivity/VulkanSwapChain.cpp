#include "VulkanSwapChain.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"

VulkanSwapChain::VulkanSwapChain(VulkanInstance* pInstance, VulkanDevice* pDevice) : m_pInstance(pInstance), m_pDevice(pDevice)
{
}

VulkanSwapChain::~VulkanSwapChain()
{
	// destroy the image view
	for (size_t i = 0; i < m_colorBuffers.size(); i++)
		vkDestroyImageView(m_pDevice->getGraphicDevice(), m_colorBuffers[i].view, nullptr);

	// destroy the swapchain
	vkDestroySwapchainKHR(m_pDevice->getGraphicDevice(), m_swapchain, nullptr);
	// destroy the surface
	vkDestroySurfaceKHR(m_pInstance->getInstance(), m_surface, nullptr);

	m_colorBuffers.clear();
	m_surfaceFormats.clear();
	m_presentModes.clear();
}

bool VulkanSwapChain::init(ANativeWindow* pWnd)
{
	VkResult res = VK_SUCCESS;

	res = createSurface(pWnd);

	getSupportedFormats();

	// find a queue support graphic and present together
	uint32_t presentQueueIndex = getGraphicsQueueWithPresentation();
	// set the device's graphics and present queue

	if(res == VK_SUCCESS)
		res = createSwapChain(pWnd);
	
	if (res == VK_SUCCESS)
		res = getColorImages();

	if (res == VK_SUCCESS)
		res = createColorImageViews();

	return res == VK_SUCCESS;
}

void VulkanSwapChain::getSupportedFormats()
{
	if (m_pDevice)
	{
		const VkPhysicalDevice* pGPU = m_pDevice->getGPU();
		uint32_t formatSize = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(*pGPU, m_surface, &formatSize, nullptr);
		
		m_surfaceFormats.clear();
		m_surfaceFormats.resize(formatSize);

		if (formatSize != 0)
		{
			vkGetPhysicalDeviceSurfaceFormatsKHR(*pGPU, m_surface, &formatSize, m_surfaceFormats.data());
			if (formatSize == 1 && m_surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
				m_colorFormat = VK_FORMAT_R8G8B8A8_UNORM;
			else
				m_colorFormat = m_surfaceFormats[0].format;
		}
	}
}

VkResult VulkanSwapChain::createSurface(ANativeWindow* pWnd)
{
	VkResult res = VK_SUCCESS;

	VkAndroidSurfaceCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	info.pNext = nullptr;
	info.flags = 0;
	info.window = pWnd;

	res = vkCreateAndroidSurfaceKHR(m_pInstance->getInstance(), &info, nullptr, &m_surface);
	return res;
}

uint32_t VulkanSwapChain::getGraphicsQueueWithPresentation()
{
	// find a queue which support graphic and present together; if not, find a seperate present queue
	uint32_t queueCount = m_pDevice->getQueueFamilySize();
	const std::vector<VkQueueFamilyProperties>& queueProps = m_pDevice->getQueueFamilyProperties();

	// Iterate over each queue and get presentation status for each. 
	std::vector<VkBool32> supportsPresent;
	supportsPresent.resize(queueCount);
	for (uint32_t i = 0; i < queueCount; i++) {
		vkGetPhysicalDeviceSurfaceSupportKHR(*m_pDevice->getGPU(), i, m_surface, &supportsPresent[i]);
	}

	// Search for a graphics queue and a present queue in the array of queue 
	// families, try to find one that supports both 
	uint32_t graphicsQueueNodeIndex = UINT32_MAX;
	uint32_t presentQueueNodeIndex = UINT32_MAX;
	for (uint32_t i = 0; i < queueCount; i++) {
		if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
			if (graphicsQueueNodeIndex == UINT32_MAX) {
				graphicsQueueNodeIndex = i;
			}

			if (supportsPresent[i] == VK_TRUE) {
				graphicsQueueNodeIndex = i;
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	if (presentQueueNodeIndex == UINT32_MAX) {
		// If didn't find a queue that supports both graphics and present, then 
		// find a separate present queue. 
		for (uint32_t i = 0; i < queueCount; ++i) {
			if (supportsPresent[i] == VK_TRUE) {
				presentQueueNodeIndex = i;
				break;
			}
		}
	}

	// Generate error if could not find both a graphics and a present queue 
	if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX) {
		return  UINT32_MAX;
	}

	return graphicsQueueNodeIndex;
}

void VulkanSwapChain::getSurfaceCapabilityAndPresentMode(ANativeWindow* pWnd)
{
	VkResult res = VK_SUCCESS;
	const VkPhysicalDevice* pGPU = m_pDevice->getGPU();
	if (pGPU)
	{
		uint32_t presentModeSize = 0;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(*pGPU, m_surface, &m_surfaceCapabilities);
		vkGetPhysicalDeviceSurfacePresentModesKHR(*pGPU, m_surface, &presentModeSize, nullptr);

		if (presentModeSize > 0)
		{
			m_presentModes.clear();
			m_presentModes.resize(presentModeSize);

			res = vkGetPhysicalDeviceSurfacePresentModesKHR(*pGPU, m_surface, &presentModeSize, m_presentModes.data());
		}

		if (m_surfaceCapabilities.currentExtent.width == (uint32_t)(-1))
		{
			// reset the extent size to the window's size
			m_surfaceCapabilities.currentExtent.width = ANativeWindow_getWidth(pWnd);
			m_surfaceCapabilities.currentExtent.height = ANativeWindow_getHeight(pWnd);
		}
		else
		{
			m_swapChainSize = m_surfaceCapabilities.currentExtent;
		}
	}
}

void VulkanSwapChain::managePresentMode()
{
	m_swapChainMode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < m_presentModes.size(); i++)
	{
		if (m_presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			m_swapChainMode = VK_PRESENT_MODE_MAILBOX_KHR;
			break;
		}

		if (m_swapChainMode == VK_PRESENT_MODE_MAILBOX_KHR
			&& m_presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			m_swapChainMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
	}

	m_numberOfSwapChainImages = m_surfaceCapabilities.minImageCount + 1;
	if (m_surfaceCapabilities.maxImageCount > 0
		&& m_numberOfSwapChainImages > m_surfaceCapabilities.maxImageCount)
	{
		m_numberOfSwapChainImages = m_surfaceCapabilities.maxImageCount;
	}
	
	m_swapChainImages.resize(m_numberOfSwapChainImages);

	if (m_surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		m_surfaceTransformFlags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	else
		m_surfaceTransformFlags = m_surfaceCapabilities.currentTransform;
}

VkResult VulkanSwapChain::createSwapChain(ANativeWindow* pWnd)
{
	getSurfaceCapabilityAndPresentMode(pWnd);

	managePresentMode();

	VkSwapchainCreateInfoKHR info = {};
	info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	info.pNext = nullptr;
	info.surface = m_surface;
	info.minImageCount = m_numberOfSwapChainImages;
	info.imageFormat = m_colorFormat;
	info.imageExtent = m_swapChainSize;
	info.preTransform = m_surfaceTransformFlags;
	info.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	info.imageArrayLayers = 1;
	info.presentMode = m_swapChainMode;
	info.oldSwapchain = VK_NULL_HANDLE;
	info.clipped = true;
	info.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;

	VkResult res = vkCreateSwapchainKHR(m_pDevice->getGraphicDevice(), &info, nullptr, &m_swapchain);;
	return res;
}

VkResult VulkanSwapChain::createColorBuffers()
{
	VkResult res = VK_SUCCESS;
	res = getColorImages();
	if (res == VK_SUCCESS)
		res = createColorImageViews();
	return res;
}

VkResult VulkanSwapChain::getColorImages()
{
	VkResult res = VK_SUCCESS;
	uint32_t swapChainImageSize = 0;
	res = vkGetSwapchainImagesKHR(m_pDevice->getGraphicDevice(), m_swapchain, &swapChainImageSize, nullptr);

	m_swapChainImages.clear();
	m_swapChainImages.resize(swapChainImageSize);
	res = vkGetSwapchainImagesKHR(m_pDevice->getGraphicDevice(), m_swapchain, &swapChainImageSize, m_swapChainImages.data());

	return res;
}

VkResult VulkanSwapChain::createColorImageViews()
{
	VkResult res = VK_SUCCESS;

	for (size_t i = 0; i < m_swapChainImages.size(); i++)
	{
		VkImageViewCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		info.pNext = nullptr;
		info.format = m_colorFormat;
		info.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
		info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		info.subresourceRange.baseMipLevel = 0;
		info.subresourceRange.levelCount = 1;
		info.subresourceRange.baseArrayLayer = 0;
		info.subresourceRange.layerCount = 1;
		info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		info.flags = 0;
		info.image = m_swapChainImages[i];

		VkImageView imageView;
		res = vkCreateImageView(m_pDevice->getGraphicDevice(), &info, nullptr, &imageView);

		if (res == VK_SUCCESS)
		{
			ColorBuffer colorBuffer;
			colorBuffer.image = m_swapChainImages[i];
			colorBuffer.view = imageView;
			m_colorBuffers.emplace_back(colorBuffer);
		}	
	}

	return res;
}
