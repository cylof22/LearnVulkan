#include "common\vulkan_wrapper.h"
#include "VulkanPipeline.h"
#include "VulkanApplication.h"
#include "VulkanDevice.h"
#include "VulkanRenderable.h"
#include "VulkanHardwareVertexBuffer.h"

VulkanPipeline::VulkanPipeline(VulkanApplication * pApp, VulkanDevice * pDevice) : m_pApp(pApp), m_pDevice(pDevice)
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

bool VulkanPipeline::createGraphicPipeline(const VulkanRenderable* pRenderable, const VulkanPipelineState& pipelineState, VkPipeline& pipeline)
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
	dynamicStates[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_LINE_WIDTH;
	
	// vertex input state info
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.flags = 0;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &(pRenderable->getVertexBuffer()->getVertexInputBinding());
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexAttributeDescriptions = pRenderable->getVertexBuffer()->getVertexInputAttributes().data();

	// vertex attribute state info
	VkPipelineInputAssemblyStateCreateInfo vertexAssemblyInfo = {};
	vertexAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vertexAssemblyInfo.pNext = nullptr;
	vertexAssemblyInfo.topology = pRenderable->getTopologyType();
	vertexAssemblyInfo.primitiveRestartEnable = pRenderable->isIndexRestart();

	// rasterization state info
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.pNext = nullptr;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationInfo.depthClampEnable = VK_TRUE;
	rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.depthBiasConstantFactor = 0;
	rasterizationInfo.depthClampEnable = VK_FALSE;
	rasterizationInfo.depthBiasSlopeFactor = 0;
	rasterizationInfo.lineWidth = 1.0f;

	// color blend state info
	VkPipelineColorBlendAttachmentState blendState = {};
	blendState.colorWriteMask = 0xf;
	blendState.blendEnable = VK_FALSE;
	blendState.alphaBlendOp = VK_BLEND_OP_ADD;
	blendState.colorBlendOp = VK_BLEND_OP_ADD;
	blendState.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	blendState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	VkPipelineColorBlendStateCreateInfo blendInfo = {};
	blendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	blendInfo.pNext = nullptr;
	blendInfo.flags = 0;
	blendInfo.attachmentCount = 1;
	blendInfo.pAttachments = &blendState;
	blendInfo.logicOpEnable = VK_FALSE;
	blendInfo.blendConstants[0] = 1.0;
	blendInfo.blendConstants[1] = 1.0;
	blendInfo.blendConstants[2] = 1.0;
	blendInfo.blendConstants[3] = 1.0;

	// viewport state info
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = nullptr;
	viewportInfo.flags = 0;
	viewportInfo.viewportCount = 1;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = nullptr;
	viewportInfo.pViewports = nullptr;

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
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;

	// pipeline layout info
	VkPipelineLayout layout;
	VkPipelineLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	// Todo: more information is about the input shader's parameters
	res = vkCreatePipelineLayout(m_pDevice->getGraphicDevice(), &layoutInfo, nullptr, &layout);

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.flags = 0;
	pipelineInfo.basePipelineHandle = 0;
	pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &vertexAssemblyInfo;
	pipelineInfo.pRasterizationState = &rasterizationInfo;
	pipelineInfo.pColorBlendState = &blendInfo;
	// need to check the VulkanRenderable's tessellation control and evaluation program

	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.pMultisampleState = &multisampleInfo;

	//pipelineInfo.pStages = ;
	//pipelineInfo.stageCount = ;
	//pipelineInfo.renderPass = ;
	//pipelineInfo.subpass = 0;

	// pipeline layout 
	//pipelineInfo.layout = ;
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
