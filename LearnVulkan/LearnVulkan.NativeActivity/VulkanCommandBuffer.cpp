#include "pch.h"
#include "VulkanGraphicContext.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemoryBarrierSet.h"
#include "VulkanHardwareBuffer.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanHardwareIndexBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffer.h"

VulkanCommandBuffer::VulkanCommandBuffer(VulkanGraphicContext* pContext)
	: m_pGraphicContext(pContext)
{
}


VulkanCommandBuffer::~VulkanCommandBuffer()
{
}

void VulkanCommandBuffer::pipelineBarrier(VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VulkanMemoryBarrierSet & memoryBarrierSet)
{
	vkCmdPipelineBarrier(m_cmdBuffer, srcStageMask, dstStageMask, 0, 
		memoryBarrierSet.getMemoryBarriers().size(), (VkMemoryBarrier*) memoryBarrierSet.getMemoryBarriers().data(),
		memoryBarrierSet.getBufferBarriers().size(), (VkBufferMemoryBarrier*)memoryBarrierSet.getBufferBarriers().data(),
		memoryBarrierSet.getImageBarriers().size(), (VkImageMemoryBarrier*)memoryBarrierSet.getImageBarriers().data()
		);
}

void VulkanCommandBuffer::waitForEvent(VkEvent event, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, const VulkanMemoryBarrierSet & memoryBarrierSet)
{
	vkCmdWaitEvents(m_cmdBuffer, 1, &event, srcStageMask, dstStageMask,
		memoryBarrierSet.getMemoryBarriers().size(), (VkMemoryBarrier*)memoryBarrierSet.getMemoryBarriers().data(),
		memoryBarrierSet.getBufferBarriers().size(), (VkBufferMemoryBarrier*)memoryBarrierSet.getBufferBarriers().data(),
		memoryBarrierSet.getImageBarriers().size(), (VkImageMemoryBarrier*)memoryBarrierSet.getImageBarriers().data()
	);
}

void VulkanCommandBuffer::setEvent(VkEvent event, VkPipelineStageFlags stage)
{
	vkCmdSetEvent(m_cmdBuffer, event, stage);
}

void VulkanCommandBuffer::resetEvent(VkEvent event, VkPipelineStageFlags stage)
{
	vkCmdResetEvent(m_cmdBuffer, event, stage);
}

void VulkanCommandBuffer::bindGraphicPipeline(const VulkanGraphicPipeline & graphicPipeline)
{
}

void VulkanCommandBuffer::bindComputePipeline(const VulkanComputePipeline & computePipeline)
{
}

void VulkanCommandBuffer::bindDescriptorSetGraphic(VkPipelineLayout graphicPipelineLayout, VkDescriptorSet descriptorSet)
{
	std::vector<VkDescriptorSet> descriptorSets;
	descriptorSets.emplace_back(descriptorSet);
	bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipelineLayout, 0, descriptorSets);
}

void VulkanCommandBuffer::bindDescriptorSetGraphic(VkPipelineLayout graphicPipelineLayout, uint32_t index, std::vector<VkDescriptorSet>& descriptorSets)
{
	bindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS, graphicPipelineLayout, index, descriptorSets);
}

void VulkanCommandBuffer::bindDescriptorSetCompute(VkPipelineLayout computePipelineLayout, VkDescriptorSet descriptorSet)
{
	std::vector<VkDescriptorSet> descriptorSets;
	descriptorSets.emplace_back(descriptorSet);
	bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0, descriptorSets);
}

void VulkanCommandBuffer::bindDescriptorSetCompute(VkPipelineLayout computePipelineLayout, uint32_t index, std::vector<VkDescriptorSet>& descriptorSets)
{
	bindDescriptorSets(VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, index, descriptorSets);
}

void VulkanCommandBuffer::bindDescriptorSets(VkPipelineBindPoint bindType, VkPipelineLayout pipelineLayout, uint32_t index, std::vector<VkDescriptorSet>& descriptorSets)
{
	vkCmdBindDescriptorSets(m_cmdBuffer, bindType, pipelineLayout, index, descriptorSets.size(), descriptorSets.data(), 0, nullptr);
}

void VulkanCommandBuffer::bindVertexBuffer(const VulkanHardwareVertexBuffer & vertexBuffer, uint32_t binding, VkDeviceSize offset /*= { 0 }*/)
{
	vkCmdBindVertexBuffers(m_cmdBuffer, binding, 1, (VkBuffer*)&vertexBuffer.getVertexBuffer(), &offset);
}

void VulkanCommandBuffer::bindIndexBuffer(const VulkanHardwareIndexBuffer & indexBuffer, VkDeviceSize offset /*= { 0 }*/)
{
	vkCmdBindIndexBuffer(m_cmdBuffer, indexBuffer.getBuffer(), offset, indexBuffer.getType());
}

void VulkanCommandBuffer::beginRecording()
{
	m_isRecording = true;
	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.pInheritanceInfo = nullptr;

	vkBeginCommandBuffer(m_cmdBuffer, &info);
}

void VulkanCommandBuffer::beginRecording(const VulkanRenderPass & renderPass, uint32_t subPassSize)
{
	m_isRecording = true;

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
	inheritanceInfo.pNext = nullptr;
	inheritanceInfo.renderPass = renderPass.getRenderPass();
	inheritanceInfo.subpass = subPassSize;
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;

	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(m_cmdBuffer, &info);

}

void VulkanCommandBuffer::beginRecording(const VulkanFrameBuffer & fbo, uint32_t subPassSize)
{
	m_isRecording = true;

	VkCommandBufferInheritanceInfo inheritanceInfo = {};
	inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	inheritanceInfo.pNext = nullptr;
	inheritanceInfo.renderPass = fbo.getAttachedRenderPass().getRenderPass();
	inheritanceInfo.subpass = subPassSize;
	inheritanceInfo.framebuffer = fbo.getFrameBuffer();
	inheritanceInfo.occlusionQueryEnable = VK_FALSE;
	inheritanceInfo.queryFlags = 0;
	inheritanceInfo.pipelineStatistics = 0;

	VkCommandBufferBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.pInheritanceInfo = &inheritanceInfo;

	vkBeginCommandBuffer(m_cmdBuffer, &info);
}

void VulkanCommandBuffer::endRecording()
{
	vkEndCommandBuffer(m_cmdBuffer);
	m_isRecording = false;
}

void VulkanCommandBuffer::submit(const VkSemaphore & waitSemaphore, const VkSemaphore & signalSemaphore, const VkFence fence)
{
	VkSemaphore* waitSems = NULL;
	VkSemaphore* signalSems = NULL;

	if (waitSemaphore != VK_NULL_HANDLE)
	{
		waitSems = (VkSemaphore*)&waitSemaphore;
	}
	if (signalSemaphore != VK_NULL_HANDLE)
	{
		signalSems = (VkSemaphore*)&signalSemaphore;
	}
	submitCommandBuffers(m_pGraphicContext->getMainQueue(), m_pGraphicContext->getDevice(), &m_cmdBuffer, 1, waitSems, waitSems != 0, signalSems, signalSems != 0, fence);
}

void VulkanCommandBuffer::submit(VkFence & fence)
{
	uint32_t swapIndex = m_pGraphicContext->getSwapChainIndex();
	VkSemaphore checkBeginingSemaphore = m_pGraphicContext->getBegingRecordingCheckingSemaphore(swapIndex);
	VkSemaphore beginingSeamphore = m_pGraphicContext->getBegingRecordingSemaphore(swapIndex);

	submitCommandBuffers(m_pGraphicContext->getMainQueue(), m_pGraphicContext->getDevice(), &m_cmdBuffer, 1, &checkBeginingSemaphore,
		checkBeginingSemaphore != VK_NULL_HANDLE, &beginingSeamphore,
		beginingSeamphore != VK_NULL_HANDLE, fence);
}

void VulkanCommandBuffer::submit()
{
	uint32_t swapIndex = m_pGraphicContext->getSwapChainIndex();
	VkSemaphore checkBeginingSemaphore = m_pGraphicContext->getBegingRecordingCheckingSemaphore(swapIndex);
	VkSemaphore beginingSeamphore = m_pGraphicContext->getBegingRecordingSemaphore(swapIndex);
	VkFence fence = m_pGraphicContext->getRenderFence(swapIndex);

	submitCommandBuffers(m_pGraphicContext->getMainQueue(), m_pGraphicContext->getDevice(), &m_cmdBuffer, 1, &checkBeginingSemaphore,
		checkBeginingSemaphore != VK_NULL_HANDLE, &beginingSeamphore,
		beginingSeamphore != VK_NULL_HANDLE, fence);

}

void VulkanCommandBuffer::submitEndOfFrame(VkSemaphore & waitSemaphore)
{
	uint32_t swapIndex = m_pGraphicContext->getSwapChainIndex();
	VkFence renderFence = m_pGraphicContext->getRenderFence(swapIndex);
	VkSemaphore finishedRenderingSemaphore = m_pGraphicContext->getFinishingRecordingSemaphore(swapIndex);

	submitCommandBuffers(m_pGraphicContext->getMainQueue(), m_pGraphicContext->getDevice(), &m_cmdBuffer, 1,
		&waitSemaphore, waitSemaphore != VK_NULL_HANDLE, &finishedRenderingSemaphore,
		finishedRenderingSemaphore != VK_NULL_HANDLE, renderFence);
}

void VulkanCommandBuffer::submitStartOfFrame(VkSemaphore & signalSemaphore, const VkFence & fence)
{
	uint32_t swapIndex = m_pGraphicContext->getSwapChainIndex();
	VkSemaphore checkBeginingSemaphore = m_pGraphicContext->getBegingRecordingCheckingSemaphore(swapIndex);

	submitCommandBuffers(m_pGraphicContext->getMainQueue(), m_pGraphicContext->getDevice(), &m_cmdBuffer, 1,
		&checkBeginingSemaphore, checkBeginingSemaphore != VK_NULL_HANDLE,
		&signalSemaphore, signalSemaphore != VK_NULL_HANDLE, fence);
}

void VulkanCommandBuffer::enqueueSecondaryCmd(VkCommandBuffer & secondaryCmdBuffer)
{
}

void VulkanCommandBuffer::enqueueSecondaryCmd(VkCommandBuffer * secondaryCmdBuffers, uint32_t numCmdBuffers)
{
}

void VulkanCommandBuffer::enqueueSecondaryCmdBeginMultiple(uint32_t expectedMax)
{
}

void VulkanCommandBuffer::enqueueSecondaryCmdsEnqueueMultiple(VkCommandBuffer * secondaryCmdBuffers, uint32_t numCmdBuffers)
{
}

void VulkanCommandBuffer::enqueueSecondaryCmdsSubmitMultiple(bool keepAllocated)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 * clearColors, uint32_t numClearColors, float clearDepth, uint32_t clearStencil)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, bool inlineFirstSubpass, const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 * clearColors, uint32_t numClearColors, float * clearDepth, uint32_t * clearStencil, uint32_t numClearDepthStencil)
{
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, bool inlineFirstSubpass, const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
}

void VulkanCommandBuffer::endRenderPass()
{
}

void VulkanCommandBuffer::updateBuffer(VulkanHardwareBuffer & buffer, const void * data, uint32_t offset, uint32_t length)
{
}

void VulkanCommandBuffer::copyBuffer(VulkanHardwareBuffer src, VulkanHardwareBuffer dest, uint32_t srcOffset, uint32_t destOffset, uint32_t sizeInBytes)
{
}

void VulkanCommandBuffer::drawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexOffset, uint32_t firstInstance, uint32_t instanceCount)
{
}

void VulkanCommandBuffer::drawArrays(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
}

void VulkanCommandBuffer::drawArraysIndirect(VulkanHardwareBuffer & buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
}

void VulkanCommandBuffer::drawIndexedIndirect(VulkanHardwareIndexBuffer & buffer)
{
}

void VulkanCommandBuffer::drawIndirect(VulkanHardwareBuffer & buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
}

void VulkanCommandBuffer::dispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
}

void VulkanCommandBuffer::submitCommandBuffers(VkQueue queue, VkDevice device, const VkCommandBuffer * cmdBuffs, uint32_t numCmdBuffs, const VkSemaphore * waitSems, uint32_t numWaitSems, const VkSemaphore * signalSems, uint32_t numSignalSems, VkFence signalFence)
{
	VkResult res;
	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	VkSubmitInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	nfo.waitSemaphoreCount = numWaitSems;
	nfo.pWaitSemaphores = waitSems;
	nfo.pWaitDstStageMask = &pipeStageFlags;
	nfo.pCommandBuffers = cmdBuffs;
	nfo.commandBufferCount = numCmdBuffs;
	nfo.pSignalSemaphores = signalSems;
	nfo.signalSemaphoreCount = numSignalSems;
	res = vkQueueSubmit(queue, 1, &nfo, signalFence);
	assert(res == VK_SUCCESS);
}
