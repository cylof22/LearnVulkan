#pragma once
#include "common\vulkan_wrapper.h"
#include <vector>

class VulkanSwapChain;
class VulkanDevice;
class VulkanInstance;

class VulkanRenderer
{
public:
	VulkanRenderer(VulkanInstance* pInstance, VulkanDevice* pDevice);
	~VulkanRenderer();

	bool init(ANativeWindow* pWnd);

	bool createCmdPool();
	bool createSwapChain(ANativeWindow* pWnd);
	bool createDepthBuffer(ANativeWindow* pWnd);
	bool createRenderPass(bool includeDepth, bool clear = true);
	bool createFrameBuffer(ANativeWindow* pWnd, bool includeDepth, bool clear = true);
	void render();
private:
	VulkanSwapChain* m_pSwapChain;
	VulkanDevice* m_pGraphicDevice;
	VulkanInstance* m_pInstance;

	VkFence m_renderFence;
	VkCommandPool m_cmdPool;
	VkRenderPass m_renderPass;

	std::vector<VkFramebuffer> m_framebuffers;
	VkFormat m_depthFormat;
	VkImageView m_depthImageView;
};