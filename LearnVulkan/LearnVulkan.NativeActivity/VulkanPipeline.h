#pragma once
#include "common\vulkan_wrapper.h"

class VulkanApplication;
class VulkanDevice;
class VulkanRenderable;
class VulkanPipelineState;

class VulkanPipeline
{
public:
	VulkanPipeline(VulkanApplication* pApp, VulkanDevice* pDevice);
	~VulkanPipeline();

	VkResult createPipeLineCache();

	bool createGraphicPipeline(const VulkanRenderable* pRenderable, const VulkanPipelineState& pipelineState, VkPipeline& pipeline);
	bool createComputePipeline();
	void destroyPipeline(const VkPipeline& pipeline);
	void destroyPipelineCache(const VkPipelineCache& pipelineCache);

private:
	VkPipelineCache m_pipelineCache;

	VulkanApplication* m_pApp;
	VulkanDevice* m_pDevice;
};