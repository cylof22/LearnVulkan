#pragma once
#include "common\vulkan_wrapper.h"

class VulkanHardwareIndexBuffer
{
public:
	VulkanHardwareIndexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pIndexData, uint32_t indexSize);
	~VulkanHardwareIndexBuffer();

	const VkBuffer& getBuffer() { return m_indexBuffer; }
	const uint32_t size() const { return m_size; }

private:
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexMemory;
	uint32_t m_size;
};
