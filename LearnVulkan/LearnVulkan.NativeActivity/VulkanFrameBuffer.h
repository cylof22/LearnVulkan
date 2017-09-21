#pragma once
#include <vector>
#include "common\vulkan_wrapper.h"
#include "VulkanRenderPass.h"

class VulkanGraphicContext;

class VulkanFrameBuffer
{
public:
	VulkanFrameBuffer(VulkanGraphicContext* pContext);
	~VulkanFrameBuffer();

	const VulkanRenderPass& getAttachedRenderPass() const { return m_vulkanRenderPass; }
	const VkFramebuffer& getFrameBuffer() const { return m_frameBuffer; }

	bool isDepthEnable() const { return m_isDepth; }
	bool isStencilEnable() const { return m_isStencil; }

	void colorBufferSize(uint32_t colorSize);
	void depthStencilSize(uint32_t depthSize);

	VkImage* getColorBuffer();
	VkImageView* getColorBufferView();

	VkImage* getDepthStencilBuffer();
	VkDeviceMemory* getDepthStencilMemory();
	VkImageView* getDepthStencilBufferView();

	const VkFormat getColorFormat() const;
	void setColorFormat(VkFormat colorFormat);

	const VkFormat getDepthFormat() const;
	void setDepthFormat(VkFormat depthFormat);

private:
	VulkanRenderPass m_vulkanRenderPass;
	VkFramebuffer m_frameBuffer;

	bool m_isDepth;
	bool m_isStencil;

	// color format
	VkFormat m_colorFormat;

	// depth format
	VkFormat m_depthFormat;

	// color buffer
	std::vector<VkImage> m_colorBuffer;
	std::vector<VkImageView> m_colorBufferView;

	// depth buffer
	std::vector<VkImage> m_depthStencilBuffer;
	std::vector<VkDeviceMemory> m_depthStencilMemory;
	std::vector<VkImageView> m_depthStencilBufferView;
};

