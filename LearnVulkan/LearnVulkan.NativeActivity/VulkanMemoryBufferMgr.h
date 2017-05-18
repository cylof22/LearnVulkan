#pragma once
#include "common\vulkan_wrapper.h"
class VulkanHardwareVertexBuffer;
class VulkanHardwareIndexBuffer;
class VulkanDevice;

class VulkanMemoryBufferMgr
{
public:
	VulkanMemoryBufferMgr();
	~VulkanMemoryBufferMgr();

	static VulkanMemoryBufferMgr* get();

	bool createVertexBuffer(const VkPhysicalDevice* pGPU, const VkDevice& device, const void* pVertexData, uint32_t size, uint32_t stride,
		VkBuffer& vertexBuffer, VkDeviceMemory& vertexMemory);

	bool createIndexBuffer(const VkPhysicalDevice& rGPU, const VkDevice& device, const void* pIndexData, uint32_t indexSize, 
		VkBuffer& indexBuffer, VkDeviceMemory& indexMemory);

	bool createUniformBuffer(const VkPhysicalDevice& rGPU, const VkDevice& device, const void* pUniformData, uint32_t uniformSize,
		VkBuffer& uniformBuffer, VkDeviceMemory& uniformMemory);

private:
	static VulkanMemoryBufferMgr* m_pMgr;
};