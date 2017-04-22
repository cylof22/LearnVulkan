#include "VkCommandBufferMgr.h"

VkCommandBufferMgr* VkCommandBufferMgr::m_pMgr = nullptr;

VkCommandBufferMgr * VkCommandBufferMgr::get()
{
	if (m_pMgr == nullptr)
		m_pMgr = new VkCommandBufferMgr();
	return nullptr;
}

VkResult VkCommandBufferMgr::createCommandBuffer(const VkDevice * pDevice, const VkCommandPool & cmdPool, VkCommandBuffer * pCmdBuffer, 
	const VkCommandBufferAllocateInfo * cmdBufferInfo)
{
	VkResult res = VK_SUCCESS;
	if (cmdBufferInfo)
	{
		res = vkAllocateCommandBuffers(*pDevice, cmdBufferInfo, pCmdBuffer);
	}
	else
	{
		VkCommandBufferAllocateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext = nullptr;
		info.commandPool = cmdPool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		info.commandBufferCount = (uint32_t)(sizeof(pCmdBuffer) / sizeof(VkCommandBuffer));
		
		res = vkAllocateCommandBuffers(*pDevice, &info, pCmdBuffer);
	}
	return res;
}

VkResult VkCommandBufferMgr::beginCommandBuffer(const VkCommandBuffer * pCmdBuffer, const VkCommandBufferBeginInfo * pBeginCmdBufferInfo)
{
	VkResult res = VK_SUCCESS;
	if (pBeginCmdBufferInfo)
	{
		res = vkBeginCommandBuffer(*pCmdBuffer, pBeginCmdBufferInfo);
	}
	else
	{
		VkCommandBufferInheritanceInfo inheritanceInfo;
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.pNext = nullptr;
		inheritanceInfo.renderPass = VK_NULL_HANDLE;
		inheritanceInfo.subpass = 0;
		inheritanceInfo.framebuffer = VK_NULL_HANDLE;
		inheritanceInfo.occlusionQueryEnable = VK_FALSE;
		inheritanceInfo.queryFlags = 0;
		inheritanceInfo.pipelineStatistics = 0;

		VkCommandBufferBeginInfo info;
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext = nullptr;
		info.flags = 0;
		info.pInheritanceInfo = &inheritanceInfo;

		res = vkBeginCommandBuffer(*pCmdBuffer, &info);
	}
	return res;
}

VkResult VkCommandBufferMgr::endCommandBuffer(const VkCommandBuffer * pCmdBuffer)
{
	return vkEndCommandBuffer(*pCmdBuffer);
}

VkResult VkCommandBufferMgr::submitCommandBuffer(const VkQueue & queue, const VkCommandBuffer* pCmdBuffer, const VkSubmitInfo * pInfo, const VkFence & fence)
{
	VkResult res = VK_SUCCESS;
	if (pInfo)
		vkQueueSubmit(queue, 1, pInfo, fence);

	VkSubmitInfo info;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;
	info.waitSemaphoreCount = 0;
	info.pWaitSemaphores = nullptr;
	info.pWaitDstStageMask = nullptr;
	info.commandBufferCount = (uint32_t)(sizeof(pCmdBuffer) / sizeof(VkCommandBuffer));
	info.pCommandBuffers = pCmdBuffer;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;

	vkQueueSubmit(queue, 1, &info, fence);

	res = vkQueueWaitIdle(queue);
	return res;
}
