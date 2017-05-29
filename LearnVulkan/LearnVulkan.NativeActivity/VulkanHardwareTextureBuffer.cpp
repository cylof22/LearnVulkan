#include "VulkanHardwareTextureBuffer.h"
#include "VulkanMemoryMgr.h"
#include "VkCommandBufferMgr.h"
#include "VulkanDevice.h"
#include <gli\load.hpp>
#include <gli\texture2d.hpp>

VulkanHardwareTextureBuffer::VulkanHardwareTextureBuffer(const VulkanDevice& device, 
	const VkCommandPool& rCmdPool, const char* pData, uint32_t size,
	VkImageUsageFlags imageUsage)
{
	gli::texture2d image2D(gli::load(pData, size));

	assert(!image2D.empty());

	uint32_t width = (uint32_t)image2D.extent()[0];
	uint32_t height = (uint32_t)image2D.extent()[1];
	uint32_t minMapLevels = (uint32_t)image2D.levels();
	gli::gl GL(gli::gl::PROFILE_GL33);
	gli::gl::format const gliFormat = GL.translate(image2D.format(), image2D.swizzles());
	VkFormat vkTextureFormat = VulkanMemoryMgr::get()->imageFormatConvert(gliFormat);
	VkResult res;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = vkTextureFormat;
	imageInfo.extent.width = (uint32_t)image2D.extent()[0];
	imageInfo.extent.height = (uint32_t)image2D.extent()[1];
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = (uint32_t)image2D.levels();
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage = imageUsage;
	imageInfo.flags = 0;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;

	res = vkCreateImage(device.getGraphicDevice(), &imageInfo, nullptr, &m_rImage);
	assert(res == VK_SUCCESS);

	// check the memory requirements
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device.getGraphicDevice(), m_rImage, &memoryRequirements);

	// create the device memory
	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.pNext = nullptr;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
	imageAllocateInfo.memoryTypeIndex = 0;

	// check the required memory type
	VulkanMemoryMgr::get()->memoryTypeFromProperties(device.getGPU(), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		imageAllocateInfo.memoryTypeIndex);

	res = vkAllocateMemory(device.getGraphicDevice(), &imageAllocateInfo, nullptr, &m_rMemory);
	assert(res == VK_SUCCESS);

	// bind the image device memory
	res = vkBindImageMemory(device.getGraphicDevice(), m_rImage, m_rMemory, 0);
	assert(res == VK_SUCCESS);

	// populate the device memory
	VkImageSubresource subResouce = {};
	subResouce.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResouce.mipLevel = 0;
	subResouce.arrayLayer = 0;

	VkSubresourceLayout layout;
	uint8_t *pMappedData;
	vkGetImageSubresourceLayout(device.getGraphicDevice(), m_rImage, &subResouce, &layout);

	res = vkMapMemory(device.getGraphicDevice(), m_rMemory, 0, imageAllocateInfo.allocationSize, 0, (void**)&pMappedData);
	assert(res == VK_SUCCESS);

	// Why not use memcpy directly?
	uint8_t* offsetData = (uint8_t*)pData;
	for (uint32_t y = 0; y < image2D.extent()[0]; ++y)
	{
		size_t imageSize = image2D.extent()[1] * 4;
		memcpy(pMappedData, offsetData, imageSize);

		pMappedData += layout.rowPitch;
	}
	vkUnmapMemory(device.getGraphicDevice(), m_rMemory);

	// use cmd buffer to convert the imagelayout to shader_read optimally
	VkCommandBuffer texturelayoutBuffer;
	VkCommandBufferMgr::get()->createCommandBuffer(&device.getGraphicDevice(), rCmdPool, &texturelayoutBuffer);
	VkCommandBufferMgr::get()->beginCommandBuffer(&texturelayoutBuffer);
	// How to define subresource range as a input parameter
	VkImageSubresourceRange textureSubResourceRange = {};
	textureSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	textureSubResourceRange.baseMipLevel = 0;
	textureSubResourceRange.levelCount = imageInfo.mipLevels;
	textureSubResourceRange.layerCount = 1;

	VulkanMemoryMgr::get()->imageLayoutConversion(m_rImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_PREINITIALIZED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_HOST_WRITE_BIT, texturelayoutBuffer, imageInfo.mipLevels);


	VkCommandBufferMgr::get()->endCommandBuffer(&texturelayoutBuffer);

	VkFence textureFence = VK_NULL_HANDLE;
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = 0;

	vkCreateFence(device.getGraphicDevice(), &fenceInfo, nullptr, &textureFence);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &texturelayoutBuffer;

	VkCommandBufferMgr::get()->submitCommandBuffer(device.getGraphicQueue(), &texturelayoutBuffer, &submitInfo, textureFence);
	vkWaitForFences(device.getGraphicDevice(), 1, &textureFence, VK_TRUE, UINT64_MAX);

	vkDestroyFence(device.getGraphicDevice(), textureFence, nullptr);
	VkCommandBufferMgr::get()->destroyCommandBuffer(&device.getGraphicDevice(), rCmdPool, &texturelayoutBuffer);

	// create the image sampler
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.pNext = nullptr;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_WHITE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;

	// check device feature of anisotropy: See VkPhysicalDeviceFeatures
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1;

	res = vkCreateSampler(device.getGraphicDevice(), &samplerInfo, nullptr, &m_rSampler);
	assert(res == VK_SUCCESS);

	// create the image view
	VkImageViewCreateInfo textureViewInfo = {};
	textureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	textureViewInfo.pNext = nullptr;
	textureViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	textureViewInfo.format = vkTextureFormat;
	textureViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	textureViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	textureViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	textureViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	textureViewInfo.subresourceRange = textureSubResourceRange;
	textureViewInfo.flags = 0;
	textureViewInfo.image = m_rImage;

	res = vkCreateImageView(device.getGraphicDevice(), &textureViewInfo, nullptr, &m_rView);
	assert(res == VK_SUCCESS);
} 

VulkanHardwareTextureBuffer::~VulkanHardwareTextureBuffer()
{
}
