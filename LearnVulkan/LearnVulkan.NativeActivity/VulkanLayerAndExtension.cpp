#include "VulkanLayerAndExtension.h"
#include <iostream>

VkResult VulkanLayerAndExtension::getInstanceExtensionProperties(std::vector<LayerProperties>& globalLayerPropSet)
{
	// get all the layers
	uint32_t instanceLayerCount = 0;
	std::vector<VkLayerProperties> layerProperties;
	VkResult res;
	do 
	{
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, nullptr);
		if (res != VK_SUCCESS)
			return res;

		if (instanceLayerCount == 0)
			break;

		layerProperties.resize(instanceLayerCount);
		res = vkEnumerateInstanceLayerProperties(&instanceLayerCount, layerProperties.data());
	} while (res == VK_INCOMPLETE);

	// query all the extensions for each layer
	std::cout << "\nInstanced Layers" << std::endl;
	std::cout << "==================" << std::endl;
	for (auto globalLayerProp : layerProperties)
	{
		std::cout << "\n" << globalLayerProp.description
			<< "\n\t|\n\t---[Layer Name]--->"
			<< globalLayerProp.layerName << "\n";

		LayerProperties instanceLayerProps;
		instanceLayerProps.m_properties = globalLayerProp;

		res = getExtension(instanceLayerProps);
		if (res != VK_SUCCESS)
			continue;

		globalLayerPropSet.emplace_back(instanceLayerProps);

		for (auto extension : instanceLayerProps.m_extensions)
		{
			std::cout << "\t\t|\n\t\t|===[Layer Extension]-->"
				<< extension.extensionName << "\n";
		}
	}

	return res;
}

VkResult VulkanLayerAndExtension::getDeviceExtensionProperties(std::vector<LayerProperties>& deviceLayerProps, VkPhysicalDevice* gpu)
{
	VkResult res = VK_SUCCESS;

	// query all the extensions for each layer
	std::cout << "\nDevice Layers" << std::endl;
	std::cout << "==================" << std::endl;

	std::vector<LayerProperties> instanceLayerProp;
	getInstanceExtensionProperties(instanceLayerProp);
	for (auto globalLayerProp : instanceLayerProp) {
		LayerProperties layerProps;
		layerProps.m_properties = globalLayerProp.m_properties;
		res = getExtension(layerProps, gpu);
		if (res != VK_SUCCESS)
			continue;

		std::cout << "\n" << globalLayerProp.m_properties.description << "\n\t|\n\t|---[Layer Name]--> " << globalLayerProp.m_properties.layerName << "\n";
		deviceLayerProps.push_back(layerProps);

		if (layerProps.m_extensions.size()) {
			for (auto j : layerProps.m_extensions) {
				std::cout << "\t\t|\n\t\t|---[Device Extesion]--> " << j.extensionName << "\n";
			}
		}
		else {
			std::cout << "\t\t|\n\t\t|---[Device Extesion]--> No extension found \n";
		}
	}


	return res;
}

VkResult VulkanLayerAndExtension::getExtension(LayerProperties& layerProps, VkPhysicalDevice* gpu /*= nullptr*/)
{
	uint32_t extensionCount = 0;
	VkResult res;

	char* layerName = layerProps.m_properties.layerName;

	do
	{
		if (gpu)
			res = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, nullptr);
		else
			res = vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

		if (res != VK_SUCCESS || extensionCount == 0)
			continue;

		layerProps.m_extensions.resize(extensionCount);

		if (gpu)
			res = vkEnumerateDeviceExtensionProperties(*gpu, layerName, &extensionCount, layerProps.m_extensions.data());
		else
			res = vkEnumerateInstanceExtensionProperties(layerName, &extensionCount, layerProps.m_extensions.data());
	} while (res == VK_INCOMPLETE);

	return res;
}