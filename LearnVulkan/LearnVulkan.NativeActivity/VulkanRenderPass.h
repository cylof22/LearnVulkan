#pragma once
#include "common\vulkan_wrapper.h"

class VulkanRenderPass
{
public:
	VulkanRenderPass();
	~VulkanRenderPass();

	const VkRenderPass& getRenderPass() const { return m_renderPass; }

private:
	VkRenderPass m_renderPass;
};

