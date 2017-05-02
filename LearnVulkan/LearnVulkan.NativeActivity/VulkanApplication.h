#pragma once
#include <vector>

class VulkanInstance;
class VulkanDevice;
class VulkanRenderer;

class VulkanApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	bool createVulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, ANativeWindow* pWnd);

	VkResult getPhysicalDevices(std::vector<VkPhysicalDevice>& gpus);

	VkResult handShakeWithDevice(VkPhysicalDevice* pGPU, const std::vector<const char*>& layers, const std::vector<const char*>& extensions, ANativeWindow* pWnd);

	const VulkanInstance* getVulkanInstance() { return m_pInstance; }

	VulkanRenderer* getRender() { return m_pRender; }
private:
	VulkanInstance* m_pInstance;
	VulkanDevice* m_pDevice;
	VulkanRenderer* m_pRender;
};
