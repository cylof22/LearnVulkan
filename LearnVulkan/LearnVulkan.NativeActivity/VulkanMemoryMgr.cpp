#include "pch.h"
#include "VulkanMemoryMgr.h"

VulkanMemoryMgr* VulkanMemoryMgr::m_pMemoryMgr = nullptr;

VkPhysicalDeviceMemoryProperties g_GpuMemoryProperties = {};
bool isDevicePropertiesReady = false;

VulkanMemoryMgr * VulkanMemoryMgr::get()
{
	if (m_pMemoryMgr == nullptr)
		m_pMemoryMgr = new VulkanMemoryMgr();

	return m_pMemoryMgr;
}

bool VulkanMemoryMgr::memoryTypeFromProperties(VkPhysicalDevice rGPU, uint32_t typeBits, VkFlags requirementsMask, uint32_t& typeIndex)
{
	if (rGPU != VK_NULL_HANDLE)
	{
		if (!isDevicePropertiesReady)
		{
			vkGetPhysicalDeviceMemoryProperties(rGPU, &g_GpuMemoryProperties);
			isDevicePropertiesReady = true;
		}
		
		for (uint32_t i = 0; i < g_GpuMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((g_GpuMemoryProperties.memoryTypes[i].propertyFlags & requirementsMask) == requirementsMask)
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

bool VulkanMemoryMgr::imageLayoutConversion(VkDevice gpuDevice, VkQueue presentQueue, VkCommandPool cmdPool, 
	VkImage image,
	VkImageLayout oldImageLayout,
	VkImageLayout newImageLayout,
	VkImageSubresourceRange subresourceRange,
	VkPipelineStageFlags srcStageMask /*= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT*/,
	VkPipelineStageFlags dstStageMask /*= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT*/)
{
	VkCommandBufferAllocateInfo staggingCmdInfo;
	staggingCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	staggingCmdInfo.pNext = NULL;
	staggingCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	staggingCmdInfo.commandPool = cmdPool;
	staggingCmdInfo.commandBufferCount = 1;

	VkCommandBuffer staggingCmd;
	VK_VALIDATION_RESULT(vkAllocateCommandBuffers(gpuDevice, &staggingCmdInfo, &staggingCmd));

	VkCommandBufferBeginInfo staggingBeginInfo = {};
	staggingBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	staggingBeginInfo.pNext = NULL;
	staggingBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	staggingBeginInfo.pInheritanceInfo = NULL;

	VK_VALIDATION_RESULT(vkBeginCommandBuffer(staggingCmd, &staggingBeginInfo));

	setImageLayout(staggingCmd, image, oldImageLayout, newImageLayout, subresourceRange, srcStageMask, dstStageMask);

	VK_VALIDATION_RESULT(vkEndCommandBuffer(staggingCmd));

	VkFence conversionFence;
	VkFenceCreateInfo conversionFenceInfo;
	conversionFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	conversionFenceInfo.pNext = NULL;
	conversionFenceInfo.flags = 0;
	VK_VALIDATION_RESULT(vkCreateFence(gpuDevice, &conversionFenceInfo, VK_ALLOC_CALLBACK, &conversionFence));

	VkSubmitInfo skyboxSubmitInfo = {};
	skyboxSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	skyboxSubmitInfo.pNext = NULL;
	skyboxSubmitInfo.commandBufferCount = 1;
	skyboxSubmitInfo.pCommandBuffers = &staggingCmd;

	VK_VALIDATION_RESULT(vkQueueSubmit(presentQueue, 1, &skyboxSubmitInfo, conversionFence));

	VK_VALIDATION_RESULT(vkWaitForFences(gpuDevice, 1, &conversionFence, true, UINT64_MAX));

	vkFreeCommandBuffers(gpuDevice, cmdPool, 1, &staggingCmd);
	vkDestroyFence(gpuDevice, conversionFence, VK_ALLOC_CALLBACK);
	return true;
}

void VulkanMemoryMgr::setImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask)
{
	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.pNext = nullptr;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source 
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(cmdbuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

VkFormat VulkanMemoryMgr::imageFormatConvert(const gli::gl::format gliFormat)
{
	VkFormat vkImageFormat = VK_FORMAT_UNDEFINED;

	switch (gliFormat.Internal)
	{
	case gli::gl::INTERNAL_RGBA8_UNORM:
		vkImageFormat = VK_FORMAT_R8G8B8A8_UNORM;
		break;
	default:
		vkImageFormat = VK_FORMAT_UNDEFINED;
	}

	return vkImageFormat;
}
