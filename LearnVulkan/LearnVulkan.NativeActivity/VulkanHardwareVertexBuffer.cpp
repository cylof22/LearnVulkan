#include "VulkanHardwareVertexBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareVertexBuffer::VulkanHardwareVertexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void * pVertexData, uint32_t vertexSize, uint32_t stride)
	: m_vertexBuffer(VK_NULL_HANDLE), m_vertexMemory(VK_NULL_HANDLE)
{
	VulkanMemoryBufferMgr::get()->createVertexBuffer(pGPU, device, pVertexData, vertexSize, stride, m_vertexBuffer, m_vertexMemory);
	m_vertexBinding.binding = 0;
	m_vertexBinding.stride = stride;
	m_vertexBinding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	m_vertexAttributes.resize(2);
	
	m_vertexAttributes[0].binding = 0;
	m_vertexAttributes[0].location = 0;
	m_vertexAttributes[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	m_vertexAttributes[0].offset = 0;

	m_vertexAttributes[1].binding = 0;
	m_vertexAttributes[1].location = 1;
	m_vertexAttributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	m_vertexAttributes[1].offset = 16;

	m_size = vertexSize / stride;
}

VulkanHardwareVertexBuffer::~VulkanHardwareVertexBuffer()
{
}
