#include "pch.h"
#include "VkMemoryMgr.h"

VulkanMemoryMgr* VulkanMemoryMgr::m_pMemoryMgr = nullptr;

VulkanMemoryMgr * VulkanMemoryMgr::get()
{
	if (m_pMemoryMgr == nullptr)
		m_pMemoryMgr = new VulkanMemoryMgr();

	return nullptr;
}

bool VulkanMemoryMgr::memoryTypeFromProperties(const VkPhysicalDevice* pGPU, uint32_t typeBits, VkFlags requirementsMask, uint32_t& typeIndex)
{
	if (pGPU)
	{
		VkPhysicalDeviceMemoryProperties gpuMemoryProperties;
		vkGetPhysicalDeviceMemoryProperties(*pGPU, &gpuMemoryProperties);

		for (uint32_t i = 0; i < 32; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((gpuMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
				{
					typeIndex = i;
					return true;
				}
			}
			typeBits >>= 1;
		}
	}
	
	return false;
}

bool VulkanMemoryMgr::imageLayoutConversion(const VkImage& image, const VkImageAspectFlags aspectMask, VkImageLayout srcLayout, VkImageLayout dstLayout, 
	VkAccessFlagBits srcAcessFlag, const VkCommandBuffer & cmdBuffer)
{
	if (cmdBuffer == VK_NULL_HANDLE)
		return false;

	VkImageMemoryBarrier imageBarrier;
	imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageBarrier.pNext = nullptr;
	imageBarrier.srcAccessMask = srcAcessFlag;
	imageBarrier.dstAccessMask = 0;
	imageBarrier.oldLayout = srcLayout;
	imageBarrier.newLayout = dstLayout;
	imageBarrier.image = image;
	imageBarrier.subresourceRange.aspectMask = aspectMask;
	imageBarrier.subresourceRange.baseMipLevel = 0;
	imageBarrier.subresourceRange.levelCount = 1;
	imageBarrier.subresourceRange.baseArrayLayer = 0;
	imageBarrier.subresourceRange.layerCount = 1;

	if (srcLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	switch (dstLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		imageBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_UNDEFINED:
		assert(0);
		break;
	}

	VkPipelineStageFlags srcStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	vkCmdPipelineBarrier(cmdBuffer, srcStageFlags, dstStageFlags, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

	return true;
}
