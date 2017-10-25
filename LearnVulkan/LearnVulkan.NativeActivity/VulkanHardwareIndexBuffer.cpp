#include "VulkanGraphicContext.h"
#include "VulkanHardwareIndexBuffer.h"
#include "VulkanMemoryBufferMgr.h"

VulkanHardwareIndexBuffer::VulkanHardwareIndexBuffer(VulkanGraphicContext * pContext, VkIndexType indexType, const void * pIndexData, uint32_t indexSize)
	: VulkanHardwareBuffer(pContext, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, (VkMemoryPropertyFlagBits)(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
		pIndexData, indexSize)
	, m_type(indexType)
{
}

VulkanHardwareIndexBuffer::~VulkanHardwareIndexBuffer()
{
}

const uint32_t VulkanHardwareIndexBuffer::getIndexSize() const
{
	uint32_t indexSize = 0;
	switch (m_type)
	{
	case VK_INDEX_TYPE_UINT16:
		indexSize = m_size / sizeof(uint16_t);
		break;
	case VK_INDEX_TYPE_UINT32:
		indexSize = m_size / sizeof(uint32_t);
		break;
	default:
		assert(0);
	}
	return indexSize;
}
