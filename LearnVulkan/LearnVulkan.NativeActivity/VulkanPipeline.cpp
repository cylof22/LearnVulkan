#include "common\vulkan_wrapper.h"
#include "VulkanPipeline.h"
#include "VulkanApplication.h"
#include "VulkanDevice.h"
#include "VulkanRenderable.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanPipelineState.h"
#include <glm\glm.hpp>

VulkanPipeline::VulkanPipeline(VulkanDevice * pDevice) : m_pDevice(pDevice)
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
	res = vkCreatePipelineCache(m_pDevice->getGraphicDevice(), &info, nullptr, &m_pipelineCache);
	assert(res == VK_SUCCESS);
	return res;
}

bool VulkanPipeline::createGraphicPipeline(ANativeWindow* pWnd, const VulkanRenderable* pRenderable, const VulkanGraphicPipelineState& pipelineState, 
	VkPipeline& pipeline, VkPipelineLayout& layout)
{
	VkResult res;

	// dynamic state
	VkDynamicState dynamicStates[VK_DYNAMIC_STATE_RANGE_SIZE];
	memset(dynamicStates, 0, sizeof(dynamicStates));

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.pNext = nullptr;
	dynamicStateInfo.pDynamicStates = dynamicStates;
	dynamicStateInfo.dynamicStateCount = 0;
	dynamicStates[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	dynamicStates[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	
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
	VkPipelineInputAssemblyStateCreateInfo vertexAssemblyInfo = {};
	vertexAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vertexAssemblyInfo.pNext = nullptr;
	vertexAssemblyInfo.topology = pRenderable->getTopologyType();
	vertexAssemblyInfo.primitiveRestartEnable = VK_FALSE; //pRenderable->isIndexRestart();

	// rasterization state info
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.pNext = nullptr;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.depthClampEnable = VK_TRUE;
	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.depthBiasConstantFactor = 0;
	rasterizationInfo.depthClampEnable = VK_FALSE;
	rasterizationInfo.depthBiasSlopeFactor = 0;
	rasterizationInfo.lineWidth = 1.0f;

	// color blend state info
	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	blendState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.pNext = nullptr;
	blendInfo.flags = 0;
	blendInfo.attachmentCount = 1;
	blendInfo.pAttachments = &blendState;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.blendConstants[0] = 0.0;
	blendInfo.blendConstants[1] = 0.0;
	blendInfo.blendConstants[2] = 0.0;
	blendInfo.blendConstants[3] = 0.0;

	VkViewport viewport;
	viewport.height = (float)ANativeWindow_getHeight(pWnd);
	viewport.width = (float)ANativeWindow_getWidth(pWnd);
	viewport.minDepth = (float) 0.0f;
	viewport.maxDepth = (float) 1.0f;
	viewport.x = 0;
	viewport.y = 0;

	VkRect2D scissor;
	scissor.extent.width = ANativeWindow_getWidth(pWnd);
	scissor.extent.height = ANativeWindow_getHeight(pWnd);
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	// viewport state info
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = nullptr;
	viewportInfo.flags = 0;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &scissor;
	viewportInfo.pViewports = &viewport;

	// depth & stencile state info
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = {};
	depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilInfo.pNext = nullptr;
	depthStencilInfo.flags = 0;
	depthStencilInfo.depthTestEnable = true;
	depthStencilInfo.depthWriteEnable = true;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilInfo.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilInfo.back.compareMask = 0;
	depthStencilInfo.back.reference = 0;
	depthStencilInfo.back.depthFailOp = VK_STENCIL_OP_KEEP;
	depthStencilInfo.back.writeMask = 0;
	depthStencilInfo.minDepthBounds = 0;
	depthStencilInfo.maxDepthBounds = 0;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
	depthStencilInfo.front = depthStencilInfo.back;

	// Multi-Sample State info
	VkPipelineMultisampleStateCreateInfo multisampleInfo = {};
	multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleInfo.pNext = nullptr;
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleInfo.sampleShadingEnable = VK_FALSE;

	// push-constant info
	VkPushConstantRange mvpPushRange;
	mvpPushRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvpPushRange.offset = 0;
	mvpPushRange.size = sizeof(glm::mat4);

	// pipeline layout info
	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.setLayoutCount = 0;// pRenderable->getDescriptorSetLayout().size();
	layoutInfo.pSetLayouts = pRenderable->getDescriptorSetLayout().data();
	layoutInfo.pushConstantRangeCount = 1;
	layoutInfo.pPushConstantRanges = &mvpPushRange;

	// Todo: more information is about the input shader's parameters
	res = vkCreatePipelineLayout(m_pDevice->getGraphicDevice(), &layoutInfo, nullptr, &layout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	//pipelineInfo.basePipelineHandle = 0;
	//pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &vertexAssemblyInfo;
	pipelineInfo.pRasterizationState = &rasterizationInfo;
	pipelineInfo.pColorBlendState = &blendInfo;

	//pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.pMultisampleState = &multisampleInfo;

	// This step needs to optimize use like std::array, and the preallocate size can be 5
	std::vector<VkPipelineShaderStageCreateInfo> activeShaderStagesInfo;
	pRenderable->getShaderStageInfo(activeShaderStagesInfo);
	pipelineInfo.pStages = activeShaderStagesInfo.data();
	pipelineInfo.stageCount = activeShaderStagesInfo.size();

	pipelineInfo.renderPass = pipelineState.activeRenderPass;
	pipelineInfo.subpass = 0;

	// pipeline layout 
	pipelineInfo.layout = layout;

	res = vkCreateGraphicsPipelines(m_pDevice->getGraphicDevice(), m_pipelineCache, 1, &pipelineInfo, nullptr, &pipeline);
	assert(res == VK_SUCCESS);
	return res == VK_SUCCESS;
}

bool VulkanPipeline::createComputePipeline()
{
	return false;
}

void VulkanPipeline::destroyPipeline(const VkPipeline& pipeline)
{
	vkDestroyPipeline(m_pDevice->getGraphicDevice(), pipeline, nullptr);
}

void VulkanPipeline::destroyPipelineCache(const VkPipelineCache & pipelineCache)
{
	vkDestroyPipelineCache(m_pDevice->getGraphicDevice(), pipelineCache, nullptr);
}
