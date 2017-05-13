#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>

class VulkanHardwareVertexBuffer
{
public:
	VulkanHardwareVertexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pVertexData, uint32_t vertexSize, uint32_t stride);
	~VulkanHardwareVertexBuffer();

	VkVertexInputBindingDescription& getVertexInputBinding() { return m_vertexBinding; }
	const std::vector<VkVertexInputAttributeDescription>& getVertexInputAttributes() { return m_vertexAttributes; }
	const VkBuffer& getVertexBuffer() { return m_vertexBuffer; }

private:
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexMemory;

	VkVertexInputBindingDescription m_vertexBinding;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
};
