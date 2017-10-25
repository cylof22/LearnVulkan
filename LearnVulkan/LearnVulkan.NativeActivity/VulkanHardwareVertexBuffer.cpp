#include "VulkanHardwareVertexBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareVertexBuffer::VulkanHardwareVertexBuffer(VulkanGraphicContext* pContext, const void * pVertexData, uint32_t vertexSize, uint32_t stride, 
	std::vector<std::pair<VkFormat, VkDeviceSize>> descriptions, uint32_t binding)
	: VulkanHardwareBuffer(pContext, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pVertexData, vertexSize)
{
	m_vertexBinding.binding = binding;
	m_vertexBinding.stride = stride;
	m_vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	for (unsigned int i = 0; i < descriptions.size(); ++i)
	{
		VkVertexInputAttributeDescription attributeDescription;
		attributeDescription.binding = binding;
		attributeDescription.location = i;
		attributeDescription.format = descriptions[i].first;
		attributeDescription.offset = descriptions[i].second;

		m_vertexAttributes.emplace_back(attributeDescription);
	}

	m_size = vertexSize / stride;
}

VulkanHardwareVertexBuffer::~VulkanHardwareVertexBuffer()
{
}
