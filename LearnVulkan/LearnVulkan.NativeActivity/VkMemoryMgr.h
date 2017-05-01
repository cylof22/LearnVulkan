#pragma once
#include "common\vulkan_wrapper.h"

class VulkanMemoryMgr
{
public:
	static VulkanMemoryMgr* get();

	bool memoryTypeFromProperties(const VkPhysicalDevice* pGPU, uint32_t typeBits, VkFlags requirementsMask, uint32_t& typeIndex);
	bool imageLayoutConversion(const VkImage& image, const VkImageAspectFlags aspectMask, VkImageLayout srcLayout, VkImageLayout dstLayout, 
		VkAccessFlagBits srcAccessFlags, const VkCommandBuffer& cmdBuffer);
private:
	static VulkanMemoryMgr* m_pMemoryMgr;
};