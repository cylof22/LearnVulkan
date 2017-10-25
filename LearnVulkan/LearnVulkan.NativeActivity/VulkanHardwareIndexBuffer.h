#pragma once
#include "common\vulkan_wrapper.h"
#include "VulkanHardwareBuffer.h"

class VulkanGraphicContext;

class VulkanHardwareIndexBuffer : public VulkanHardwareBuffer
{
public:
	VulkanHardwareIndexBuffer(VulkanGraphicContext* pContext, VkIndexType indexType, const void * pIndexData, uint32_t indexSize);
	virtual ~VulkanHardwareIndexBuffer();

	const uint32_t getIndexSize() const;

	VkIndexType getType() const { return m_type; }

private:
	uint32_t m_size;
	VkIndexType m_type;
};
