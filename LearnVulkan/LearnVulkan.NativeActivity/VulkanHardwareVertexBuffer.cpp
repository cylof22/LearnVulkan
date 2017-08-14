#include "VulkanHardwareVertexBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareVertexBuffer::VulkanHardwareVertexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pVertexData, uint32_t vertexSize, uint32_t stride,
	std::vector<std::pair<VkFormat, VkDeviceSize>> descriptions, uint32_t binding /*= 0*/)
	: m_vertexBuffer(VK_NULL_HANDLE), m_vertexMemory(VK_NULL_HANDLE)
{
	VulkanMemoryBufferMgr::get()->createVertexBuffer(pGPU, device, pVertexData, vertexSize, stride, m_vertexBuffer, m_vertexMemory);
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
