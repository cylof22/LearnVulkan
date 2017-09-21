#pragma once
#include "common\vulkan_wrapper.h"

class VulkanHardwareBuffer
{
public:
	VulkanHardwareBuffer();
	~VulkanHardwareBuffer();

protected:
	VkBuffer m_buffer;
};

