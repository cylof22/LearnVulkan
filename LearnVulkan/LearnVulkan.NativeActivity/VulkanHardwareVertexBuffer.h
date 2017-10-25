#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>
#include "VulkanHardwareBuffer.h"

class VulkanGraphicContext;

class VulkanHardwareVertexBuffer: public VulkanHardwareBuffer
{
public:
	VulkanHardwareVertexBuffer(VulkanGraphicContext* pContext, const void * pVertexData, uint32_t vertexSize, uint32_t stride,
		std::vector<std::pair<VkFormat, VkDeviceSize>> descriptions, uint32_t binding = 0);

	virtual ~VulkanHardwareVertexBuffer();

	const VkVertexInputBindingDescription& getVertexInputBinding() const { return m_vertexBinding; }
	const std::vector<VkVertexInputAttributeDescription>& getVertexInputAttributes() const { return m_vertexAttributes; }

private:

	VkVertexInputBindingDescription m_vertexBinding;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;
};
