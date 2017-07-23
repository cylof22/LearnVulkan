#pragma once
#include "common\vulkan_wrapper.h"
#include <gli\gl.hpp>
#include <gli\format.hpp>

class VulkanMemoryMgr
{
public:
	static VulkanMemoryMgr* get();

	bool memoryTypeFromProperties(const VkPhysicalDevice* pGPU, uint32_t typeBits, VkFlags requirementsMask, uint32_t& typeIndex);

	bool imageLayoutConversion(VkDevice gpuDevice, VkQueue presentQueue, VkCommandPool cmdPool,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	void setImageLayout(VkCommandBuffer cmdbuffer,
		VkImage image,
		VkImageLayout oldImageLayout,
		VkImageLayout newImageLayout,
		VkImageSubresourceRange subresourceRange,
		VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
		VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	VkFormat imageFormatConvert(const  gli::gl::format gliFormat);

private:
	static VulkanMemoryMgr* m_pMemoryMgr;
};