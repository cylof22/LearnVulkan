#pragma once
#include <vector>

class VulkanInstance;
class VulkanApplication
{
public:
	VulkanApplication();
	~VulkanApplication();

	bool createVulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions);
private:
	VulkanInstance* m_instance;
};
