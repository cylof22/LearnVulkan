#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>
#include <glm\glm.hpp>
#include "VulkanHardwareBuffer.h"

class VulkanHardwareUniformBuffer : public VulkanHardwareBuffer
{
public:
	VulkanHardwareUniformBuffer(VulkanGraphicContext* pContext, const void * pUniformData, uint32_t uniformSize);
	~VulkanHardwareUniformBuffer();

	VkDescriptorBufferInfo& getDesriptorInfo() { return m_uniformBufferInfo; }

private:
	VkDescriptorBufferInfo m_uniformBufferInfo;
};
