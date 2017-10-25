#pragma once
#include "common\vulkan_wrapper.h"

class VulkanGraphicContext;

class VulkanHardwareBuffer
{
public:
	VulkanHardwareBuffer(VulkanGraphicContext* pContext, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memoryProp, const void * pBufferData, uint32_t bufferSize);
	virtual ~VulkanHardwareBuffer();

	const VkBuffer& getBuffer() const { return m_buffer; }
	const VkDeviceMemory& getMemory() const { return m_memory; }
	const uint32_t getSize() const { return m_size; }

protected:
	VkBuffer m_buffer;
	VkDeviceMemory m_memory;
	VkBufferUsageFlags m_bufferUsage;
	uint32_t m_size;

	VulkanGraphicContext* m_pContext;
};

