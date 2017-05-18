#pragma once
#include "common\vulkan_wrapper.h"
#include <memory>
#include <vector>
#include <queue>
#include <tuple>

class VulkanSwapChain;
class VulkanDevice;
class VulkanInstance;
class VulkanRenderable;
class VulkanPipeline;

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanInstance* pInstance, VulkanDevice* pDevice);
	~VulkanRenderer();

	bool init(ANativeWindow* pWnd);

	bool createCmdPool();
	bool createDescriptorPool();
	bool createSwapChain(ANativeWindow* pWnd);
	bool createDepthBuffer(ANativeWindow* pWnd);
	bool createRenderPass(bool includeDepth, bool clear = true);
	bool createFrameBuffer(ANativeWindow* pWnd, bool includeDepth, bool clear = true);
	bool createCommandBuffers(bool includeDepth);
	void render();

	void addRenderable(VulkanRenderable* renderEntity, const VkPipeline& pipeline, const VkPipelineLayout& layout);

protected:
	bool reInit();
private:
	VulkanSwapChain* m_pSwapChain;
	VulkanDevice* m_pGraphicDevice;
	VulkanInstance* m_pInstance;
	VulkanPipeline* m_pPipeline;

	VkFence m_renderFence;
	VkCommandPool m_cmdPool;
	VkRenderPass m_renderPass;

	VkDescriptorPool m_descriptorPool;

	std::vector<VkFramebuffer> m_framebuffers;
	std::vector<VkCommandBuffer> m_cmdDraws;

	VkFormat m_depthFormat;
	VkImageView m_depthImageView;

	ANativeWindow* m_pWnd;

	VkSemaphore m_presentCompleteSemaphore;
	VkSemaphore m_drawCompleteSemaphore;

	std::vector<std::tuple<VulkanRenderable*, VkPipeline, VkPipelineLayout>> m_renderEntityInfo;
};