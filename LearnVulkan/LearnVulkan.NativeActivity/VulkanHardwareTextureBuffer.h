#pragma once
#include "common\vulkan_wrapper.h"

class VulkanDevice;

class VulkanHardwareTextureBuffer
{
public:
	VulkanHardwareTextureBuffer(const VulkanDevice& rDevice, const VkCommandPool& rCmdPool,
		const char* pData,  uint32_t size, 
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT);
	~VulkanHardwareTextureBuffer();
private:
	VkImage m_rImage;
	VkImageView m_rView;
	VkSampler m_rSampler;
	VkDeviceMemory m_rMemory;
	VkDescriptorImageInfo m_rImageDescriptor;
};
