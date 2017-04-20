#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice * pDevice) : m_pGPU(pDevice), m_pDevice(nullptr), m_pGraphicQueue(nullptr), m_queueFamilySize(0)
{
	if (pDevice != nullptr)
	{
		vkGetPhysicalDeviceMemoryProperties(*pDevice, &m_gpuMemoryProperties);
	}
}

VulkanDevice::~VulkanDevice()
{
	if (m_pDevice != nullptr)
	{

	}
}

VkResult VulkanDevice::createDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	VkResult res = VK_SUCCESS;

	float queuePriority = 0.0;
	// Device queue information
	VkDeviceQueueCreateInfo queueInfo = { };
	queueInfo.flags = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.queueFamilyIndex = m_graphicQueueIndex;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &queuePriority;

	// Device creation information
	VkDeviceCreateInfo deviceInfo = { };
	deviceInfo.flags = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.pNext = nullptr;
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;
	deviceInfo.enabledLayerCount = layers.size();
	deviceInfo.ppEnabledLayerNames = layers.data();
	deviceInfo.enabledExtensionCount = extensions.size();
	deviceInfo.ppEnabledExtensionNames = extensions.data();

	res = vkCreateDevice(*m_pGPU, &deviceInfo, nullptr, m_pDevice);
	return res;
}

VkResult VulkanDevice::initDevice(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	VkResult res = VK_SUCCESS;
	res = initDeviceQueue(layers, extensions);
	if (res == VK_SUCCESS)
		initGraphicQueue();
	return res;
}

void VulkanDevice::initGraphicQueue()
{
	vkGetDeviceQueue(*m_pDevice, m_graphicQueueIndex, 0, m_pGraphicQueue);
}

VkResult VulkanDevice::destroyDevice()
{
	VkResult res = VK_SUCCESS;
	return res;
}

bool VulkanDevice::getGraphicQueueHandle()
{
	bool found = false;
	for (size_t i = 0; i < m_queueFamilySize; i++)
	{
		if (m_queueProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			found = true;
			m_graphicQueueIndex = i;
		}
	}

	return found;
}

VkResult VulkanDevice::initDeviceQueue(const std::vector<const char*>& layers, const std::vector<const char*>& extensions)
{
	VkResult res = VK_SUCCESS;
	getGpuQueuesAndProperties();
	if (getGraphicQueueHandle())
		res = createDevice(layers, extensions);

	return res;
}

void VulkanDevice::getGpuQueuesAndProperties()
{
	vkGetPhysicalDeviceQueueFamilyProperties(*m_pGPU, &m_queueFamilySize, nullptr);
	if (m_queueFamilySize != 0)
	{
		m_queueProperties.resize(m_queueFamilySize);
		vkGetPhysicalDeviceQueueFamilyProperties(*m_pGPU, &m_queueFamilySize, m_queueProperties.data());
	}
}
