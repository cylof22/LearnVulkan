#pragma once
#include "common\vulkan_wrapper.h"
#include <gli\gl.hpp>
#include <gli\format.hpp>

class VulkanMemoryMgr
{
public:
	static VulkanMemoryMgr* get();

	bool memoryTypeFromProperties(const VkPhysicalDevice* pGPU, uint32_t typeBits, VkFlags requirementsMask, uint32_t& typeIndex);
	bool imageLayoutConversion(const VkImage& image, const VkImageAspectFlags aspectMask, VkImageLayout srcLayout, VkImageLayout dstLayout, 
		VkAccessFlags srcAccessFlags, const VkCommandBuffer& cmdBuffer, uint32_t mipMapLevels = 1, uint32_t layerCount = 1);
	VkFormat imageFormatConvert(const  gli::gl::format gliFormat);

private:
	static VulkanMemoryMgr* m_pMemoryMgr;
};