#include "VulkanHardwareUniformBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareUniformBuffer::VulkanHardwareUniformBuffer(VulkanGraphicContext * pContext, const void * pUniformData, uint32_t uniformSize)
	: VulkanHardwareBuffer(pContext, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, pUniformData, uniformSize)
{
	// Update the local data structure with uniform buffer for house keeping 
	m_uniformBufferInfo.buffer = m_buffer;
	m_uniformBufferInfo.offset = 0;
	m_uniformBufferInfo.range = uniformSize;
}

VulkanHardwareUniformBuffer::~VulkanHardwareUniformBuffer()
{
}