#include "pch.h"
#include "common\vulkan_wrapper.h"
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "VulkanRenderer.h"
#include "VulkanApplication.h"

VulkanApplication::VulkanApplication() : m_pInstance(nullptr), m_pDevice(nullptr), m_pRender(nullptr)
{
}

VulkanApplication::~VulkanApplication()
{
	if (m_pInstance != nullptr)
	{
		delete m_pInstance;
		m_pInstance = nullptr;
	}
}

bool VulkanApplication::createVulkanInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, ANativeWindow* pWnd)
{
	std::vector<VkPhysicalDevice> gpuList;

	if (m_pInstance == nullptr)
		m_pInstance = new VulkanInstance;

	VkResult res = m_pInstance->createInstance(layers, extensions, "VulkanEngine");

	if (res == VK_SUCCESS)
		getPhysicalDevices(gpuList);

	if (!gpuList.empty())
	{
		res = handShakeWithDevice(&gpuList[0], layers, extensions, pWnd);
	}
	return res == VK_SUCCESS;
}

VkResult VulkanApplication::getPhysicalDevices(std::vector<VkPhysicalDevice>& gpus)
{
	VkResult res = VK_SUCCESS;
	uint32_t gpuDeviceSize = 0;
	res = vkEnumeratePhysicalDevices(m_pInstance->getInstance(), &gpuDeviceSize, nullptr);

	if (gpuDeviceSize != 0)
	{
		gpus.resize(gpuDeviceSize);
		res = vkEnumeratePhysicalDevices(m_pInstance->getInstance(), &gpuDeviceSize, gpus.data());
	}
	return res;
}

VkResult VulkanApplication::handShakeWithDevice(VkPhysicalDevice * pGPU, const std::vector<const char*>& layers, const std::vector<const char*>& extensions, ANativeWindow* pWnd)
{
	VkResult res = VK_SUCCESS;
	if (pGPU != nullptr)
	{
		m_pDevice = new VulkanDevice(pGPU);
		if (m_pDevice != nullptr)
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

			deviceExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
			res = m_pDevice->initDevice(deviceLayers, deviceExtensions);

			if (res == VK_SUCCESS)
			{
				m_pRender = new VulkanRenderer(m_pInstance,m_pDevice);
				if (m_pRender)
					m_pRender->init(pWnd);
			}
		}
		else
		{
			res = VK_ERROR_OUT_OF_HOST_MEMORY;
		}
		
	}
	return res;
}
