#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>
#include <glm\glm.hpp>

class VulkanHardwareUniformBuffer
{
public:
	VulkanHardwareUniformBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pUniformData, uint32_t uniformSize);
	~VulkanHardwareUniformBuffer();

	VkDescriptorBufferInfo& getBuferDesriptorInfo() { return m_uniformBufferInfo; }

	bool updateUniformBuffer(const VkCommandBuffer& cmdBuffer, const void* pUniformData, uint32_t uniformSize);

	VkDeviceMemory getMemory() { return m_uniformMemory; }

private:
	VkBuffer m_uniformBuffer;
	VkDeviceMemory m_uniformMemory;
	VkDescriptorBufferInfo m_uniformBufferInfo;

	VkDevice m_GraphicDevice;
};
