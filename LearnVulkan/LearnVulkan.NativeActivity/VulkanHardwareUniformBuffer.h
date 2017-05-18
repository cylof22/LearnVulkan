#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>

class VulkanHardwareUniformBuffer
{
public:
	VulkanHardwareUniformBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pUniformData, uint32_t uniformSize);
	~VulkanHardwareUniformBuffer();

private:
	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformMemory;
	VkDescriptorBufferInfo m_uniformBufferInfo;
};
