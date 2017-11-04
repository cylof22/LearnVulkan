#pragma once
#include "common\vulkan_wrapper.h"

class VulkanGraphicContext;
class VulkanHardwareTextureBuffer;

class VulkanHardwareBuffer
{
public:
	VulkanHardwareBuffer(VulkanGraphicContext* pContext, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryProp, const void * pBufferData, uint32_t bufferSize);
	virtual ~VulkanHardwareBuffer();

	const VkBuffer& getBuffer() const { return m_buffer; }
	const VkDeviceMemory& getMemory() const { return m_memory; }
	const uint32_t size() const { return m_size; }

	bool updateBuffer(VkCommandBuffer& cmdBuffer, const void* pUniformData, VkDeviceSize uniformSize, VkDeviceSize offset = 0);
	bool copyBuffer(VkCommandBuffer& cmdBuffer, VulkanHardwareBuffer src, uint32_t srcOffset, uint32_t destOffset, uint32_t sizeInBytes);
	bool copyImage(VkCommandBuffer& cmdBuffer, VulkanHardwareTextureBuffer srcTexture, VkBufferImageCopy * regions, uint32_t numRegions);
protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_memory;
	VkBufferUsageFlags m_bufferUsage;
	uint32_t m_size;

	VulkanGraphicContext* m_pContext;
};

