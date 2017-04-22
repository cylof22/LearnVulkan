#pragma once
#include <vector>
#include "common\vulkan_wrapper.h"

class VulkanDevice
{
public:
	VulkanDevice(VkPhysicalDevice* pDevice);
	~VulkanDevice();

	VkResult initDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
	const VkQueue& getGraphicQueue() { return m_graphicQueue; }
protected:
	void initGraphicQueue();
	VkResult createDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
	VkResult destroyDevice();

	bool getGraphicQueueHandle();
	VkResult initDeviceQueue(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	void getGpuQueuesAndProperties();

private:
	VkPhysicalDevice* m_pGPU;
	VkDevice         m_device;
	VkQueue m_graphicQueue;

	// VkPhysicalDeviceProperties m_gpuProperties;
	VkPhysicalDeviceMemoryProperties m_gpuMemoryProperties;

	std::vector<VkQueueFamilyProperties> m_queueProperties;

	uint32_t m_graphicQueueIndex;
	uint32_t m_queueFamilySize;
};
