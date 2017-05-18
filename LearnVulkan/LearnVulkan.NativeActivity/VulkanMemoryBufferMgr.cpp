#include "VulkanMemoryBufferMgr.h"
#include "common\vulkan_wrapper.h"
#include "VulkanDevice.h"
#include "VulkanMemoryMgr.h"
#include "VulkanHardwareVertexBuffer.h"

VulkanMemoryBufferMgr* VulkanMemoryBufferMgr::m_pMgr = nullptr;


VulkanMemoryBufferMgr::VulkanMemoryBufferMgr()
{
}

VulkanMemoryBufferMgr::~VulkanMemoryBufferMgr()
{
}

VulkanMemoryBufferMgr * VulkanMemoryBufferMgr::get()
{
	if (m_pMgr == nullptr)
		m_pMgr = new VulkanMemoryBufferMgr();

	return m_pMgr;
}

bool VulkanMemoryBufferMgr::createVertexBuffer(const VkPhysicalDevice* rGPU, const VkDevice& device, const void * pVertexData, uint32_t vertexSize, uint32_t stride,
	VkBuffer& vertexBuffer, VkDeviceMemory& vertexMemory)
{
	VkResult res;
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	info.size = vertexSize;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.flags = 0;

	res = vkCreateBuffer(device, &info, nullptr, &vertexBuffer);

	VkMemoryRequirements memRequires;
	vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequires);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRequires.size;

	bool isOk = VulkanMemoryMgr::get()->memoryTypeFromProperties(rGPU, memRequires.memoryTypeBits, 
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, allocInfo.memoryTypeIndex);

	if (isOk)
	{
		res = vkAllocateMemory(device, &allocInfo, nullptr, &vertexMemory);
		assert(res == VK_SUCCESS);

		void* pData;
		res = vkMapMemory(device, vertexMemory, 0, memRequires.size, 0, &pData);

		assert(res == VK_SUCCESS);

		memcpy(pData, pVertexData, vertexSize);

		vkUnmapMemory(device, vertexMemory);

		res = vkBindBufferMemory(device, vertexBuffer, vertexMemory, 0);

		return true;
	}

	return false;
}

bool VulkanMemoryBufferMgr::createIndexBuffer(const VkPhysicalDevice& rGPU, const VkDevice& device, const void * pIndexData, uint32_t indexSize, VkBuffer& indexBuffer, VkDeviceMemory& indexMemory)
{
	VkResult res;
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	info.size = indexSize;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.flags = 0;

	res = vkCreateBuffer(device, &info, nullptr, &indexBuffer);

	VkMemoryRequirements memRequires;
	vkGetBufferMemoryRequirements(device, indexBuffer, &memRequires);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRequires.size;

	bool isOk = VulkanMemoryMgr::get()->memoryTypeFromProperties(&rGPU, memRequires.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, allocInfo.memoryTypeIndex);

	if (isOk)
	{
		res = vkAllocateMemory(device, &allocInfo, nullptr, &indexMemory);
		assert(res == VK_SUCCESS);

		void* pData;
		res = vkMapMemory(device, indexMemory, 0, memRequires.size, 0, &pData);

		assert(res == VK_SUCCESS);

		memcpy(pData, pIndexData, indexSize);

		vkUnmapMemory(device, indexMemory);

		res = vkBindBufferMemory(device, indexBuffer, indexMemory, 0);

		return true;
	}

	return false;
}

bool VulkanMemoryBufferMgr::createUniformBuffer(const VkPhysicalDevice & rGPU, const VkDevice & device, const void * pUniformData, uint32_t uniformSize, VkBuffer & uniformBuffer, VkDeviceMemory & uniformMemory)
{
	VkResult res;
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	info.size = uniformSize;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.flags = 0;

	res = vkCreateBuffer(device, &info, nullptr, &uniformBuffer);

	VkMemoryRequirements memRequires;
	vkGetBufferMemoryRequirements(device, uniformBuffer, &memRequires);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRequires.size;

	bool isOk = VulkanMemoryMgr::get()->memoryTypeFromProperties(&rGPU, memRequires.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, allocInfo.memoryTypeIndex);

	if (isOk)
	{
		res = vkAllocateMemory(device, &allocInfo, nullptr, &uniformMemory);
		assert(res == VK_SUCCESS);

		void* pData;
		res = vkMapMemory(device, uniformMemory, 0, memRequires.size, 0, &pData);

		assert(res == VK_SUCCESS);

		memcpy(pData, pUniformData, uniformSize);

		VkMappedMemoryRange uniformMappedRange = {};
		uniformMappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		uniformMappedRange.pNext = nullptr;
		uniformMappedRange.memory = uniformMemory;
		uniformMappedRange.offset = 0;
		uniformMappedRange.size = uniformSize;

		// for non-coherent memory, use the follow function to unmap the memory
		vkInvalidateMappedMemoryRanges(device, 1, &uniformMappedRange);

		res = vkBindBufferMemory(device, uniformBuffer, uniformMemory, 0);

		return true;
	}

	return false;
}
