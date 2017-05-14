#include "VulkanHardwareIndexBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareIndexBuffer::VulkanHardwareIndexBuffer(const VkPhysicalDevice * pGPU, const VkDevice & device, const void * pIndexData, uint32_t indexSize)
{
	VulkanMemoryBufferMgr::get()->createIndexBuffer(*pGPU, device, pIndexData, indexSize, m_indexBuffer, m_indexMemory);


}

VulkanHardwareIndexBuffer::~VulkanHardwareIndexBuffer()
{
}
