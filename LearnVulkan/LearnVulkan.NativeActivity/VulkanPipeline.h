#pragma once
#include "common\vulkan_wrapper.h"

class VulkanDevice;
class VulkanRenderable;
struct VulkanGraphicPipelineState;

class VulkanPipeline
{
public:
	VulkanPipeline(VulkanDevice* pDevice);
	~VulkanPipeline();

	VkResult createPipeLineCache();

	bool createGraphicPipeline(ANativeWindow* pWnd, const VulkanRenderable* pRenderable, const VulkanGraphicPipelineState& pipelineState, VkPipeline& pipeline);
	bool createComputePipeline();
	void destroyPipeline(const VkPipeline& pipeline);
	void destroyPipelineCache(const VkPipelineCache& pipelineCache);

private:
	VkPipelineCache m_pipelineCache;

	VulkanDevice* m_pDevice;
};