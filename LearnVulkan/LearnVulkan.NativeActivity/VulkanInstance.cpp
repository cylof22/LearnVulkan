#include "VulkanInstance.h"

VulkanInstance::VulkanInstance()
{

}

VulkanInstance::~VulkanInstance()
{
	vkDestroyInstance(m_instance, nullptr);
}

VkResult VulkanInstance::createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const char * appName)
{
	VkResult res = VK_SUCCESS;

	// set the application information
	VkApplicationInfo appInfo;
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = appName;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// set the instance information
	VkInstanceCreateInfo instanceInfo;
	// set the specific layer and extension information
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledLayerCount = layers.size();
	instanceInfo.ppEnabledLayerNames = layers.data();
	instanceInfo.enabledExtensionCount = extensions.size();
	instanceInfo.ppEnabledExtensionNames = extensions.data();

	res = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
	return res;
}
