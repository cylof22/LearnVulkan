#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>

class VulkanDevice;

class VulkanHardwareTextureBuffer
{
public:
	VulkanHardwareTextureBuffer(const VulkanDevice* rDevice, const VkCommandPool& rCmdPool,
		const char* pData,  uint32_t size, 
		VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT, bool isUseStb = false);

	VulkanHardwareTextureBuffer(const VulkanDevice* pDevice, const VkCommandPool& rCmdPool,
		std::vector<std::pair<char*, long>>& rDataArray, VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_SAMPLED_BIT,
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_CUBE, bool isUseStb = false);

	~VulkanHardwareTextureBuffer();

	VkDescriptorImageInfo& getDescriptorInfo() { return m_rImageDescriptor; }
	const VkImage& image() const { return m_rImage; }
	const VkImageLayout layout() const { return m_rImageDescriptor.imageLayout; }

private:
	VkImage m_rImage;
	VkImageView m_rView;
	VkSampler m_rSampler;
	VkDeviceMemory m_rMemory;
	VkDescriptorImageInfo m_rImageDescriptor;
};
