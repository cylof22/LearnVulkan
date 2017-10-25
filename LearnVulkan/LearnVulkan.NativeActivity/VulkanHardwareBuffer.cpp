#include "pch.h"
#include "VulkanGraphicContext.h"
#include "VulkanMemoryMgr.h"
#include "VulkanHardwareBuffer.h"


VulkanHardwareBuffer::VulkanHardwareBuffer(VulkanGraphicContext* pContext, VkImageUsageFlags usage, VkMemoryPropertyFlagBits memoryProp, const void * pBufferData, uint32_t bufferSize)
	: m_bufferUsage(usage), m_pContext(pContext)
{
	VkResult res;
	VkBufferCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	info.pNext = nullptr;
	info.usage = m_bufferUsage;
	info.size = bufferSize;
	info.queueFamilyIndexCount = 0;
	info.pQueueFamilyIndices = nullptr;
	info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	info.flags = 0;

	const VkDevice& rDevice = m_pContext->getDevice();

	res = vkCreateBuffer(rDevice, &info, VK_ALLOC_CALLBACK, &m_buffer);

	VkMemoryRequirements memRequires;
	vkGetBufferMemoryRequirements(rDevice, m_buffer, &memRequires);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.pNext = nullptr;
	allocInfo.memoryTypeIndex = 0;
	allocInfo.allocationSize = memRequires.size;

	// VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	bool isOk = VulkanMemoryMgr::get()->memoryTypeFromProperties(m_pContext->getGPU(), memRequires.memoryTypeBits,
		memoryProp, allocInfo.memoryTypeIndex);

	if (isOk)
	{
		res = vkAllocateMemory(rDevice, &allocInfo, VK_ALLOC_CALLBACK, &m_memory);
		assert(res == VK_SUCCESS);

		void* pData;
		res = vkMapMemory(rDevice, m_memory, 0, memRequires.size, 0, &pData);

		assert(res == VK_SUCCESS);

		memcpy(pData, pBufferData, bufferSize);

		vkUnmapMemory(rDevice, m_memory);

		res = vkBindBufferMemory(rDevice, m_buffer, m_memory, 0);
	}
	assert(res == VK_SUCCESS)
}


VulkanHardwareBuffer::~VulkanHardwareBuffer()
{
	if (m_buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(m_pContext->getDevice(), m_buffer, VK_ALLOC_CALLBACK);
		m_buffer = VK_NULL_HANDLE;
	}

	if (m_memory != VK_NULL_HANDLE)
	{
		vkFreeMemory(m_pContext->getDevice(), m_memory, VK_ALLOC_CALLBACK);
		m_memory = VK_NULL_HANDLE;
	}
}
