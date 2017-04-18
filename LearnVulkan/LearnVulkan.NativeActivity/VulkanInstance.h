#pragma once

#include "common\vulkan_wrapper.h"
#include "VulkanLayerAndExtension.h"
#include <vector>

class VulkanInstance
{
public:
	VulkanInstance();
	~VulkanInstance();

	VkResult createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const char* appName);
private:
	VkInstance m_instance;
};