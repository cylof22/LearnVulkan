#include "VulkanHardwareUniformBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareUniformBuffer::VulkanHardwareUniformBuffer(const VkPhysicalDevice * pGPU, const VkDevice & device, const void * pUniformData, uint32_t uniformSize)
{
	VulkanMemoryBufferMgr::get()->createUniformBuffer(*pGPU, device, pUniformData, uniformSize, m_uniformBuffer, m_uniformMemory);

	// Update the local data structure with uniform buffer for house keeping 
	m_uniformBufferInfo.buffer = m_uniformBuffer;
	m_uniformBufferInfo.offset = 0;
	m_uniformBufferInfo.range = uniformSize;

	m_GraphicDevice = device;
}

VulkanHardwareUniformBuffer::~VulkanHardwareUniformBuffer()
{
}

bool VulkanHardwareUniformBuffer::updateUniformBuffer(const VkCommandBuffer& cmdBuffer, const void* pUniformData, uint32_t uniformSize)
{
	//VkDeviceSize offset = { 0 };
	//vkCmdUpdateBuffer(cmdBuffer, m_uniformBuffer, offset, uniformSize, pUniformData);
	VulkanMemoryBufferMgr::get()->updateUniformBuffer(m_GraphicDevice, cmdBuffer, pUniformData, uniformSize, m_uniformBuffer, m_uniformMemory);
	return true;
}
