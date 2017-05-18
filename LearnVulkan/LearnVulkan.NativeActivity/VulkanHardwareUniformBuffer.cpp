#include "VulkanHardwareUniformBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareUniformBuffer::VulkanHardwareUniformBuffer(const VkPhysicalDevice * pGPU, const VkDevice & device, const void * pUniformData, uint32_t uniformSize)
{
	VulkanMemoryBufferMgr::get()->createUniformBuffer(*pGPU, device, pUniformData, uniformSize, m_uniformBuffer, m_uniformMemory);

	// Update the local data structure with uniform buffer for house keeping 
	m_uniformBufferInfo.buffer = m_uniformBuffer;
	m_uniformBufferInfo.offset = 0;
	m_uniformBufferInfo.range = uniformSize;
}

VulkanHardwareUniformBuffer::~VulkanHardwareUniformBuffer()
{
}
