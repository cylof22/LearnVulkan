#pragma once
#include <array>
#include <vector>
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

	bool createGraphicPipeline(ANativeWindow* pWnd, const VulkanRenderable* pRenderable, const VulkanGraphicPipelineState& pipelineState, VkPipeline& pipeline, VkPipelineLayout& layout);
	bool createComputePipeline();
	void destroyPipeline(const VkPipeline& pipeline);
	void destroyPipelineCache(const VkPipelineCache& pipelineCache);

	void setViewport(float width, float height, float offsetWidth = 0.0f, float offsetHeight = 0.0f, float minDepth = 0.0f, float maxDepth = 1.0f);
	void addDynamicState(const VkDynamicState state);

	void enableIndexRestart(bool isRestart);
	void setTopologyType(VkPrimitiveTopology topologyType);

	void setPolygonMode(VkPolygonMode polygonMode);
	void setCullMode(VkCullModeFlagBits cullMode, VkFrontFace frontFace);
	void setLineWidth(float width);

	void addColorBlendAttachment(VkPipelineColorBlendAttachmentState blendAttachmentState);

	void enableDepthTest(bool isEnable, bool isWriteEnable = true, VkCompareOp compareOp = VK_COMPARE_OP_LESS_OR_EQUAL, bool isDepthBoundTestEnable = false);
	void enableStencilTest(bool isEnable, VkStencilOpState frontState, VkStencilOpState backState);
private:
	VkPipelineCache m_pipelineCache;

	VulkanDevice* m_pDevice;

	uint32_t mDynamicStateSize;
	std::array<VkDynamicState, VK_DYNAMIC_STATE_RANGE_SIZE> mDynamicStates;
	VkPipelineDynamicStateCreateInfo mDynamicStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, 0, nullptr };

	VkPipelineInputAssemblyStateCreateInfo mAssemblyStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, VK_FALSE };

	VkPipelineRasterizationStateCreateInfo mRasterizationStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_TRUE,
		VK_FALSE, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0, 0, 0, 1.0f };

	VkPipelineColorBlendStateCreateInfo mBlendStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0,
		VK_FALSE, VK_LOGIC_OP_AND, 0, nullptr, { 0.0f, 0.0f, 0.0f, 0.0f } 
	};
	std::vector<VkPipelineColorBlendAttachmentState> mBlendAttachments;

	VkPipelineDepthStencilStateCreateInfo mDepthStencilStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0,
		VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_FALSE, VK_FALSE,
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		{ VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_STENCIL_OP_KEEP, VK_COMPARE_OP_ALWAYS, 0, 0, 0 },
		0.0f, 1.0f
	};

	VkPipelineMultisampleStateCreateInfo mMultiSampleStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0,
		VK_SAMPLE_COUNT_1_BIT, VK_FALSE, 1.0f, nullptr, VK_FALSE, VK_FALSE };

	VkPipelineViewportStateCreateInfo mViewportStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr };

	VkPipelineVertexInputStateCreateInfo mVertexInputStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1,
		nullptr, 0, nullptr };

	VkPipelineLayoutCreateInfo mPipelineLayoutInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 0, nullptr };

	VkGraphicsPipelineCreateInfo mGraphicPipelineInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, nullptr, 0,
		0, nullptr, &mVertexInputStateInfo, &mAssemblyStateInfo, nullptr, nullptr, &mRasterizationStateInfo, &mMultiSampleStateInfo, 
		&mDepthStencilStateInfo, &mBlendStateInfo, nullptr, VK_NULL_HANDLE, VK_NULL_HANDLE, 0, VK_NULL_HANDLE, 0 };
};