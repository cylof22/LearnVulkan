#include "common\vulkan_wrapper.h"
#include "VulkanGraphicPipeline.h"
#include "VulkanApplication.h"
#include "VulkanDevice.h"
#include "VulkanRenderable.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanPipelineState.h"
#include <glm\glm.hpp>

VulkanGraphicPipeline::VulkanGraphicPipeline(VulkanDevice * pDevice) : m_pDevice(pDevice), mDynamicStateSize(0)
{
}

VulkanGraphicPipeline::~VulkanGraphicPipeline()
{
}

VkResult VulkanGraphicPipeline::createPipeLineCache()
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

bool VulkanGraphicPipeline::createGraphicPipeline(ANativeWindow* pWnd, const VulkanRenderable* pRenderable, const VulkanGraphicPipelineState& pipelineState,
	VkPipeline& pipeline, VkPipelineLayout& layout)
{
	VkResult res;

	// add viewport and scissor state
	mDynamicStateInfo.dynamicStateCount = mDynamicStateSize;
	mDynamicStateInfo.pDynamicStates = mDynamicStates.data();
	
	// vertex input state info
	mVertexInputStateInfo.pVertexBindingDescriptions = &(pRenderable->getVertexBuffer()->getVertexInputBinding());
	mVertexInputStateInfo.vertexAttributeDescriptionCount = pRenderable->getVertexBuffer()->getVertexInputAttributes().size();
	mVertexInputStateInfo.pVertexAttributeDescriptions = pRenderable->getVertexBuffer()->getVertexInputAttributes().data();

	// vertex attribute state info
	mAssemblyStateInfo.topology = pRenderable->getTopologyType();
	mAssemblyStateInfo.primitiveRestartEnable = pRenderable->isIndexRestart();

	// color blend state info
	mBlendStateInfo.attachmentCount = mBlendAttachments.size();
	mBlendStateInfo.pAttachments = mBlendAttachments.data();

	// pipeline layout info
	mPipelineLayoutInfo.setLayoutCount = pRenderable->getDescriptorSetLayout().size();
	mPipelineLayoutInfo.pSetLayouts = pRenderable->getDescriptorSetLayout().data();
	mPipelineLayoutInfo.pushConstantRangeCount = pRenderable->getPushConstant().size();
	mPipelineLayoutInfo.pPushConstantRanges = pRenderable->getPushConstant().data();

	// Todo: more information is about the input shader's parameters
	res = vkCreatePipelineLayout(m_pDevice->getGraphicDevice(), &mPipelineLayoutInfo, VK_ALLOC_CALLBACK, &layout);

	//pipelineInfo.pDynamicState = &dynamicStateInfo;
	if(mViewportStateInfo.viewportCount != 0)
		mGraphicPipelineInfo.pViewportState = &mViewportStateInfo;

	// This step needs to optimize use like std::array, and the preallocate size can be 5
	std::vector<VkPipelineShaderStageCreateInfo> activeShaderStagesInfo;
	pRenderable->getShaderStageInfo(activeShaderStagesInfo);
	mGraphicPipelineInfo.pStages = activeShaderStagesInfo.data();
	mGraphicPipelineInfo.stageCount = activeShaderStagesInfo.size();

	mGraphicPipelineInfo.renderPass = pipelineState.activeRenderPass;
	mGraphicPipelineInfo.subpass = 0;

	// pipeline layout 
	mGraphicPipelineInfo.layout = layout;

	res = vkCreateGraphicsPipelines(m_pDevice->getGraphicDevice(), m_pipelineCache, 1, &mGraphicPipelineInfo, VK_ALLOC_CALLBACK, &pipeline);
	assert(res == VK_SUCCESS);
	return res == VK_SUCCESS;
}

void VulkanGraphicPipeline::destroyPipeline(const VkPipeline& pipeline)
{
	vkDestroyPipeline(m_pDevice->getGraphicDevice(), pipeline, VK_ALLOC_CALLBACK);
}

void VulkanGraphicPipeline::destroyPipelineCache(const VkPipelineCache & pipelineCache)
{
	vkDestroyPipelineCache(m_pDevice->getGraphicDevice(), pipelineCache, VK_ALLOC_CALLBACK);
}

void VulkanGraphicPipeline::setViewport(float width, float height, float offsetWidth, float offsetHeight, float minDepth, float maxDepth)
{
	mViewport.height = height;
	mViewport.width = width;
	mViewport.minDepth = minDepth;
	mViewport.maxDepth = maxDepth;
	mViewport.x = offsetWidth;
	mViewport.y = offsetHeight;
	addDynamicState(VK_DYNAMIC_STATE_VIEWPORT);
	
	mScissor.extent.width = width;
	mScissor.extent.height = height;
	mScissor.offset.x = offsetWidth;
	mScissor.offset.y = offsetHeight;
	addDynamicState(VK_DYNAMIC_STATE_SCISSOR);
}

void VulkanGraphicPipeline::addDynamicState(const VkDynamicState state)
{
	mDynamicStateSize++;
	mDynamicStates[mDynamicStateSize] = state;
}

void VulkanGraphicPipeline::enableIndexRestart(bool isRestart)
{
	mAssemblyStateInfo.primitiveRestartEnable = isRestart;
}

void VulkanGraphicPipeline::setTopologyType(VkPrimitiveTopology topologyType)
{
	mAssemblyStateInfo.topology = topologyType;
}

void VulkanGraphicPipeline::setPolygonMode(VkPolygonMode polygonMode)
{
	mRasterizationStateInfo.polygonMode = polygonMode;
}

void VulkanGraphicPipeline::setCullMode(VkCullModeFlagBits cullMode, VkFrontFace frontFace)
{
	mRasterizationStateInfo.cullMode = cullMode;
	mRasterizationStateInfo.frontFace = frontFace;
}

void VulkanGraphicPipeline::setLineWidth(float width)
{
	mRasterizationStateInfo.lineWidth = width;
}

void VulkanGraphicPipeline::addColorBlendAttachment(VkPipelineColorBlendAttachmentState blendAttachmentState)
{
	mBlendAttachments.emplace_back(blendAttachmentState);
}

void VulkanGraphicPipeline::enableDepthTest(bool isEnable, bool isWriteEnable, VkCompareOp compareOp, bool isDepthBoundTestEnable)
{
	mDepthStencilStateInfo.depthTestEnable = isEnable;
	mDepthStencilStateInfo.depthWriteEnable = isWriteEnable;
	mDepthStencilStateInfo.depthCompareOp = compareOp;
	mDepthStencilStateInfo.depthBoundsTestEnable = isDepthBoundTestEnable;
}

void VulkanGraphicPipeline::enableStencilTest(bool isEnable, VkStencilOpState frontState, VkStencilOpState backState)
{
	mDepthStencilStateInfo.stencilTestEnable = isEnable;
}
