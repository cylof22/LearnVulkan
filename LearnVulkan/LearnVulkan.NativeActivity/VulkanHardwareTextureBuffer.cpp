#include "VulkanHardwareTextureBuffer.h"
#include "VulkanMemoryMgr.h"
#include "VkCommandBufferMgr.h"
#include "VulkanDevice.h"
#include <gli\load.hpp>
#include <gli\texture2d.hpp>
#include "Stb\stb_image.h"

VulkanHardwareTextureBuffer::VulkanHardwareTextureBuffer(const VulkanDevice* pDevice, 
	const VkCommandPool& rCmdPool, const char* pData, uint32_t size,
	VkImageUsageFlags imageUsage, bool isUseStb /*=false*/)
{
	uint32_t width, height, minMapLevels;
	VkFormat vkTextureFormat;
	uint8_t* pImageData;

	if (!isUseStb)
	{
		gli::texture2d image2D(gli::load(pData, size));

		assert(!image2D.empty());

		width = (uint32_t)image2D.extent()[0];
		height = (uint32_t)image2D.extent()[1];
		minMapLevels = (uint32_t)image2D.levels();
		gli::gl GL(gli::gl::PROFILE_GL33);
		gli::gl::format const gliFormat = GL.translate(image2D.format(), image2D.swizzles());
		vkTextureFormat = VulkanMemoryMgr::get()->imageFormatConvert(gliFormat);
		pImageData = (uint8_t*)image2D.data();
	}
	else
	{
		int x, y, comp;
		stbi_uc* pngData = stbi_load_from_memory((stbi_uc*)pData, size, &x, &y, &comp, 4);
		
		width = x;
		height = y;
		minMapLevels = 1;
		vkTextureFormat = VK_FORMAT_R8G8B8A8_UNORM;
		pImageData = (uint8_t*) pngData;
	}
	
	VkResult res;

	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.pNext = nullptr;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = vkTextureFormat;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = minMapLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.queueFamilyIndexCount = 0;
	imageInfo.pQueueFamilyIndices = nullptr;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.usage = imageUsage;
	imageInfo.flags = 0;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.tiling = VK_IMAGE_TILING_LINEAR;

	res = vkCreateImage(pDevice->getGraphicDevice(), &imageInfo, VK_ALLOC_CALLBACK, &m_rImage);
	assert(res == VK_SUCCESS);

	// check the memory requirements
	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(pDevice->getGraphicDevice(), m_rImage, &memoryRequirements);

	// create the device memory
	VkMemoryAllocateInfo imageAllocateInfo = {};
	imageAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	imageAllocateInfo.pNext = nullptr;
	imageAllocateInfo.allocationSize = memoryRequirements.size;
	imageAllocateInfo.memoryTypeIndex = 0;

	// check the required memory type
	VulkanMemoryMgr::get()->memoryTypeFromProperties(*pDevice->getGPU(), memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		imageAllocateInfo.memoryTypeIndex);

	res = vkAllocateMemory(pDevice->getGraphicDevice(), &imageAllocateInfo, nullptr, &m_rMemory);
	assert(res == VK_SUCCESS);

	// How to define subresource range as a input parameter
	VkImageSubresourceRange textureSubResourceRange = {};
	textureSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	textureSubResourceRange.baseMipLevel = 0;
	textureSubResourceRange.levelCount = imageInfo.mipLevels;
	textureSubResourceRange.baseArrayLayer = 0;
	textureSubResourceRange.layerCount = imageInfo.arrayLayers;

	// bind the image device memory
	res = vkBindImageMemory(pDevice->getGraphicDevice(), m_rImage, m_rMemory, 0);
	assert(res == VK_SUCCESS);

	// Use stagging memory
	if (true)
	{
		// Use stagging buffer to copy the skybox image
		struct {
			VkBuffer buffer;
			VkDeviceMemory memory;
		} staging_res;

		VkBufferCreateInfo buffer_info = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
			width * height * 4 ,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, NULL
		};
		vkCreateBuffer(pDevice->getGraphicDevice(), &buffer_info, VK_ALLOC_CALLBACK, &staging_res.buffer);

		VkMemoryRequirements mreq_buffer = { 0 };
		vkGetBufferMemoryRequirements(pDevice->getGraphicDevice(), staging_res.buffer, &mreq_buffer);

		uint32_t memoryType = 0;
		VulkanMemoryMgr::get()->memoryTypeFromProperties(*pDevice->getGPU(), mreq_buffer.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memoryType);
		VkMemoryAllocateInfo alloc_info_buffer = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, mreq_buffer.size,
			memoryType
		};
		vkAllocateMemory(pDevice->getGraphicDevice(), &alloc_info_buffer, nullptr, &staging_res.memory);
		vkBindBufferMemory(pDevice->getGraphicDevice(), staging_res.buffer, staging_res.memory, 0);

		uint8_t *ptr;
		vkMapMemory(pDevice->getGraphicDevice(), staging_res.memory, 0, VK_WHOLE_SIZE, 0, (void **)&ptr);
		VkDeviceSize offset = 0;
		VkBufferImageCopy buffer_copy_region;
			
		size_t size = width * height * 4;
		memcpy(ptr + offset, pImageData, size);

		buffer_copy_region.bufferRowLength = 0;
		buffer_copy_region.bufferImageHeight = 0;
		buffer_copy_region.bufferOffset = offset;
		buffer_copy_region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		buffer_copy_region.imageSubresource.baseArrayLayer = 0;
		buffer_copy_region.imageSubresource.layerCount = 1;
		buffer_copy_region.imageSubresource.mipLevel = 0;
		buffer_copy_region.imageExtent.width = width;
		buffer_copy_region.imageExtent.height = height;
		buffer_copy_region.imageExtent.depth = 1;
		buffer_copy_region.imageOffset.x = 0;
		buffer_copy_region.imageOffset.y = 0;
		buffer_copy_region.imageOffset.z = 0;

		stbi_image_free(pImageData);

		vkUnmapMemory(pDevice->getGraphicDevice(), staging_res.memory);

		// update the stagging buffer into the cubebox image
		VkCommandBufferAllocateInfo staggingCmdInfo;
		staggingCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		staggingCmdInfo.pNext = NULL;
		staggingCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		staggingCmdInfo.commandPool = rCmdPool;
		staggingCmdInfo.commandBufferCount = 1;

		VkCommandBuffer staggingCmd;
		vkAllocateCommandBuffers(pDevice->getGraphicDevice(), &staggingCmdInfo, &staggingCmd);

		VkCommandBufferBeginInfo staggingBeginInfo = {};
		staggingBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		staggingBeginInfo.pNext = NULL;
		staggingBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		staggingBeginInfo.pInheritanceInfo = NULL;

		VkImageSubresourceRange textureSubresourceRange;
		textureSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		textureSubresourceRange.baseMipLevel = 0;
		textureSubresourceRange.levelCount = 1;
		textureSubresourceRange.baseArrayLayer = 0;
		textureSubresourceRange.layerCount = 1;

		vkBeginCommandBuffer(staggingCmd, &staggingBeginInfo);

		VulkanMemoryMgr::get()->setImageLayout(staggingCmd, m_rImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureSubResourceRange);

		vkCmdCopyBufferToImage(staggingCmd, staging_res.buffer, m_rImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &buffer_copy_region);

		VulkanMemoryMgr::get()->setImageLayout(staggingCmd, m_rImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textureSubResourceRange);

		vkEndCommandBuffer(staggingCmd);

		VkFence skyboxFence;
		VkFenceCreateInfo skyboxFenceInfo;
		skyboxFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		skyboxFenceInfo.pNext = NULL;
		skyboxFenceInfo.flags = 0;
		vkCreateFence(pDevice->getGraphicDevice(), &skyboxFenceInfo, VK_ALLOC_CALLBACK, &skyboxFence);

		VkPipelineStageFlags skyboxPipelineStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		VkSubmitInfo skyboxSubmitInfo = {};
		skyboxSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		skyboxSubmitInfo.pNext = NULL;
		skyboxSubmitInfo.commandBufferCount = 1;
		skyboxSubmitInfo.pCommandBuffers = &staggingCmd;

		vkQueueSubmit(pDevice->getGraphicQueue(), 1, &skyboxSubmitInfo, skyboxFence);

		vkWaitForFences(pDevice->getGraphicDevice(), 1, &skyboxFence, true, UINT64_MAX);
	}
	else
	{
		// populate the device memory
		VkImageSubresource subResouce = {};
		subResouce.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResouce.mipLevel = 0;
		subResouce.arrayLayer = 0;

		VkSubresourceLayout layout;
		vkGetImageSubresourceLayout(pDevice->getGraphicDevice(), m_rImage, &subResouce, &layout);

		uint8_t *pMappedData;
		res = vkMapMemory(pDevice->getGraphicDevice(), m_rMemory, 0, imageAllocateInfo.allocationSize, 0, (void**)&pMappedData);
		assert(res == VK_SUCCESS);

		// Why not use memcpy directly?
		//memcpy(pMappedData, image2D.data(), memoryRequirements.size);
		uint8_t* offsetData = pImageData;
		for (uint32_t y = 0; y < width; ++y)
		{
			size_t imageSize = height * 4;
			memcpy(pMappedData, offsetData, imageSize);

			offsetData += imageSize;
			pMappedData += layout.rowPitch;
		}
		vkUnmapMemory(pDevice->getGraphicDevice(), m_rMemory);

		// convert to shader reader image
		VkImageSubresourceRange textureSubResourceRange;
		textureSubResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		textureSubResourceRange.baseMipLevel = 0;
		textureSubResourceRange.levelCount = imageInfo.mipLevels;
		textureSubResourceRange.baseArrayLayer = 0;
		textureSubResourceRange.layerCount = imageInfo.arrayLayers;

		VulkanMemoryMgr::get()->imageLayoutConversion(pDevice->getGraphicDevice(), pDevice->getGraphicQueue(), rCmdPool, 
			m_rImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textureSubResourceRange);
	}

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

	res = vkCreateSampler(pDevice->getGraphicDevice(), &samplerInfo, VK_ALLOC_CALLBACK, &m_rSampler);
	assert(res == VK_SUCCESS);

	// create the image view
	VkImageViewCreateInfo textureViewInfo = {};
	textureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
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

	res = vkCreateImageView(pDevice->getGraphicDevice(), &textureViewInfo, VK_ALLOC_CALLBACK, &m_rView);
	assert(res == VK_SUCCESS);

	m_rImageDescriptor.sampler = m_rSampler;
	m_rImageDescriptor.imageView = m_rView;
	m_rImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}

VulkanHardwareTextureBuffer::VulkanHardwareTextureBuffer(const VulkanDevice * pDevice, const VkCommandPool & rCmdPool, 
	std::vector<std::pair<char*, long>>& rDataArray, VkImageUsageFlags usageFlags, VkImageViewType viewType, bool isUseStb)
{
	if (!rDataArray.empty())
	{
		VkResult res;
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;

		VkBufferCreateInfo buffer_info = {
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL, 0,
			1024 * 1024 * 4 * 6,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE, 0, NULL
		};
		res = vkCreateBuffer(pDevice->getGraphicDevice(), &buffer_info, VK_ALLOC_CALLBACK, &stagingBuffer);

		VkMemoryRequirements mreq_buffer = { 0 };
		vkGetBufferMemoryRequirements(pDevice->getGraphicDevice(), stagingBuffer, &mreq_buffer);

		VkMemoryAllocateInfo alloc_info_buffer = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, mreq_buffer.size,
			0 };

		// check the required memory type
		VulkanMemoryMgr::get()->memoryTypeFromProperties(*pDevice->getGPU(), mreq_buffer.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			alloc_info_buffer.memoryTypeIndex);

		res = vkAllocateMemory(pDevice->getGraphicDevice(), &alloc_info_buffer, nullptr, &stagingMemory);
		res = vkBindBufferMemory(pDevice->getGraphicDevice(), stagingBuffer, stagingMemory, 0);

		uint8_t *ptr;
		res = vkMapMemory(pDevice->getGraphicDevice(), stagingMemory, 0, VK_WHOLE_SIZE, 0, (void **)&ptr);

		VkDeviceSize offset = 0;
		std::array<VkBufferImageCopy, 6> buffer_copy_regions;
		for (int i = 0; i < 6; ++i)
		{
			int w = 1024, h = 1024, comp = 4;
			stbi_uc* data = stbi_load_from_memory((stbi_uc*)rDataArray[i].first, rDataArray[i].second, &w, &h, &comp, 4);
			if (!data)
				continue;

			size_t size = w * h * comp;
			memcpy(ptr + offset, data, size);

			buffer_copy_regions[i].bufferRowLength = 0;
			buffer_copy_regions[i].bufferImageHeight = 0;
			buffer_copy_regions[i].bufferOffset = size;
			buffer_copy_regions[i].imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			buffer_copy_regions[i].imageSubresource.baseArrayLayer = i;
			buffer_copy_regions[i].imageSubresource.layerCount = 1;
			buffer_copy_regions[i].imageSubresource.mipLevel = 0;
			buffer_copy_regions[i].imageExtent.width = w;
			buffer_copy_regions[i].imageExtent.height = h;
			buffer_copy_regions[i].imageExtent.depth = 1;
			buffer_copy_regions[i].imageOffset.x = 0;
			buffer_copy_regions[i].imageOffset.y = 0;
			buffer_copy_regions[i].imageOffset.z = 0;

			offset += size;
			stbi_image_free((stbi_uc*)data);
		}

		vkUnmapMemory(pDevice->getGraphicDevice(), stagingMemory);

		VkImageCreateInfo image_info = {
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, NULL, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
			VK_IMAGE_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM,{ 1024, 1024, 1 }, 1, 6, VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE, 0,
			NULL, VK_IMAGE_LAYOUT_UNDEFINED
		};

		res = vkCreateImage(pDevice->getGraphicDevice(), &image_info, VK_ALLOC_CALLBACK, &m_rImage);

		VkMemoryRequirements mreq_image = { 0 };
		vkGetImageMemoryRequirements(pDevice->getGraphicDevice(), m_rImage, &mreq_image);

		VkDeviceMemory optimal_image_mem;
		VkMemoryAllocateInfo alloc_info_image = {
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, NULL, mreq_image.size,
			0
		};

		VulkanMemoryMgr::get()->memoryTypeFromProperties(*pDevice->getGPU(), mreq_image.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			alloc_info_image.memoryTypeIndex);

		res = vkAllocateMemory(pDevice->getGraphicDevice(), &alloc_info_image, nullptr, &optimal_image_mem);
		res = vkBindImageMemory(pDevice->getGraphicDevice(), m_rImage, optimal_image_mem, 0);

		// update the stagging buffer into the cubebox image
		VkCommandBufferAllocateInfo staggingCmdInfo;
		staggingCmdInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		staggingCmdInfo.pNext = NULL;
		staggingCmdInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		staggingCmdInfo.commandPool = rCmdPool;
		staggingCmdInfo.commandBufferCount = 1;

		VkCommandBuffer staggingCmd;
		vkAllocateCommandBuffers(pDevice->getGraphicDevice(), &staggingCmdInfo, &staggingCmd);

		VkCommandBufferBeginInfo staggingBeginInfo = {};
		staggingBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		staggingBeginInfo.pNext = NULL;
		staggingBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		staggingBeginInfo.pInheritanceInfo = NULL;

		vkBeginCommandBuffer(staggingCmd, &staggingBeginInfo);

		VkImageSubresourceRange textureSubresourceRange;
		textureSubresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		textureSubresourceRange.baseMipLevel = 0;
		textureSubresourceRange.levelCount = image_info.mipLevels;
		textureSubresourceRange.baseArrayLayer = 0;
		textureSubresourceRange.layerCount = image_info.arrayLayers;

		VulkanMemoryMgr::get()->setImageLayout(staggingCmd, m_rImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, textureSubresourceRange);

		vkCmdCopyBufferToImage(staggingCmd, stagingBuffer, m_rImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, buffer_copy_regions.size(), buffer_copy_regions.data());

		VulkanMemoryMgr::get()->setImageLayout(staggingCmd, m_rImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, textureSubresourceRange);

		vkEndCommandBuffer(staggingCmd);

		VkFence skyboxFence;
		VkFenceCreateInfo skyboxFenceInfo;
		skyboxFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		skyboxFenceInfo.pNext = NULL;
		skyboxFenceInfo.flags = 0;
		vkCreateFence(pDevice->getGraphicDevice(), &skyboxFenceInfo, nullptr, &skyboxFence);

		VkPipelineStageFlags skyboxPipelineStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		VkSubmitInfo skyboxSubmitInfo = {};
		skyboxSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		skyboxSubmitInfo.pNext = NULL;
		skyboxSubmitInfo.commandBufferCount = 1;
		skyboxSubmitInfo.pCommandBuffers = &staggingCmd;

		vkQueueSubmit(pDevice->getGraphicQueue(), 1, &skyboxSubmitInfo, skyboxFence);

		vkWaitForFences(pDevice->getGraphicDevice(), 1, &skyboxFence, true, UINT64_MAX);

		VkImageViewCreateInfo image_view_info = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, NULL, 0,
			m_rImage, VK_IMAGE_VIEW_TYPE_CUBE, VK_FORMAT_R8G8B8A8_UNORM,
			{ VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A },
			{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 6 }
		};
		res = vkCreateImageView(pDevice->getGraphicDevice(), &image_view_info, VK_ALLOC_CALLBACK, &m_rView);

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

		res = vkCreateSampler(pDevice->getGraphicDevice(), &samplerInfo, VK_ALLOC_CALLBACK, &m_rSampler);
		assert(res == VK_SUCCESS);

		m_rImageDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		m_rImageDescriptor.imageView = m_rView;
		m_rImageDescriptor.sampler = m_rSampler;
	}
}

VulkanHardwareTextureBuffer::~VulkanHardwareTextureBuffer()
{
}
