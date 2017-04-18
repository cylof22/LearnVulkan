#pragma once
#include <vector>
#include "common\vulkan_wrapper.h"

class VulkanLayerAndExtension 
{
public:
	struct LayerProperties
	{
		VkLayerProperties m_properties;
		std::vector<VkExtensionProperties> m_extensions;
	};

	VkResult getInstanceExtensionProperties(std::vector<LayerProperties>& globalLayerPropSet);
	VkResult getDeviceExtensionProperties(std::vector<LayerProperties>& deviceLayerProps, VkPhysicalDevice* gpu);
private:
	VkResult getExtension(LayerProperties& layerProps, VkPhysicalDevice* gpu = nullptr);
};