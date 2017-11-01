#include "pch.h"
#include "VulkanGraphicContext.h"
#include "VulkanCommandBuffer.h"
#include "VulkanMemoryBarrierSet.h"
#include "VulkanHardwareBuffer.h"
#include "VulkanHardwareVertexBuffer.h"
#include "VulkanHardwareIndexBuffer.h"
#include "VulkanRenderPass.h"
#include "VulkanFrameBuffer.h"
#include <array>

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
	vkCmdBindVertexBuffers(m_cmdBuffer, binding, 1, (VkBuffer*)&vertexBuffer.getBuffer(), &offset);
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
	//objectRefs.push_back(secondaryCmdBuffer);
	
	assert(secondaryCmdBuffer != VK_NULL_HANDLE);
	vkCmdExecuteCommands(m_cmdBuffer, 1, &secondaryCmdBuffer);
}

void VulkanCommandBuffer::enqueueSecondaryCmd(VkCommandBuffer * secondaryCmdBuffers, uint32_t numCmdBuffers)
{
	// add to objectRefs: record the enqueue second command buffers

	vkCmdExecuteCommands(m_cmdBuffer, numCmdBuffers, secondaryCmdBuffers);
}

void VulkanCommandBuffer::enqueueSecondaryCmdBeginMultiple(uint32_t expectedNumber)
{
	m_multiEnqueueCache.resize(0);
	m_multiEnqueueCache.reserve(expectedNumber);
}

void VulkanCommandBuffer::enqueueSecondaryCmdsEnqueueMultiple(VkCommandBuffer * secondaryCmdBuffers, uint32_t numCmdBuffers)
{
	m_multiEnqueueCache.reserve(m_multiEnqueueCache.size() + numCmdBuffers);
	for (uint32_t i = 0; i < numCmdBuffers; ++i)
	{
		//objectRefs.push_back(secondaryCmdBuffers[i]);
		m_multiEnqueueCache.push_back(secondaryCmdBuffers[i]);
	}
}

void VulkanCommandBuffer::enqueueSecondaryCmdsSubmitMultiple(bool keepAllocated)
{
	vkCmdExecuteCommands(m_cmdBuffer, (uint32_t)m_multiEnqueueCache.size(), m_multiEnqueueCache.data());
	m_multiEnqueueCache.resize(0);
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 & clearColor, 
	float clearDepth, uint32_t clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32_t clearStencils[4];
	assert(fbo.getNumColorAttachments() <= 4);

	for (uint32_t i = 0; i < fbo.getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32_t i = 0; i < fbo.getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}

	beginRenderPass(fbo, fbo.getAttachedRenderPass(), renderArea, inlineFirstSubpass, clearColors,
		fbo.getNumColorAttachments(), clearDepths, clearStencils, fbo.getNumDepthStencilAttachments());

}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 * clearColors, 
	uint32_t numClearColors, float clearDepth, uint32_t clearStencil)
{
	glm::float32 clearDepths[4];
	uint32_t clearStencils[4];
	for (uint32_t i = 0; i < fbo.getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass(fbo, fbo.getAttachedRenderPass(), renderArea, inlineFirstSubpass, clearColors, numClearColors,
		clearDepths, clearStencils, fbo.getNumDepthStencilAttachments());
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, bool inlineFirstSubpass, const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
	glm::vec4 clearColors[4];
	glm::float32 clearDepths[4];
	uint32_t clearStencils[4];
	assert(fbo.getNumColorAttachments() <= 4);
	for (uint32_t i = 0; i < fbo.getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32_t i = 0; i < fbo.getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass(fbo, fbo.getAttachedRenderPass(), fbo.getDimensions(),
		inlineFirstSubpass, clearColors, fbo.getNumColorAttachments(), clearDepths, clearStencils,
		fbo.getNumDepthStencilAttachments());
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, const VkRect2D & renderArea, bool inlineFirstSubpass, 
	const glm::vec4 & clearColor, float clearDepth, uint32_t clearStencil)
{
	std::array<glm::vec4, 4> clearColors;
	std::array<glm::float32, 4> clearDepths;
	std::array<uint32_t, 4> clearStencils;
	assert(fbo.getNumColorAttachments() <= clearColors.size());
	for (uint32_t i = 0; i < fbo.getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32_t i = 0; i < fbo.getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}

	beginRenderPass(fbo.getFrameBuffer(), renderPass.getRenderPass(), renderArea, inlineFirstSubpass, clearColors.data(), fbo.getNumColorAttachments(),
		clearDepths.data(), clearStencils.data(), fbo.getNumDepthStencilAttachments());
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, const VkRect2D & renderArea, bool inlineFirstSubpass, 
	const glm::vec4 * clearColors, uint32_t numClearColors, float * clearDepth, uint32_t * clearStencil, uint32_t numClearDepthStencil)
{
	beginRenderPass(fbo.getFrameBuffer(), renderPass.getRenderPass(), renderArea, inlineFirstSubpass, clearColors, numClearColors, clearDepth, 
		clearStencil, numClearDepthStencil);
}

void VulkanCommandBuffer::beginRenderPass(VulkanFrameBuffer & fbo, const VulkanRenderPass & renderPass, bool inlineFirstSubpass, const glm::vec4 & clearColor, 
	float clearDepth, uint32_t clearStencil)
{
	std::array<glm::vec4, 4> clearColors;
	std::array<glm::float32, 4> clearDepths;
	std::array<uint32_t, 4> clearStencils;
	assert(fbo.getNumColorAttachments() <= clearColors.size());
	for (uint32_t i = 0; i < fbo.getNumColorAttachments(); ++i)
	{
		clearColors[i] = clearColor;
	}

	for (uint32_t i = 0; i < fbo.getNumDepthStencilAttachments(); ++i)
	{
		clearDepths[i] = clearDepth;
		clearStencils[i] = clearStencil;
	}
	beginRenderPass(fbo.getFrameBuffer(), renderPass.getRenderPass(), fbo.getDimensions(),
		inlineFirstSubpass, clearColors.data(), fbo.getNumColorAttachments(), clearDepths.data(), clearStencils.data(),
		fbo.getNumDepthStencilAttachments());
}

void VulkanCommandBuffer::endRenderPass()
{
	vkCmdEndRenderPass(m_cmdBuffer);
}

void VulkanCommandBuffer::updateBuffer(VulkanHardwareBuffer & buffer, const void * data, uint32_t offset, uint32_t length)
{
}

void VulkanCommandBuffer::copyBuffer(VulkanHardwareBuffer src, VulkanHardwareBuffer dest, uint32_t srcOffset, uint32_t destOffset, uint32_t sizeInBytes)
{
}

void VulkanCommandBuffer::drawIndexed(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexOffset, uint32_t firstInstance, uint32_t instanceCount)
{
	vkCmdDrawIndexed(m_cmdBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::drawArrays(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
	vkCmdDraw(m_cmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

// Todo: Need to refactor the VulkanHardwareBuffer, VulkanHardwareVertexBuffer, VulkanHardwareIndexBuffer, ?VulkanHardwareIndirectBuffer?
void VulkanCommandBuffer::drawArraysIndirect(VulkanHardwareBuffer & buffer, uint32_t offset, uint32_t drawCount, uint32_t stride)
{
	// Todo: How to get the native VkBuffer
	//vkCmdDrawIndirect(m_cmdBuffer, , offset, drawCount, stride);
}

void VulkanCommandBuffer::drawIndexedIndirect(VulkanHardwareIndexBuffer & buffer)
{
	// Todo: How to get the native index VkBuffer
	//objectRefs.push_back(buffer);
	//vkCmdDrawIndexedIndirect(m_cmdBuffer, , 0, 1, 0);
}

void VulkanCommandBuffer::drawIndirect(VulkanHardwareBuffer & buffer, uint32_t offset, uint32_t count, uint32_t stride)
{
	// Todo: How to get the native indirect VkBuffer
	//objectRefs.push_back(buffer);
	//vkCmdDrawIndirect(m_cmdBuffer, , offset, count, stride);
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

void VulkanCommandBuffer::beginRenderPass(VkFramebuffer fbo, VkRenderPass renderPass, const VkRect2D & renderArea, bool inlineFirstSubpass, const glm::vec4 * clearColors, uint32_t numClearColors, float * clearDepth, uint32_t * clearStencil, uint32_t numClearDepthStencil)
{
	VkRenderPassBeginInfo nfo = {};
	nfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	std::vector<VkClearValue> clearValues(numClearColors + numClearDepthStencil);
	uint32_t i = 0;
	for (; i < numClearColors; ++i)
	{
		memcpy(clearValues[i].color.float32, &clearColors[i], sizeof(float) * 4);
	}
	for (numClearDepthStencil += numClearColors; i < numClearDepthStencil; ++i)
	{
		clearValues[i].depthStencil.depth = clearDepth[i - numClearColors];
		clearValues[i].depthStencil.stencil = clearStencil[i - numClearColors];
	}
	nfo.pClearValues = clearValues.data();
	nfo.clearValueCount = (uint32_t)clearValues.size();
	nfo.framebuffer = fbo;
	nfo.renderArea = renderArea;
	nfo.renderPass = renderPass;

	vkCmdBeginRenderPass(m_cmdBuffer, &nfo, inlineFirstSubpass ?
		VK_SUBPASS_CONTENTS_INLINE : VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
}
