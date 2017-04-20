#pragma once
#include <vector>

class VulkanInstance;
class VulkanDevice;

class VulkanApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	bool createVulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

	VkResult getPhysicalDevices(std::vector<VkPhysicalDevice>& gpus);

	VkResult handShakeWithDevice(VkPhysicalDevice* pGPU, const std::vector<const char*>& layers, const std::vector<const char*>& extensions);

private:
	VulkanInstance* m_instance;
	VulkanDevice* m_device;
};
