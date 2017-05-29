#pragma once
#include "common\vulkan_wrapper.h"

class VkCommandBufferMgr
{
public:
	static VkCommandBufferMgr* get();

	VkResult createCommandBuffer(const VkDevice* pDevice, const VkCommandPool& cmdPool, VkCommandBuffer* pCmdBuffer,
		const VkCommandBufferAllocateInfo* cmdBufferInfo = nullptr);

	VkResult beginCommandBuffer(const VkCommandBuffer* pCmdbuffer, const VkCommandBufferBeginInfo* pBeginCmdBufferInfo = nullptr);

	VkResult endCommandBuffer(const VkCommandBuffer* pCmdBuffer);

	VkResult submitCommandBuffer(const VkQueue& queue, const VkCommandBuffer* pCmdBuffer, const VkSubmitInfo* pInfo = nullptr, const VkFence& fence = VK_NULL_HANDLE);

	VkResult destroyCommandBuffer(const VkDevice* pDevice, const VkCommandPool& cmdPool, const VkCommandBuffer* pCmdBuffer);
private:
	static VkCommandBufferMgr* m_pMgr;
};