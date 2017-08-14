#include "common\vulkan_wrapper.h"
#include "VulkanPipeline.h"
#include "VulkanApplication.h"
#include "VulkanDevice.h"
#include "VulkanRenderable.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanPipelineState.h"
#include <glm\glm.hpp>

VulkanPipeline::VulkanPipeline(VulkanDevice * pDevice) : m_pDevice(pDevice), mDynamicStateSize(0)
{
}

VulkanPipeline::~VulkanPipeline()
{
}

VkResult VulkanPipeline::createPipeLineCache()
{
	VkResult res;
	VkPipelineCacheCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	info.pNext = nullptr;
	info.initialDataSize = 0;
	info.pInitialData = nullptr;
	info.flags = 0;
	res = vkCreatePipelineCache(m_pDevice->getGraphicDevice(), &info, VK_ALLOC_CALLBACK, &m_pipelineCache);
	assert(res == VK_SUCCESS);
	return res;
}

bool VulkanPipeline::createGraphicPipeline(ANativeWindow* pWnd, const VulkanRenderable* pRenderable, const VulkanGraphicPipelineState& pipelineState, 
	VkPipeline& pipeline, VkPipelineLayout& layout)
{
	VkResult res;

	// add viewport and scissor state
	mDynamicStateInfo.dynamicStateCount = mDynamicStateSize;
	mDynamicStateInfo.pDynamicStates = mDynamicStates.data();
	
	// vertex input state info
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &(pRenderable->getVertexBuffer()->getVertexInputBinding());
	vertexInputInfo.vertexAttributeDescriptionCount = pRenderable->getVertexBuffer()->getVertexInputAttributes().size();
	vertexInputInfo.pVertexAttributeDescriptions = pRenderable->getVertexBuffer()->getVertexInputAttributes().data();

	// vertex attribute state info
	mAssemblyStateInfo.topology = pRenderable->getTopologyType();
	mAssemblyStateInfo.primitiveRestartEnable = pRenderable->isIndexRestart();

	// color blend state info
	mBlendStateInfo.attachmentCount = mBlendAttachments.size();
	mBlendStateInfo.pAttachments = mBlendAttachments.data();

	// push-constant info
	VkPushConstantRange mvpPushRange;
	mvpPushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvpPushRange.offset = 0;
	mvpPushRange.size = sizeof(glm::mat4);

	// pipeline layout info
	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.setLayoutCount = pRenderable->getDescriptorSetLayout().size();
	layoutInfo.pSetLayouts = pRenderable->getDescriptorSetLayout().data();
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &mvpPushRange;

	// Todo: more information is about the input shader's parameters
	res = vkCreatePipelineLayout(m_pDevice->getGraphicDevice(), &layoutInfo, VK_ALLOC_CALLBACK, &layout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	//pipelineInfo.basePipelineHandle = 0;
	//pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &mAssemblyStateInfo;
	pipelineInfo.pRasterizationState = &mRasterizationStateInfo;
	pipelineInfo.pColorBlendState = &mBlendStateInfo;

	//pipelineInfo.pDynamicState = &dynamicStateInfo;
	if(mViewportStateInfo.viewportCount != 0)
		pipelineInfo.pViewportState = &mViewportStateInfo;

	pipelineInfo.pDepthStencilState = &mDepthStencilStateInfo;
	pipelineInfo.pMultisampleState = &mMultiSampleStateInfo;

	// This step needs to optimize use like std::array, and the preallocate size can be 5
	std::vector<VkPipelineShaderStageCreateInfo> activeShaderStagesInfo;
	pRenderable->getShaderStageInfo(activeShaderStagesInfo);
	pipelineInfo.pStages = activeShaderStagesInfo.data();
	pipelineInfo.stageCount = activeShaderStagesInfo.size();

	pipelineInfo.renderPass = pipelineState.activeRenderPass;
	pipelineInfo.subpass = 0;

	// pipeline layout 
	pipelineInfo.layout = layout;

	res = vkCreateGraphicsPipelines(m_pDevice->getGraphicDevice(), m_pipelineCache, 1, &pipelineInfo, VK_ALLOC_CALLBACK, &pipeline);
	assert(res == VK_SUCCESS);
	return res == VK_SUCCESS;
}

bool VulkanPipeline::createComputePipeline()
{
	return false;
}

void VulkanPipeline::destroyPipeline(const VkPipeline& pipeline)
{
	vkDestroyPipeline(m_pDevice->getGraphicDevice(), pipeline, VK_ALLOC_CALLBACK);
}

void VulkanPipeline::destroyPipelineCache(const VkPipelineCache & pipelineCache)
{
	vkDestroyPipelineCache(m_pDevice->getGraphicDevice(), pipelineCache, VK_ALLOC_CALLBACK);
}

void VulkanPipeline::setViewport(float width, float height, float offsetWidth, float offsetHeight, float minDepth, float maxDepth)
{
	VkViewport viewport;
	viewport.height = height;
	viewport.width = width;
	viewport.minDepth = minDepth;
	viewport.maxDepth = maxDepth;
	viewport.x = offsetWidth;
	viewport.y = offsetHeight;
	addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	mViewportStateInfo.viewportCount = 1;
	mViewportStateInfo.pViewports = &viewport;
	
	VkRect2D scissor;
	scissor.extent.width = width;
	scissor.extent.height = height;
	scissor.offset.x = offsetWidth;
	scissor.offset.y = offsetHeight;
	mViewportStateInfo.scissorCount = 1;
	mViewportStateInfo.pScissors = &scissor;

	addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
}

void VulkanPipeline::addDynamicState(const VkDynamicState state)
{
	mDynamicStateSize++;
	mDynamicStates[mDynamicStateSize] = state;
}

void VulkanPipeline::enableIndexRestart(bool isRestart)
{
	mAssemblyStateInfo.primitiveRestartEnable = isRestart;
}

void VulkanPipeline::setTopologyType(VkPrimitiveTopology topologyType)
{
	mAssemblyStateInfo.topology = topologyType;
}

void VulkanPipeline::setPolygonMode(VkPolygonMode polygonMode)
{
	mRasterizationStateInfo.polygonMode = polygonMode;
}

void VulkanPipeline::setCullMode(VkCullModeFlagBits cullMode, VkFrontFace frontFace)
{
	mRasterizationStateInfo.cullMode = cullMode;
	mRasterizationStateInfo.frontFace = frontFace;
}

void VulkanPipeline::setLineWidth(float width)
{
	mRasterizationStateInfo.lineWidth = width;
}

void VulkanPipeline::addColorBlendAttachment()
{
}

void VulkanPipeline::enableDepthTest(bool isEnable, bool isWriteEnable, VkCompareOp compareOp, bool isDepthBoundTestEnable)
{
	mDepthStencilStateInfo.depthTestEnable = isEnable;
	mDepthStencilStateInfo.depthWriteEnable = isWriteEnable;
	mDepthStencilStateInfo.depthCompareOp = compareOp;
	mDepthStencilStateInfo.depthBoundsTestEnable = isDepthBoundTestEnable;
}

void VulkanPipeline::enableStencilTest(bool isEnable, VkStencilOpState frontState, VkStencilOpState backState)
{
	mDepthStencilStateInfo.stencilTestEnable = isEnable;
}
