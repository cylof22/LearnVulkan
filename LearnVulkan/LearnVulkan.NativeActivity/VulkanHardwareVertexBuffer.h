#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>

class VulkanHardwareVertexBuffer
{
public:
	VulkanHardwareVertexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pVertexData, uint32_t vertexSize, uint32_t stride,
		std::vector<std::pair<VkFormat, VkDeviceSize>> descriptions, uint32_t binding = 0);
	~VulkanHardwareVertexBuffer();

	VkVertexInputBindingDescription& getVertexInputBinding() { return m_vertexBinding; }
	const std::vector<VkVertexInputAttributeDescription>& getVertexInputAttributes() { return m_vertexAttributes; }
	const VkBuffer& getVertexBuffer() { return m_vertexBuffer; }

	const uint32_t size() const { return m_size; };
private:
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexMemory;

	VkVertexInputBindingDescription m_vertexBinding;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
	uint32_t m_size;
};
