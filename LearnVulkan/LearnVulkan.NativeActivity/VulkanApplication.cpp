#include "pch.h"
#include "common\vulkan_wrapper.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanApplication.h"

VulkanApplication::VulkanApplication() : m_instance(nullptr)
{
}

VulkanApplication::~VulkanApplication()
{
	if (m_instance != nullptr)
	{
		delete m_instance;
		m_instance = nullptr;
	}
}

bool VulkanApplication::createVulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	std::vector<VkPhysicalDevice> gpuList;

	if (m_instance == nullptr)
		m_instance = new VulkanInstance;

	VkResult res = m_instance->createInstance(layers, extensions, "VulkanEngine");

	if (res == VK_SUCCESS)
		getPhysicalDevices(gpuList);

	if (!gpuList.empty())
	{
		res = handShakeWithDevice(&gpuList[0], layers, extensions);
	}
	return res == VK_SUCCESS;
}

VkResult VulkanApplication::getPhysicalDevices(std::vector<VkPhysicalDevice>& gpus)
{
	VkResult res = VK_SUCCESS;
	uint32_t gpuDeviceSize = 0;
	res = vkEnumeratePhysicalDevices(m_instance->getInstance(), &gpuDeviceSize, nullptr);

	if (gpuDeviceSize != 0)
	{
		gpus.resize(gpuDeviceSize);
		res = vkEnumeratePhysicalDevices(m_instance->getInstance(), &gpuDeviceSize, gpus.data());
	}
	return res;
}

VkResult VulkanApplication::handShakeWithDevice(VkPhysicalDevice * pGPU, const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	VkResult res = VK_SUCCESS;
	if (pGPU != nullptr)
	{
		m_device = new VulkanDevice(pGPU);
		if (m_device != nullptr)
		{
			// print the available extension in debug
			VulkanLayerAndExtension layerAndExtensions;
			std::vector<VulkanLayerAndExtension::LayerProperties> deviceLayerProps;
			layerAndExtensions.getDeviceExtensionProperties(deviceLayerProps, pGPU);

			std::vector<const char*> deviceLayers;
			std::vector<const char*> deviceExtensions;
			for (const auto& layerProp : deviceLayerProps)
			{
				deviceLayers.emplace_back(layerProp.m_properties.layerName);

				for (const auto& extension : layerProp.m_extensions)
					deviceExtensions.emplace_back(extension.extensionName);
			}
			res = m_device->initDevice(deviceLayers, deviceExtensions);
		}
		else
		{
			res = VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		
	}
	return res;
}
