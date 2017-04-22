#pragma once
#include "common\vulkan_wrapper.h"

class VkCommandBufferMgr
{
public:
	static VkCommandBufferMgr* get();

	VkResult createCommandBuffer(const VkDevice* pDevice, const VkCommandPool& cmdPool, VkCommandBuffer* pCmdBuffer,
		const VkCommandBufferAllocateInfo* cmdBufferInfo);

	VkResult beginCommandBuffer(const VkCommandBuffer* pCmdbuffer, const VkCommandBufferBeginInfo* pBeginCmdBufferInfo);

	VkResult endCommandBuffer(const VkCommandBuffer* pCmdBuffer);

	VkResult submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* pCmdBuffer, const VkSubmitInfo* pInfo, const VkFence& fence);
private:
	static VkCommandBufferMgr* m_pMgr;
};