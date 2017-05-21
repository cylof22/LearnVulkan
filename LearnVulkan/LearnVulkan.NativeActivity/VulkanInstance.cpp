#include "VulkanInstance.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "AndroidProject1.NativeActivity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "AndroidProject1.NativeActivity", __VA_ARGS__))

VKAPI_ATTR VkBool32 VKAPI_CALL debugFunction(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObj, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData)
{
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		LOGI("[VK_DEBUG_REPORT] ERROR: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		LOGI("[VK_DEBUG_REPORT] WARNING: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		LOGI("[VK_DEBUG_REPORT] INFORMATION: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		LOGI("[VK_DEBUG_REPORT] PERFORMANCE: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		LOGI("[VK_DEBUG_REPORT] VALIDATION: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else
		return VK_FALSE;

	return VK_SUCCESS;
}

VulkanInstance::VulkanInstance()
{

}

VulkanInstance::~VulkanInstance()
{
	vkDestroyDebugReportCallbackEXT(m_instance, m_debugReportCallback, nullptr);
	vkDestroyInstance(m_instance, nullptr);
}

VkResult VulkanInstance::createInstance(const std::vector<const char*>& layers, const std::vector<const char*>& extensions, const char * appName)
{
	VkResult res = VK_SUCCESS;

	// set the application information
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = appName;
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// set the instance information
	VkInstanceCreateInfo instanceInfo = {};
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

	if (res == VK_SUCCESS)
	{
		vkCreateDebugReportCallbackEXT  = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
		vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(m_instance, "vkDebugReportMessageEXT");

		dbgReportCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		dbgReportCallbackInfo.pNext = nullptr;
		dbgReportCallbackInfo.pUserData = nullptr;
		dbgReportCallbackInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
			VK_DEBUG_REPORT_ERROR_BIT_EXT |
			VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		dbgReportCallbackInfo.pfnCallback = debugFunction;

		res = vkCreateDebugReportCallbackEXT(m_instance, &dbgReportCallbackInfo, nullptr, &m_debugReportCallback);
	}
	

	return res;
}
