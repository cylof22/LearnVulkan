#include "VulkanRenderer.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VkMemoryMgr.h"
#include "VkCommandBufferMgr.h"

VulkanRenderer::VulkanRenderer(VulkanInstance* pInstance, VulkanDevice * pDevice) : m_pGraphicDevice(pDevice), m_pInstance(pInstance)
{
	
}

VulkanRenderer::~VulkanRenderer()
{
	if (m_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(m_pGraphicDevice->getGraphicDevice(), m_renderPass, nullptr);

	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		vkDestroyFramebuffer(m_pGraphicDevice->getGraphicDevice(), m_framebuffers[i], nullptr);
	m_framebuffers.clear();
}

bool VulkanRenderer::init(ANativeWindow* pWnd)
{
	if (!pWnd)
		return false;

	bool isOk = false;

	m_pSwapChain = new VulkanSwapChain(m_pInstance, m_pGraphicDevice);

	if (m_pSwapChain)
		isOk = createSwapChain(pWnd);

	if (isOk)
		isOk = createCmdPool();

	if (isOk)
		isOk = createDepthBuffer(pWnd);

	if (isOk)
		createRenderPass(true);

	if(isOk)
		createFrameBuffer(pWnd, true);

	return isOk;
}

bool VulkanRenderer::createCmdPool()
{
	if (m_pGraphicDevice)
	{
		VkCommandPoolCreateInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = m_pGraphicDevice->getGraphicQueueFamilyIndex();

		VkResult res = vkCreateCommandPool(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &m_cmdPool);
		return res == VK_SUCCESS;
	}
	return false;
}

bool VulkanRenderer::createSwapChain(ANativeWindow* pWnd)
{
	bool isOk = false;
	if (m_pSwapChain)
		isOk = m_pSwapChain->init(pWnd);

	return isOk;
}

bool VulkanRenderer::createDepthBuffer(ANativeWindow* pWnd)
{
	VkResult res = VK_SUCCESS;

	m_depthFormat = VK_FORMAT_D16_UNORM;
	VkImageTiling depthTiling = VK_IMAGE_TILING_OPTIMAL;
	VkFormatProperties props;
	vkGetPhysicalDeviceFormatProperties(*m_pGraphicDevice->getGPU(), m_depthFormat, &props);
	if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		depthTiling = VK_IMAGE_TILING_OPTIMAL;
	}
	else if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		depthTiling = VK_IMAGE_TILING_LINEAR;
	}

	// create the depth image
	VkImageCreateInfo info;
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.format = m_depthFormat;
	info.tiling = depthTiling;
	info.extent.width = ANativeWindow_getWidth(pWnd);
	info.extent.height = ANativeWindow_getHeight(pWnd);
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	info.flags = 0;

	VkImage depthImage;
	res = vkCreateImage(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &depthImage);
	if (res != VK_SUCCESS)
		return false;

	//get the image memory requirement
	VkMemoryRequirements depthImageRequirement;
	vkGetImageMemoryRequirements(m_pGraphicDevice->getGraphicDevice(), depthImage, &depthImageRequirement);

	//allocate the device memory
	VkMemoryAllocateInfo allocateInfo;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = depthImageRequirement.size;
	allocateInfo.memoryTypeIndex = 0;

	bool isOk = VulkanMemoryMgr::get()->memoryTypeFromProperties(m_pGraphicDevice->getGPU(), 
		depthImageRequirement.memoryTypeBits, 0, allocateInfo.memoryTypeIndex);

	if (isOk)
	{
		VkDeviceMemory depthMemory;
		res = vkAllocateMemory(m_pGraphicDevice->getGraphicDevice(), &allocateInfo, nullptr, &depthMemory);
		if (res == VK_SUCCESS)
		{
			res = vkBindImageMemory(m_pGraphicDevice->getGraphicDevice(), depthImage, depthMemory, 0);
		}
	}

	// create the depth image view
	VkCommandBuffer depthImageLayoutCmdBuffer;
	VkCommandBufferMgr::get()->createCommandBuffer(&m_pGraphicDevice->getGraphicDevice(), m_cmdPool, &depthImageLayoutCmdBuffer, nullptr);
	VkCommandBufferMgr::get()->beginCommandBuffer(&depthImageLayoutCmdBuffer, nullptr);
	{
		VulkanMemoryMgr::get()->imageLayoutConversion(depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, depthImageLayoutCmdBuffer);
	}
	VkCommandBufferMgr::get()->endCommandBuffer(&depthImageLayoutCmdBuffer);
	VkCommandBufferMgr::get()->submitCommandBuffer(m_pGraphicDevice->getGraphicQueue(), &depthImageLayoutCmdBuffer, nullptr, m_renderFence);

	VkImageViewCreateInfo depthImageViewInfo;
	depthImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthImageViewInfo.pNext = nullptr;
	depthImageViewInfo.image = depthImage;
	depthImageViewInfo.format = m_depthFormat;
	depthImageViewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY };
	depthImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthImageViewInfo.subresourceRange.baseMipLevel = 0;
	depthImageViewInfo.subresourceRange.levelCount = 1;
	depthImageViewInfo.subresourceRange.baseArrayLayer = 0;
	depthImageViewInfo.subresourceRange.layerCount = 1;
	depthImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthImageViewInfo.flags = 0;

	if (m_depthFormat == VK_FORMAT_D16_UNORM_S8_UINT ||
		m_depthFormat == VK_FORMAT_D24_UNORM_S8_UINT ||
		m_depthFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
		depthImageViewInfo.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;

	res = vkCreateImageView(m_pGraphicDevice->getGraphicDevice(), &depthImageViewInfo, nullptr, &m_depthImageView);

	return isOk && res == VK_SUCCESS;
}

// draw every frame
void VulkanRenderer::render()
{
}

bool VulkanRenderer::createRenderPass(bool includeDepth, bool clear /*=true*/)
{
	VkResult res = VK_SUCCESS;

	VkAttachmentDescription passAttachments[2];

	// specify the color attachment
	passAttachments[0].format = m_pSwapChain->getColorFormat();
	passAttachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	passAttachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	passAttachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	passAttachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	passAttachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	passAttachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	passAttachments[0].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;

	// specify the depth attachment
	if (includeDepth)
	{
		passAttachments[1].format = m_depthFormat;
		passAttachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		passAttachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		passAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		passAttachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		passAttachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		passAttachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		passAttachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		passAttachments[1].flags = VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	}

	// define the color buffer attachment binding point
	VkAttachmentReference colorReference;
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference;
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subPassDesc = {};
	subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subPassDesc.flags = 0;
	subPassDesc.inputAttachmentCount = 0;
	subPassDesc.pInputAttachments = nullptr;
	subPassDesc.colorAttachmentCount = 1;
	subPassDesc.pColorAttachments = &colorReference;
	subPassDesc.preserveAttachmentCount = 0;
	subPassDesc.pResolveAttachments = nullptr;
	subPassDesc.pDepthStencilAttachment = includeDepth ? &depthReference : nullptr;
	subPassDesc.preserveAttachmentCount = 0;
	subPassDesc.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	info.pNext = nullptr;
	info.attachmentCount = includeDepth ? 2 : 1;
	info.pAttachments = passAttachments;
	info.subpassCount = 1;
	info.pSubpasses = &subPassDesc;
	info.dependencyCount = 0;
	info.pDependencies = nullptr;

	res = vkCreateRenderPass(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &m_renderPass);

	return res == VK_SUCCESS;
}

bool VulkanRenderer::createFrameBuffer(ANativeWindow* pWnd, bool includeDepth, bool clear)
{
	VkResult res = VK_SUCCESS;
	VkImageView imageViews[2];

	// set the depth image view
	imageViews[1] = m_depthImageView;

	VkFramebufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.renderPass = m_renderPass;
	info.attachmentCount = includeDepth ? 2 : 1;
	info.pAttachments = imageViews;
	info.width = ANativeWindow_getWidth(pWnd);
	info.height = ANativeWindow_getHeight(pWnd);
	info.layers = 0;

	m_framebuffers.clear();
	m_framebuffers.resize(m_pSwapChain->getColorBuffers().size());

	for (uint32_t i = 0; i < m_pSwapChain->getColorBuffers().size(); i++)
	{
		imageViews[0] = (m_pSwapChain->getColorBuffers())[i].view;
		res = vkCreateFramebuffer(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &(m_framebuffers[i]));
		assert(res == VK_SUCCESS);
	}

	return res = VK_SUCCESS;
}
