#pragma once
#include <vector>
#include <glm\glm.hpp>
#include "common\vulkan_wrapper.h"

class VulkanMemoryBarrierSet;
class VulkanGraphicPipeline;
class VulkanComputePipeline;
class VulkanHardwareVertexBuffer;
class VulkanHardwareIndexBuffer;
class VulkanRenderPass;
class VulkanFrameBuffer;

class VulkanCommandBuffer
{
public:
	VulkanCommandBuffer();
	~VulkanCommandBuffer();

	// Synchronization
	void pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VulkanMemoryBarrierSet& memoryBarrierSet);
	void waitForEvent(VkEvent event, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VulkanMemoryBarrierSet& memoryBarrierSet);
	//void waitForEvents(, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VulkanMemoryBarrierSet& memoryBarrierSet);
	void setEvent(VkEvent event, VkPipelineStageFlags stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
	void resetEvent(VkEvent event, VkPipelineStageFlags stage = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

	// Pipelines
	void bindGraphicPipeline(const VulkanGraphicPipeline& graphicPipeline);
	void bindComputePipeline(const VulkanComputePipeline& computePipeline);

	// Descriptor Set
	void bindDescriptorSetGraphic(VkPipelineLayout graphicPipelineLayout, VkDescriptorSet descriptorSet);
	void bindDescriptorSetGraphic(VkPipelineLayout graphicPipelineLayout, uint32_t startIndex, std::vector<VkDescriptorSet>& descriptorSets);
	void bindDescriptorSetCompute(VkPipelineLayout computePipelineLayout, VkDescriptorSet descriptorSet);
	void bindDescriptorSetCompute(VkPipelineLayout computePipelineLayout, uint32_t index, std::vector<VkDescriptorSet>& descriptorSets);
	void bindDescriptorSets(VkPipelineBindPoint bindType, VkPipelineLayout pipelineLayout, uint32_t index, std::vector<VkDescriptorSet>& descriptorSets);
	void bindVertexBuffer(const VulkanHardwareVertexBuffer& vertexBuffer, uint32_t binding, VkDeviceSize offset = { 0 });
	void bindIndexBuffer(const VulkanHardwareIndexBuffer& indexBuffer, VkDeviceSize offset = { 0 });

	// Recording
	void beginRecording();
	void beginRecording(const VulkanRenderPass& renderPass, uint32_t subPassSize);
	void beginRecording(const VulkanFrameBuffer& fbo, uint32_t subPassSize);
	void endRecording();

	// Submitting
	void submit(const VkSemaphore& waitSemaphore, const VkSemaphore& signalSemaphore, const VkFence fence = VK_NULL_HANDLE);
	//void submit(SemaphoreSet& waitSemaphores, SemaphoreSet& signalSemaphores, const VkFence fence = VK_NULL_HANDLE);
	void submit(VkFence& fence);
	void submit();
	void submitEndOfFrame(VkSemaphore& waitSemaphore);
	void submitStartOfFrame(VkSemaphore& signalSemaphore, const VkFence& fence = VK_NULL_HANDLE);

	// Secondary Command Buffer
	void enqueueSecondaryCmd(VkCommandBuffer& secondaryCmdBuffer);
	void enqueueSecondaryCmd(VkCommandBuffer* secondaryCmdBuffers, uint32_t numCmdBuffers);
	void enqueueSecondaryCmdBeginMultiple(uint32_t expectedMax = 255);
	void enqueueSecondaryCmdsEnqueueMultiple(VkCommandBuffer* secondaryCmdBuffers, uint32_t numCmdBuffers);
	void enqueueSecondaryCmdsSubmitMultiple(bool keepAllocated = false);

	// Renderpass 
	void beginRenderPass(VulkanFrameBuffer& fbo, const VkRect2D& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float clearDepth = 1.f, uint32_t clearStencil = 0);
	void beginRenderPass(VulkanFrameBuffer& fbo, const VkRect2D& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32_t numClearColors, float clearDepth = 1.f, uint32_t clearStencil = 0);
	void beginRenderPass(VulkanFrameBuffer& fbo, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float clearDepth = 1.f, uint32_t clearStencil = 0);
	void beginRenderPass(VulkanFrameBuffer& fbo, const VulkanRenderPass& renderPass, const VkRect2D& renderArea, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float clearDepth = 1.f, uint32_t clearStencil = 0);
	void beginRenderPass(VulkanFrameBuffer& fbo, const VulkanRenderPass& renderPass, const VkRect2D& renderArea, bool inlineFirstSubpass, const glm::vec4* clearColors, uint32_t numClearColors, float* clearDepth, uint32_t* clearStencil, uint32_t numClearDepthStencil);
	void beginRenderPass(VulkanFrameBuffer& fbo, const VulkanRenderPass& renderPass, bool inlineFirstSubpass, const glm::vec4& clearColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), float clearDepth = 1.f, uint32_t clearStencil = 0);
	void endRenderPass();

	// Buffer, Image
	void updateBuffer(Buffer& buffer, const void* data, uint32_t offset, uint32_t length);
	void copyBuffer(api::Buffer src, api::Buffer dest, uint32_t srcOffset, uint32_t destOffset, uint32_t sizeInBytes);
	void blitImage(api::TextureStore& src, api::TextureStore& dest, types::ImageLayout srcLayout, types::ImageLayout dstLayout, types::ImageBlitRange* regions, uint32 numRegions, types::SamplerFilter filter);
	void copyImageToBuffer(api::TextureStore& srcImage, types::ImageLayout srcImageLayout, api::Buffer& dstBuffer, types::BufferImageCopy* regions, uint32 numRegions);

	// Drawing 
	void drawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexOffset = 0, uint32_t firstInstance = 0, uint32_t instanceCount = 1);
	void drawArrays(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance = 0, uint32_t instanceCount = 1);
	void drawArraysIndirect(api::Buffer& buffer, uint32 offset, uint32 drawCount, uint32 stride);
	void drawIndexedIndirect(Buffer& buffer);
	void drawIndirect(Buffer& buffer, uint32_t offset, uint32_t count, uint32_t stride);
	void dispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1);

private:
	VkCommandBuffer m_cmdBuffer;
	bool m_isRecording;
};

