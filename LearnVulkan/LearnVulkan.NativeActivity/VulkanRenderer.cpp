#include "VulkanRenderer.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanSwapChain.h"
#include "VulkanMemoryMgr.h"
#include "VkCommandBufferMgr.h"
#include "VulkanPipeline.h"
#include "VulkanPipelineState.h"
#include "VulkanGpuProgram.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanHardwareIndexBuffer.h"
#include "VulkanHardwareUniformBuffer.h"
#include "VulkanRenderable.h"
#include "VulkanDescriptorSetMgr.h"
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

VulkanRenderer::VulkanRenderer(VulkanInstance* pInstance, VulkanDevice * pDevice) : m_pGraphicDevice(pDevice), m_pInstance(pInstance),  m_pPipeline(nullptr)
	, m_pWnd(nullptr), m_activeCommandBufferId(0)
{
	m_pSwapChain = new VulkanSwapChain(m_pInstance, m_pGraphicDevice);
	m_pPipeline = new VulkanPipeline(pDevice);
	m_pPipeline->createPipeLineCache();

	// present semaphore
	VkSemaphoreCreateInfo presentSemaphoreInfo = {};
	presentSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	presentSemaphoreInfo.pNext = nullptr;
	presentSemaphoreInfo.flags = 0;

	vkCreateSemaphore(m_pGraphicDevice->getGraphicDevice(), &presentSemaphoreInfo, nullptr, &m_presentCompleteSemaphore);

	// draw semaphore
	VkSemaphoreCreateInfo drawSemaphoreInfo = {};
	drawSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	drawSemaphoreInfo.pNext = nullptr;
	drawSemaphoreInfo.flags = 0;

	vkCreateSemaphore(m_pGraphicDevice->getGraphicDevice(), &drawSemaphoreInfo, nullptr, &m_drawCompleteSemaphore);

	VkFenceCreateInfo uniformFenceInfo = {};
	uniformFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	uniformFenceInfo.pNext = 0;
	uniformFenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	vkCreateFence(m_pGraphicDevice->getGraphicDevice(), &uniformFenceInfo, nullptr, &m_cmdFence);
}

VulkanRenderer::~VulkanRenderer()
{
	if (m_renderPass != VK_NULL_HANDLE)
		vkDestroyRenderPass(m_pGraphicDevice->getGraphicDevice(), m_renderPass, nullptr);

	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
		vkDestroyFramebuffer(m_pGraphicDevice->getGraphicDevice(), m_framebuffers[i], nullptr);
	m_framebuffers.clear();

	vkDestroySemaphore(m_pGraphicDevice->getGraphicDevice(), m_presentCompleteSemaphore, nullptr);
	vkDestroySemaphore(m_pGraphicDevice->getGraphicDevice(), m_drawCompleteSemaphore, nullptr);
}

bool VulkanRenderer::init(ANativeWindow* pWnd)
{
	if (!pWnd)
		return false;
	m_pWnd = pWnd;

	bool isDepthBuffer = true;
	bool isOk = false;

	if (m_pSwapChain)
		isOk = createSwapChain(pWnd);

	if (isOk)
		isOk = createCmdPool();

	if (isOk)
		isOk = createDescriptorPool();

	if (isOk && isDepthBuffer)
		isOk = createDepthBuffer(pWnd);

	if (isOk)
		isOk = createRenderPass(isDepthBuffer);

	if(isOk)
		isOk = createFrameBuffer(pWnd, isDepthBuffer);

	if (isOk)
		isOk = createCommandBuffers(isDepthBuffer);
	
	return isOk;
}

bool VulkanRenderer::createCmdPool()
{
	if (m_pGraphicDevice)
	{
		VkCommandPoolCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext = nullptr;
		info.queueFamilyIndex = m_pGraphicDevice->getGraphicQueueFamilyIndex();
		info.flags = 0;

		VkResult res = vkCreateCommandPool(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &m_cmdPool);
		return res == VK_SUCCESS;
	}
	return false;
}

bool VulkanRenderer::createDescriptorPool()
{
	VkResult res = VK_RESULT_MAX_ENUM;

	VkDescriptorPoolSize descriptorPools[2];

	descriptorPools[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPools[0].descriptorCount = 1;

	descriptorPools[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPools[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	info.pNext = nullptr;
	info.maxSets = 1;
	info.poolSizeCount = 2;
	info.pPoolSizes = descriptorPools;

	res = vkCreateDescriptorPool(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &m_descriptorPool);
	return res == VK_SUCCESS;
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
	VkImageCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.format = m_depthFormat;
	info.tiling = depthTiling;
	info.extent.width = ANativeWindow_getWidth(pWnd);
	info.extent.height = ANativeWindow_getHeight(pWnd);
	info.extent.depth = 1;
	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	info.flags = 0;

	VkImage depthImage;
	res = vkCreateImage(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &depthImage);
	if (res != VK_SUCCESS)
		return false;

	//get the image memory requirement
	VkMemoryRequirements depthImageRequirement;
	vkGetImageMemoryRequirements(m_pGraphicDevice->getGraphicDevice(), depthImage, &depthImageRequirement);

	//allocate the device memory
	VkMemoryAllocateInfo allocateInfo = {};
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
	res = VkCommandBufferMgr::get()->createCommandBuffer(&m_pGraphicDevice->getGraphicDevice(), m_cmdPool, &depthImageLayoutCmdBuffer);
	res = VkCommandBufferMgr::get()->beginCommandBuffer(&depthImageLayoutCmdBuffer);
	{
		VulkanMemoryMgr::get()->imageLayoutConversion(depthImage, VK_IMAGE_ASPECT_DEPTH_BIT, 
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, (VkAccessFlagBits)0, depthImageLayoutCmdBuffer);
	}
	res = VkCommandBufferMgr::get()->endCommandBuffer(&depthImageLayoutCmdBuffer);
	res = VkCommandBufferMgr::get()->submitCommandBuffer(m_pGraphicDevice->getGraphicQueue(), &depthImageLayoutCmdBuffer, nullptr, m_renderFence);

	VkImageViewCreateInfo depthImageViewInfo = {};
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
	// Only for temporary: If not, the whole background buffer will be messy
	sleep(1);
	VkResult res = VK_SUCCESS;

	vkDeviceWaitIdle(m_pGraphicDevice->getGraphicDevice());

	uint32_t activeColorBufferId = m_activeCommandBufferId;

	const VkSwapchainKHR* pSwapChain = m_pSwapChain->getSwapChain();

	VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentCompleteSemaphore;
	submitInfo.pWaitDstStageMask = &submitPipelineStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_cmdDraws[activeColorBufferId];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_drawCompleteSemaphore;

	// queue the command buffer for execution
	VkCommandBufferMgr::get()->submitCommandBuffer(m_pGraphicDevice->getGraphicQueue(), &m_cmdDraws[activeColorBufferId], &submitInfo, m_cmdFence);

	// present the image in the window
	VkPresentInfoKHR present = {};
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pNext = nullptr;
	present.swapchainCount = 1;
	present.pSwapchains = pSwapChain;
	present.pImageIndices = &activeColorBufferId;
	present.waitSemaphoreCount = 1;
	present.pWaitSemaphores = &m_drawCompleteSemaphore;
	present.pResults = nullptr;

	res = vkQueuePresentKHR(m_pGraphicDevice->getGraphicQueue(), &present);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		assert(false);
		reInit();
	}
	else
	{

	}
	assert(res == VK_SUCCESS);
}

void VulkanRenderer::addRenderable(VulkanRenderable * renderEntity, const VkPipeline & pipeline, const VkPipelineLayout& layout)
{
	m_renderEntityInfo.emplace_back(std::make_tuple(renderEntity, pipeline, layout));
}

bool VulkanRenderer::reInit()
{
	vkDeviceWaitIdle(m_pGraphicDevice->getGraphicDevice());

	delete m_pSwapChain;
	m_pSwapChain = new VulkanSwapChain(m_pInstance, m_pGraphicDevice);

	init(m_pWnd);
	return false;
}

bool VulkanRenderer::createRenderPass(bool includeDepth, bool clear /*=true*/)
{
	VkResult res = VK_SUCCESS;
	/* Need attachments for render target and depth buffer */
	VkAttachmentDescription attachments[2];
	attachments[0].format = m_pSwapChain->getColorFormat();
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachments[0].flags = 0;

	if (includeDepth) 
	{
		attachments[1].format = m_depthFormat;
		attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
		attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		attachments[1].flags = 0;
	}

	VkAttachmentReference color_reference = {};
	color_reference.attachment = 0;
	color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depth_reference = {};
	depth_reference.attachment = 1;
	depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_reference;
	subpass.pResolveAttachments = nullptr;
	subpass.pDepthStencilAttachment = includeDepth ? &depth_reference : nullptr;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;

	VkRenderPassCreateInfo rp_info = {};
	rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	rp_info.pNext = nullptr;
	rp_info.attachmentCount = includeDepth ? 2 : 1;
	rp_info.pAttachments = attachments;
	rp_info.subpassCount = 1;
	rp_info.pSubpasses = &subpass;
	rp_info.dependencyCount = 0;
	rp_info.pDependencies = nullptr;

	res = vkCreateRenderPass(m_pGraphicDevice->getGraphicDevice(), &rp_info, NULL, &m_renderPass);
	assert(res == VK_SUCCESS);
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
	info.layers = 1;

	m_framebuffers.clear();
	m_framebuffers.resize(m_pSwapChain->getColorBuffers().size());

	for (uint32_t i = 0; i < m_pSwapChain->getColorBuffers().size(); i++)
	{
		imageViews[0] = (m_pSwapChain->getColorBuffers())[i].view;
		res = vkCreateFramebuffer(m_pGraphicDevice->getGraphicDevice(), &info, nullptr, &(m_framebuffers[i]));
		assert(res == VK_SUCCESS);
	}

	return res == VK_SUCCESS;
}

bool VulkanRenderer::createCommandBuffers(bool includeDepth)
{
	if (m_framebuffers.empty())
		return false;

	VkResult res = VK_SUCCESS;
	m_cmdDraws.resize(m_framebuffers.size());

	for (uint32_t i = 0; i < m_cmdDraws.size(); i++)
	{
		VkCommandBuffer cmdBuffer;
		res = VkCommandBufferMgr::get()->createCommandBuffer(&m_pGraphicDevice->getGraphicDevice(), m_cmdPool, &cmdBuffer);
		m_cmdDraws[i] = cmdBuffer;
	}
	return true;
}

inline glm::mat4 vulkanStyleProjection(const glm::mat4 &proj)
{
	// Flip Y in clipspace. X = -1, Y = -1 is topLeft in Vulkan.
	auto mat = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, -1.0f, 1.0f));

	// Z depth is [0, 1] range instead of [-1, 1].
	mat = glm::scale(mat, glm::vec3(1.0f, 1.0f, 0.5f));
	return glm::translate(mat, glm::vec3(0.0f, 0.0f, 1.0f)) * proj;
}

void VulkanRenderer::update(bool includeDepth)
{
	static glm::mat4 mvp = glm::mat4(1.0f);
	static float transf = 0.0f;
	static float rot = 0.0f;
	static float transfSign = 1.0;
	transf += transfSign * 0.5;

	if (transf >= 5.0f)
		transfSign = -1.0f;
	if (transf <= -5.0f)
		transfSign = 1.0f;

	float aspect = ANativeWindow_getWidth(m_pWnd) / ANativeWindow_getHeight(m_pWnd);

	//glm::mat4 projection = glm::perspective(glm::radians(45.f), 1.0f, 0.1f, 100.f);
	glm::mat4 projection = glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, -2.0f, 2.0f);

	projection = vulkanStyleProjection(projection);

	glm::mat4 view = glm::mat4(1.0f);/* glm::lookAt(glm::vec3(0.f, 0.f, 5.f),
		glm::vec3(0.f, 0.f, 0.f),
		glm::vec3(0.f, -1.f, 0.f));*/

	glm::mat4 model = glm::mat4(1.0f); 

	rot += 10.f;

	model = glm::rotate(model, rot, glm::vec3(0.0f, 1.0f, 0.0f)) 
		* glm::rotate(model, rot, glm::vec3(1.0f, 1.0f, 1.0f));

	mvp = projection * view * model;

	VkResult res = VK_RESULT_MAX_ENUM;

	vkWaitForFences(m_pGraphicDevice->getGraphicDevice(), 1, &m_cmdFence, VK_TRUE, INT64_MAX);
	vkResetFences(m_pGraphicDevice->getGraphicDevice(), 1, &m_cmdFence);

	const VkSwapchainKHR* pSwapChain = m_pSwapChain->getSwapChain();
	res = vkAcquireNextImageKHR(m_pGraphicDevice->getGraphicDevice(), *pSwapChain, UINT64_MAX, m_presentCompleteSemaphore, VK_NULL_HANDLE, &m_activeCommandBufferId);

	VkClearValue clearValues[2];
	switch (m_activeCommandBufferId)
	{
	case 0:
		clearValues[0].color.float32[0] = 1.0f;
		clearValues[0].color.float32[1] = 0.0f;
		clearValues[0].color.float32[2] = 0.0f;
		clearValues[0].color.float32[3] = 1.0f;
		break;
	case 1:
		clearValues[0].color.float32[0] = 0.0f;
		clearValues[0].color.float32[1] = 1.0f;
		clearValues[0].color.float32[2] = 0.0f;
		clearValues[0].color.float32[3] = 1.0f;
		break;
	case 2:
		clearValues[0].color.float32[0] = 0.0f;
		clearValues[0].color.float32[1] = 0.0f;
		clearValues[0].color.float32[2] = 1.0f;
		clearValues[0].color.float32[3] = 1.0f;
		break;
	default:
		clearValues[0].color.float32[0] = 0.0f;
		clearValues[0].color.float32[1] = 1.0f;
		clearValues[0].color.float32[2] = 0.0f;
		clearValues[0].color.float32[3] = 0.0f;
	}

	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = nullptr;
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.framebuffer = m_framebuffers[m_activeCommandBufferId];
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = ANativeWindow_getWidth(m_pWnd);
	renderPassBeginInfo.renderArea.extent.height = ANativeWindow_getHeight(m_pWnd);
	renderPassBeginInfo.clearValueCount = includeDepth ? 2 : 1;
	renderPassBeginInfo.pClearValues = clearValues;

	VkCommandBuffer& activeCmdBuffer = m_cmdDraws[m_activeCommandBufferId];
	
	res = VkCommandBufferMgr::get()->beginCommandBuffer(&activeCmdBuffer);

	vkCmdBeginRenderPass(activeCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	// record the static renderable
	for (auto rRenderInfo : m_renderEntityInfo)
	{
		//std::get<0>(rRenderInfo)->getUniformBuffer()->updateUniformBuffer(activeCmdBuffer, &mvp, sizeof(mvp));
		vkCmdPushConstants(activeCmdBuffer, std::get<2>(rRenderInfo), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &mvp);

		vkCmdBindPipeline(activeCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, std::get<1>(rRenderInfo));

		// create the bind vertex buffer
		VkDeviceSize offset[1] = { 0 };
		vkCmdBindVertexBuffers(activeCmdBuffer, 0, 1, &(std::get<0>(rRenderInfo)->getVertexBuffer()->getVertexBuffer()), offset);
		if(std::get<0>(rRenderInfo)->getIndexBuffer())
			vkCmdBindIndexBuffer(activeCmdBuffer, std::get<0>(rRenderInfo)->getIndexBuffer()->getBuffer(), *offset, VK_INDEX_TYPE_UINT16);

		vkCmdBindDescriptorSets(activeCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, std::get<2>(rRenderInfo), 0, 1, std::get<0>(rRenderInfo)->getDescriptorSet(), 0, nullptr);

		// draw the buffer
		if (std::get<0>(rRenderInfo)->getIndexBuffer())
		{
			vkCmdDrawIndexed(activeCmdBuffer, std::get<0>(rRenderInfo)->getIndexBuffer()->size(), 1, 0, 0, 0);
		}
		else
		{
			vkCmdDraw(activeCmdBuffer, std::get<0>(rRenderInfo)->getVertexBuffer()->size(), 1, 0, 0);
		}
	}

	vkCmdEndRenderPass(activeCmdBuffer);

	res = VkCommandBufferMgr::get()->endCommandBuffer(&activeCmdBuffer);
}
