#include "pch.h"
#include <vector>
#include "VulkanGraphicContext.h"
#include "VulkanRenderWindow.h"
#include "VulkanFrameBuffer.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR
VulkanGraphicContext::VulkanGraphicContext(ANativeWindow* pWnd)
	:m_pWnd(pWnd)
{
}
#elif VK_USE_PLATFORM_WIN32_KHR
VulkanGraphicContext::VulkanGraphicContext(HWND pWnd)
	:m_pWnd(pWnd)
{
}
#endif // VK_USE_PLATFORM_ANDROID_KHR

VulkanGraphicContext::~VulkanGraphicContext()
{
}

bool VulkanGraphicContext::initVkInstance()
{
	std::vector<const char*> extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME
		, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
#ifdef DEBUG
		, VK_EXT_DEBUG_REPORT_EXTENSION_NAME
#endif
	};

	std::vector<const char*> layers = {
#ifdef DEBUG
		"VK_LAYER_GOOGLE_threading"
		,"VK_LAYER_GOOGLE_unique_objects"
		,"VK_LAYER_LUNARG_parameter_validation"
		,"VK_LAYER_LUNARG_object_tracker"
		,"VK_LAYER_LUNARG_image"
		,"VK_LAYER_LUNARG_core_validation"
		,"VK_LAYER_LUNARG_swapchain"
		//,"VK_LAYER_LUNARG_api_dump"
		//,"VK_LAYER_LUNARG_device_limits"
#endif
	};

	// set the application information
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = "Dragon";
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

	VkResult res = vkCreateInstance(&instanceInfo, nullptr, &m_instance);
	return res == VK_SUCCESS;
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugFunctionContext(VkFlags msgFlags, VkDebugReportObjectTypeEXT objType, uint64_t srcObj, size_t location, int32_t msgCode, const char * pLayerPrefix, const char * pMsg, void * pUserData)
{
	if (msgFlags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		//LOGI("[VK_DEBUG_REPORT] ERROR: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		//LOGI("[VK_DEBUG_REPORT] WARNING: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		//LOGI("[VK_DEBUG_REPORT] INFORMATION: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		//LOGI("[VK_DEBUG_REPORT] PERFORMANCE: [ %s ] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else if (msgFlags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		//LOGI("[VK_DEBUG_REPORT] VALIDATION: [%s] Code %i : %s", pLayerPrefix, msgCode, pMsg);
	}
	else
		return VK_FALSE;

	return VK_SUCCESS;
}

bool VulkanGraphicContext::initDebugCallbacks()
{
	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = 
		(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
	//vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
	//vkDebugReportMessageEXT = (PFN_vkDebugReportMessageEXT)vkGetInstanceProcAddr(m_instance, "vkDebugReportMessageEXT");

	VkDebugReportCallbackCreateInfoEXT dbgReportCallbackInfo = {};
	dbgReportCallbackInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	dbgReportCallbackInfo.pNext = nullptr;
	dbgReportCallbackInfo.pUserData = nullptr;
	dbgReportCallbackInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
		VK_DEBUG_REPORT_ERROR_BIT_EXT |
		VK_DEBUG_REPORT_DEBUG_BIT_EXT;
	dbgReportCallbackInfo.pfnCallback = debugFunctionContext;

	VkResult res = vkCreateDebugReportCallbackEXT(m_instance, &dbgReportCallbackInfo, VK_ALLOC_CALLBACK, &m_debugReportCallback);

	return res == VK_SUCCESS;
}

bool VulkanGraphicContext::initPhysicalDevice()
{
	// detemrine the number of physical devices (gpus)
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
	//Log(Log.Information, "Number of Vulkan Physical devices: [%d]", physicalDeviceCount);
	uint32_t one = 1;
	vkEnumeratePhysicalDevices(m_instance, &one, &m_physicalDevice);
	return false;
}

static inline std::vector<const char*> getDeviceExtensions(VkPhysicalDevice device)
{
	uint32_t numItems = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &numItems, NULL);
	std::vector<VkExtensionProperties> extensions; 
	extensions.resize(numItems);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &numItems, extensions.data());

	std::vector<const char*> device_extension_names;
	for(const auto& extension : extensions)
		device_extension_names.emplace_back(extension.extensionName);

	return device_extension_names;
}

static inline std::vector<const char*> getInstanceExtensions(VkInstance instance)
{
	uint32_t numItems = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &numItems, NULL);
	std::vector<VkExtensionProperties> extensions;
	extensions.resize(numItems);
	vkEnumerateInstanceExtensionProperties(NULL, &numItems, extensions.data());

	std::vector<const char*> instance_extension_names;
	for (const auto& extension : extensions)
		instance_extension_names.emplace_back(extension.extensionName);

	return instance_extension_names;
}

bool VulkanGraphicContext::initLogicalDevice()
{
	std::vector<const char*> deviceExtensions = getDeviceExtensions(m_physicalDevice);

	// create the main queue
	uint32_t queueFamilySize = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilySize, nullptr);
	assert(queueFamilySize > 1);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilySize);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilySize, queueFamilyProperties.data());

	std::vector<VkBool32> supportsPresent(queueFamilySize);
	for (uint32_t i = 0; i < queueFamilySize; ++i)
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent[i]);

	for (uint32_t i = 0; i < queueFamilySize; ++i)
	{
		//Find the universal queue family index
		if (queueFamilyProperties[i].queueCount &&
			(queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0 &&
			(queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) != 0 &&
			(supportsPresent[i] == VK_TRUE))
		{
			m_universalQueueFamily = i;
			m_universalQueueCount = 1;
		}

		if (queueFamilyProperties[i].queueCount &&
			(queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) != 0)
		{
			m_transferQueueFamily = i;
			m_transferQueueCount = 1;
		}

		if (queueFamilyProperties[i].queueCount &&
			(queueFamilyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) != 0)
		{
			m_sparseQueueFamily = i;
			m_sparseQueueCount = 1;
		}

	}

	float queuePriority = 0.0f;
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	queueInfo.pNext = nullptr;
	queueInfo.flags = 0;
	queueInfo.pQueuePriorities = &queuePriority;

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfo;

	queueInfo.queueFamilyIndex = m_universalQueueFamily;
	queueInfo.queueCount = m_universalQueueCount;
	queueCreateInfo.emplace_back(queueInfo);

	if (m_transferQueueCount > 0)
	{
		queueInfo.queueFamilyIndex = m_transferQueueFamily;
		queueInfo.queueCount = m_transferQueueCount;
		queueCreateInfo.emplace_back(queueInfo);
	}

	if (m_sparseQueueCount > 0)
	{
		queueInfo.queueFamilyIndex = m_sparseQueueFamily;
		queueInfo.queueCount = m_sparseQueueCount;
		queueCreateInfo.emplace_back(queueInfo);
	}

	//Todo: How to create suitable queue and suitable queue size
	VkDeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.flags = 0;
	deviceCreateInfo.queueCreateInfoCount = queueCreateInfo.size();
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfo.data();

	VkResult res = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, VK_ALLOC_CALLBACK, &m_device);

	if (res == VK_SUCCESS)
	{
		vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_deviceMemProperties);

		vkGetDeviceQueue(m_device, m_universalQueueFamily, 0, &m_universalQueue);

		if(m_transferQueueCount > 0)
			vkGetDeviceQueue(m_device, m_transferQueueFamily, 0, &m_transferQueue);

		if(m_sparseQueueCount > 0)
			vkGetDeviceQueue(m_device, m_sparseQueueFamily, 0, &m_sparseQueue);

		return true;
	}
	return false;
}

bool VulkanGraphicContext::initSurface()
{
	VkResult res;
#if VK_USE_PLATFORM_ANDROID_KHR
	// create an android surface
	VkAndroidSurfaceCreateInfoKHR surfaceInfo;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.pNext = NULL;
	surfaceInfo.flags = 0;
	surfaceInfo.window = m_pWnd;
	res = vkCreateAndroidSurfaceKHR(m_instance, &surfaceInfo, VK_ALLOC_CALLBACK, &m_surface);
#elif VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
	surfaceCreateInfo.hwnd = (HWND)displayHandle.nativeWindow;
	surfaceCreateInfo.flags = 0;
	res = vkCreateWin32SurfaceKHR(platformHandle.context.instance, &surfaceCreateInfo, NULL, &displayHandle.surface);
#elif
	VkDisplayPropertiesKHR properties;
	uint32_t propertySize = 1;
	vkGetPhysicalDeviceDisplayPropertiesKHR(m_physicalDevice, &propertySize, &properties);
	m_display = properties.display;

#if DEBUG
	// output the log information

#endif
	
	// display mode
	uint32_t displayModeSize = 0;
	vkGetDisplayModePropertiesKHR(m_physicalDevice, m_display, &displayModeSize, nullptr);
	std::vector<VkDisplayModePropertiesKHR> modeProperties;
	modeProperties.resize(displayModeSize);
	vkGetDisplayModePropertiesKHR(m_physicalDevice, m_display, &displayModeSize, modeProperties.data());
	
	// modify the display surface
	VkDisplaySurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = NULL;
	surfaceCreateInfo.displayMode = modeProperties[0].displayMode;
	surfaceCreateInfo.planeIndex = 0;
	surfaceCreateInfo.planeStackIndex = 0;
	surfaceCreateInfo.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	surfaceCreateInfo.globalAlpha = 0.0f;
	surfaceCreateInfo.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
	surfaceCreateInfo.imageExtent = modeProperties[0].parameters.visibleRegion;
	res = vkCreateDisplayPlaneSurfaceKHR(m_instance, &surfaceCreateInfo, VK_ALLOC_CALLBACK, &m_surface);
#endif
	return res == VK_SUCCESS;
}

namespace {
	static inline void getColorBits(VkFormat format, uint32_t& redBits, uint32_t& greenBits, uint32_t& blueBits, uint32_t& alphaBits)
	{
		switch (format)
		{
		case VK_FORMAT_R8G8B8A8_SRGB:
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_SRGB: redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 8;  break;
		case VK_FORMAT_B8G8R8_SRGB:
		case VK_FORMAT_B8G8R8_UNORM:
		case VK_FORMAT_B8G8R8_SNORM:
		case VK_FORMAT_R8G8B8_SRGB:
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM: redBits = 8; greenBits = 8; blueBits = 8; alphaBits = 0;  break;
		case VK_FORMAT_R5G6B5_UNORM_PACK16: redBits = 5; greenBits = 6; blueBits = 5; alphaBits = 0;  break;
		default: assert(0);
		}
	}

	static inline void getDepthStencilBits(VkFormat format, uint32_t& depthBits, uint32_t& stencilBits)
	{
		switch (format)
		{
		case VK_FORMAT_D16_UNORM: depthBits = 16; stencilBits = 0; break;
		case VK_FORMAT_D16_UNORM_S8_UINT: depthBits = 16; stencilBits = 8; break;
		case VK_FORMAT_D24_UNORM_S8_UINT: depthBits = 24; stencilBits = 8; break;
		case VK_FORMAT_D32_SFLOAT: depthBits = 32; stencilBits = 0; break;
		case VK_FORMAT_D32_SFLOAT_S8_UINT: depthBits = 32; stencilBits = 8; break;
		case VK_FORMAT_X8_D24_UNORM_PACK32: depthBits = 24; stencilBits = 0; break;
		case VK_FORMAT_S8_UINT: depthBits = 0; stencilBits = 8; break;
		default: assert(0);
		}
	}

	static inline VkFormat getDepthStencilFormat(uint32_t depthBpp, uint32_t stencilBpp)
	{
		VkFormat dsFormat = VK_FORMAT_UNDEFINED;

		if (stencilBpp)
		{
			switch (depthBpp)
			{
			case 0: dsFormat = VK_FORMAT_S8_UINT; break;
			case 16: dsFormat = VK_FORMAT_D16_UNORM_S8_UINT; break;
			case 24: dsFormat = VK_FORMAT_D24_UNORM_S8_UINT; break;
			case 32: dsFormat = VK_FORMAT_D32_SFLOAT_S8_UINT; break;
			default: assert(0);
			}
		}
		else
		{
			switch (depthBpp)
			{
			case 16: dsFormat = VK_FORMAT_D16_UNORM; break;
			case 24: dsFormat = VK_FORMAT_X8_D24_UNORM_PACK32; break;
			case 32: dsFormat = VK_FORMAT_D32_SFLOAT; break;
			default: assert(0);
			}
		}

		return dsFormat;
	}

	const std::string depthStencilFormatToStr(VkFormat format)
	{
		const std::string preferredDsFormat[] =
		{
			"VK_FORMAT_D16_UNORM",
			"VK_FORMAT_X8_D24_UNORM_PACK32",
			"VK_FORMAT_D32_SFLOAT",
			"VK_FORMAT_S8_UINT",
			"VK_FORMAT_D16_UNORM_S8_UINT",
			"VK_FORMAT_D24_UNORM_S8_UINT",
			"VK_FORMAT_D32_SFLOAT_S8_UINT",
		};
		return preferredDsFormat[format - VK_FORMAT_D16_UNORM];
	}

	bool getMemoryTypeIndex(VkPhysicalDeviceMemoryProperties& deviceMemProps,
		uint32_t typeBits, VkMemoryPropertyFlagBits properties,
		uint32_t& outTypeIndex)
	{
		for (;;)
		{
			uint32_t typeBitsTmp = typeBits;
			for (uint32_t i = 0; i < 32; ++i)
			{
				if ((typeBitsTmp & 1) == 1)
				{
					if (VkMemoryPropertyFlagBits(deviceMemProps.memoryTypes[i].propertyFlags & properties) == properties)
					{
						outTypeIndex = i;
						return true;
					}
				}
				typeBitsTmp >>= 1;
			}
			if (properties & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
			{
				properties = VkMemoryPropertyFlagBits(properties & ~VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				continue;
			}
			else
			{
				break;
			}
		}
		return false;
	}


	VkDeviceMemory allocateMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& deviceMemProps,
		const VkMemoryRequirements& memoryRequirements,
		VkMemoryPropertyFlagBits allocMemProperty)
	{
		VkDeviceMemory memory;
		VkMemoryAllocateInfo sMemoryAllocInfo;
		sMemoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		sMemoryAllocInfo.pNext = NULL;
		sMemoryAllocInfo.allocationSize = memoryRequirements.size;
		assert(getMemoryTypeIndex(deviceMemProps, memoryRequirements.memoryTypeBits,
			allocMemProperty, sMemoryAllocInfo.memoryTypeIndex));
		vkAllocateMemory(device, &sMemoryAllocInfo, NULL, &memory);
		return memory;
	}

	bool allocateImageDeviceMemory(VkDevice device, VkPhysicalDeviceMemoryProperties& memprops,
		VkMemoryPropertyFlagBits allocMemProperty,
		VkImage& image, VkDeviceMemory& outMemory,
		VkMemoryRequirements* outMemRequirements)
	{
		VkMemoryRequirements memReq;
		VkMemoryRequirements* memReqPtr = &memReq;
		if (outMemRequirements)
		{
			memReqPtr = outMemRequirements;
		}
		vkGetImageMemoryRequirements(device, image, memReqPtr);
		if (memReqPtr->memoryTypeBits == 0) // find the first allowed type
			return false;

		outMemory = allocateMemory(device, memprops, *memReqPtr, allocMemProperty);
		if (outMemory == VK_NULL_HANDLE)
			return false;

		vkBindImageMemory(device, image, outMemory, 0);
		return true;
	}

	void setImageLayout(VkCommandBuffer& cmd, VkImageLayout oldLayout, VkImageLayout newLayout,
		VkImageAspectFlags aspectMask, VkAccessFlags srcAccessMask, VkImage image)
	{
		VkImageMemoryBarrier imageMemBarrier = {};
		imageMemBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageMemBarrier.pNext = NULL;
		imageMemBarrier.srcAccessMask = srcAccessMask;
		imageMemBarrier.dstAccessMask = 0;
		imageMemBarrier.oldLayout = oldLayout;
		imageMemBarrier.newLayout = newLayout;
		imageMemBarrier.image = image;
		imageMemBarrier.subresourceRange = { aspectMask, 0, 1, 0, 1 };

		if (newLayout == VK_IMAGE_LAYOUT_GENERAL)
		{
			imageMemBarrier.dstAccessMask =
				VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT
				| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_HOST_READ_BIT
				| VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_MEMORY_READ_BIT
				| VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			/* Make sure anything that was copying from this image has completed */
			imageMemBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			imageMemBarrier.dstAccessMask =
				VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			imageMemBarrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			/* Make sure any Copy or CPU writes to image are flushed */
			imageMemBarrier.dstAccessMask =
				VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL)
		{
			/* Make sure any Copy or CPU writes to image are flushed */
			imageMemBarrier.dstAccessMask =
				VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
		}
		else if (newLayout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			imageMemBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		}

		VkImageMemoryBarrier* memBarries = &imageMemBarrier;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, NULL, 0,
			NULL, 1, memBarries);
	}

	VkCommandBuffer allocPrimaryCmdBuffer(VkDevice device, VkCommandPool universalCmdPool)
	{
		VkResult res;
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandPool = universalCmdPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkCommandBuffer cmd;
		res = vkAllocateCommandBuffers(device, &allocInfo, &cmd);
		return cmd;
	}
}

bool VulkanGraphicContext::initSwapChain(bool hasDepth, bool hasStencil, uint32_t& swapChainLength)
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice,
		m_surface, &surfaceCapabilities);

#ifdef DEBUG
	Log(Log.Information, "Queried Surface Capabilities:");
	Log(Log.Information, "Min-max swap image count: %u - %u", surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);
	Log(Log.Information, "Array size: %u", surfaceCapabilities.maxImageArrayLayers);
	Log(Log.Information, "Image size (now): %dx%d", surfaceCapabilities.currentExtent.width,
		surfaceCapabilities.currentExtent.height);
	Log(Log.Information, "Image size (extent): %dx%d - %dx%d", surfaceCapabilities.minImageExtent.width,
		surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.width,
		surfaceCapabilities.maxImageExtent.height);
	Log(Log.Information, "Usage: %x", surfaceCapabilities.supportedUsageFlags);
	Log(Log.Information, "Current transform: %u", surfaceCapabilities.currentTransform);
#endif // DEBUG

#ifdef VK_USE_PLATFORM_ANDROID_KHR
	surfaceCapabilities.currentExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
		std::min(m_displayWidth, surfaceCapabilities.maxImageExtent.width));
	surfaceCapabilities.currentExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
		std::min(m_displayHeight, surfaceCapabilities.maxImageExtent.height));
#endif 

	m_displayWidth = surfaceCapabilities.currentExtent.width;
	m_displayHeight = surfaceCapabilities.currentExtent.height;

	// find optimal surface format
	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, NULL);

	VkSurfaceFormatKHR tmpformats[16]; 
	std::vector<VkSurfaceFormatKHR> tmpFormatsVector;
	VkSurfaceFormatKHR* allFormats = tmpformats;
	if (formatCount > 16)
	{
		tmpFormatsVector.resize(formatCount);
		allFormats = tmpFormatsVector.data();
	}

	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, allFormats);

	VkSurfaceFormatKHR format = allFormats[0];

	VkFormat preferredColorFormats[] =
	{
		VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SNORM,
		VK_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_R5G6B5_UNORM_PACK16
	};

	// find required rbga bits
	std::tuple<uint32_t, uint32_t, uint32_t, uint32_t> colorBits = m_pWindow->getColorBits();
	uint32_t requestedRedBpp = std::get<0>(colorBits);
	uint32_t requestedGreenBpp = std::get<1>(colorBits);
	uint32_t requestedBlueBpp = std::get<2>(colorBits);
	uint32_t requestedAlphaBpp = std::get<3>(colorBits);

	bool foundFormat = false;
	for (unsigned int i = 0; i < (sizeof(preferredColorFormats) / sizeof(preferredColorFormats[0])) && !foundFormat; ++i)
	{
		for (uint32_t f = 0; f < formatCount; ++f)
		{
			if (allFormats[f].format == preferredColorFormats[i])
			{
				uint32_t currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp = 0;
				getColorBits(allFormats[f].format, currentRedBpp, currentGreenBpp, currentBlueBpp, currentAlphaBpp);
				if (currentRedBpp == requestedRedBpp &&
					requestedGreenBpp == currentGreenBpp &&
					requestedBlueBpp == currentBlueBpp &&
					requestedAlphaBpp == currentAlphaBpp)
				{
					format = allFormats[f]; foundFormat = true; break;
				}
				else
				{
					format = allFormats[f]; foundFormat = true; break;
				}
			}
		}
	}

	bool useDepthStencil = hasDepth || hasStencil;
	std::pair<uint32_t, uint32_t> depthStencilBits = m_pWindow->getDepthStencilBits();
	VkFormat dsFormatRequested = getDepthStencilFormat(std::get<0>(depthStencilBits), std::get<1>(depthStencilBits));
	VkFormat supportedDsFormat = VK_FORMAT_UNDEFINED;
	if (useDepthStencil)
	{
		VkFormat preferredDsFormat[] =
		{
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D16_UNORM,
			VK_FORMAT_X8_D24_UNORM_PACK32
		};

		VkFormat currentDsFormat = dsFormatRequested;
		for (uint32_t f = 0; f < sizeof(preferredDsFormat) / sizeof(preferredDsFormat[0]); ++f)
		{
			VkFormatProperties prop;
			vkGetPhysicalDeviceFormatProperties(m_physicalDevice, currentDsFormat, &prop);
			if (prop.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
			{
				supportedDsFormat = currentDsFormat;
				break;
			}
			currentDsFormat = preferredDsFormat[f];
		}

		assert(dsFormatRequested != supportedDsFormat);

		getDepthStencilBits(supportedDsFormat, std::get<0>(depthStencilBits), std::get<1>(depthStencilBits));
	}

	// get the optimal present mode
	uint32_t numPresentMode;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numPresentMode, NULL);
	assert(numPresentMode > 0);
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numPresentMode, &presentModes[0]);

	// Default is FIFO - Which is typical Vsync.
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR desiredSwapMode = VK_PRESENT_MODE_FIFO_KHR;
	switch (m_pWindow->getSyncMode())
	{
	case VsyncMode::Off:
		desiredSwapMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		break;
	case VsyncMode::Mailbox:
		desiredSwapMode = VK_PRESENT_MODE_MAILBOX_KHR;
		break;
	case VsyncMode::Relaxed:
		desiredSwapMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		break;
	}

	for (size_t i = 0; i < numPresentMode; i++)
	{
		if (presentModes[i] == desiredSwapMode)
		{
			//Precise match - Break!
			swapchainPresentMode = desiredSwapMode;
			break;
		}
		//Secondary matches : Immediate and Mailbox are better fits for each other than Fifo, so set them as secondaries
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
		}
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox, set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_IMMEDIATE_KHR) && (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR))
		{
			swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}

	uint32_t swapLength = 0;
	switch (swapchainPresentMode)
	{
	case VK_PRESENT_MODE_IMMEDIATE_KHR: swapLength = 2; break;
	case VK_PRESENT_MODE_MAILBOX_KHR: swapLength = 3;  break;
	case VK_PRESENT_MODE_FIFO_KHR: swapLength = 2; break;
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR: swapLength = 2; break;
	}

	// store the color format and the display extent

	//--- create the swap chain
	VkSwapchainCreateInfoKHR swapchainCreate = {};
	swapchainCreate.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreate.clipped = VK_TRUE;
	swapchainCreate.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreate.surface = m_surface;

	swapLength = std::max<uint32_t>(swapLength, surfaceCapabilities.minImageCount);
	if (surfaceCapabilities.maxImageCount)
	{
		swapLength = std::min<uint32_t>(swapLength, surfaceCapabilities.maxImageCount);
	}

	swapLength = std::min<uint32_t>(swapLength, (uint32_t)MaxSwapChains);

	swapchainCreate.minImageCount = swapLength;
	//swapchainCreate.imageFormat = displayHandle.onscreenFbo.colorFormat;
	swapchainCreate.imageArrayLayers = 1;
	swapchainCreate.imageColorSpace = format.colorSpace;
	swapchainCreate.imageExtent = surfaceCapabilities.currentExtent;
	swapchainCreate.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_STORAGE_BIT)
	{
		swapchainCreate.imageUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
	}
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	{
		swapchainCreate.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	if (surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	{
		swapchainCreate.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	swapchainCreate.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainCreate.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreate.presentMode = swapchainPresentMode;
	swapchainCreate.queueFamilyIndexCount = 1;
	uint32_t queueFamily = 0;
	swapchainCreate.pQueueFamilyIndices = &queueFamily;

	//assertion(swapchainCreate.minImageCount <= (uint32)FrameworkCaps::MaxSwapChains,
	//	"Minimum number of swapchain images is larger than Max set");

	if (vkCreateSwapchainKHR(m_device, &swapchainCreate, nullptr, &m_swapChain) != VK_SUCCESS)
	{
		return false;
	}

	// initialize the swap chain image
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainLength, nullptr);

	m_pWindow->getScreenFbo()->colorBufferSize(swapChainLength);
	if (vkGetSwapchainImagesKHR(m_device, m_swapChain, &swapChainLength, m_pWindow->getScreenFbo()->getColorBuffer()) != VK_SUCCESS)
		return false;

	//--- create the swapchain view
	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.format = m_pWindow->getScreenFbo()->getColorFormat();
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.layerCount = 1;
	if (useDepthStencil)
		m_pWindow->getScreenFbo()->depthStencilSize(swapChainLength);

	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		viewCreateInfo.image = m_pWindow->getScreenFbo()->getColorBuffer()[i];
		if (vkCreateImageView(m_device, &viewCreateInfo, nullptr, &m_pWindow->getScreenFbo()->getColorBufferView()[i]) != VK_SUCCESS)
			return false;

		if (useDepthStencil)
		{
			// create the depth stencil image
			VkImageCreateInfo dsCreateInfo = {};
			dsCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			dsCreateInfo.format = supportedDsFormat;
			dsCreateInfo.extent.width = m_pWindow->getWidth();
			dsCreateInfo.extent.height = m_pWindow->getHeight();
			dsCreateInfo.extent.depth = 1;
			dsCreateInfo.imageType = VK_IMAGE_TYPE_2D;
			dsCreateInfo.arrayLayers = 1;
			dsCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			dsCreateInfo.mipLevels = 1;
			dsCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			dsCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			dsCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			dsCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			VkResult rslt = vkCreateImage(m_device, &dsCreateInfo, nullptr, &m_pWindow->getScreenFbo()->getDepthStencilBuffer()[i]);

			// allocate the image memory
			bool isOk = allocateImageDeviceMemory(m_device, m_deviceMemProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				m_pWindow->getScreenFbo()->getDepthStencilBuffer()[i],
				m_pWindow->getScreenFbo()->getDepthStencilMemory()[i], nullptr);

			// create the depth stencil view
			VkImageViewCreateInfo dsViewCreateInfo = {};
			dsViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			dsViewCreateInfo.image = m_pWindow->getScreenFbo()->getDepthStencilBuffer()[i];
			dsViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			dsViewCreateInfo.format = supportedDsFormat;
			dsViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
			dsViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
			dsViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
			dsViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
			dsViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
			dsViewCreateInfo.subresourceRange.levelCount = 1;
			dsViewCreateInfo.subresourceRange.layerCount = 1;

			m_pWindow->getScreenFbo()->setDepthFormat(supportedDsFormat);
			vkCreateImageView(m_device, &dsViewCreateInfo, nullptr, &m_pWindow->getScreenFbo()->getDepthStencilBufferView()[i]);
		}
	}

	return false;
}

bool VulkanGraphicContext::initSynchronizationObjects(uint32_t numSwapImages)
{
	// create the semaphores
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	VkResult res;
	for (uint32_t i = 0; i < numSwapImages; ++i)
	{
		res = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphoreFinishedRendering[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphoreCanBeginRendering[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphoreCanPresent[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphoreImageAcquired[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fencePrePresent[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fenceRender[i]);
		assert(res == VK_SUCCESS);

		res = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fenceAcquire[i]);
		assert(res == VK_SUCCESS);
	}

	res = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fencePrePresent[numSwapImages]);
	assert(res == VK_SUCCESS);

	res = vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_fenceAcquire[numSwapImages]);
	assert(res == VK_SUCCESS);

	res = vkCreateSemaphore(m_device, &semaphoreCreateInfo, NULL, &m_semaphoreImageAcquired[numSwapImages]);
	assert(res == VK_SUCCESS);

	return true;
}

bool VulkanGraphicContext::initPresentCommandBuffer(uint32_t swapChainLength)
{
	// create the commandpool and setup commandbuffer
	VkCommandBufferAllocateInfo cinfo = {};
	cinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cinfo.commandPool = m_universalCommandPool;
	cinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cinfo.commandBufferCount = swapChainLength;

	vkAllocateCommandBuffers(m_device, &cinfo, m_acquireBarrierCommandBuffersRenderQueue);
	vkAllocateCommandBuffers(m_device, &cinfo, m_presentBarrierCommandBuffersRenderQueue);

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = NULL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	VkCommandBufferBeginInfo beginnfo = {};
	beginnfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	for (uint32_t swapIndex = 0; swapIndex < swapChainLength; ++swapIndex)
	{
		barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		barrier.image = m_pWindow->getScreenFbo()->getColorBuffer()[swapIndex];
		vkBeginCommandBuffer(m_presentBarrierCommandBuffersRenderQueue[swapIndex], &beginnfo);
		vkCmdPipelineBarrier(m_presentBarrierCommandBuffersRenderQueue[swapIndex],
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0,
			NULL, 1, &barrier);
		vkEndCommandBuffer(m_presentBarrierCommandBuffersRenderQueue[swapIndex]);

		// post present
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		barrier.image = m_pWindow->getScreenFbo()->getColorBuffer()[swapIndex];
		vkBeginCommandBuffer(m_acquireBarrierCommandBuffersRenderQueue[swapIndex], &beginnfo);
		vkCmdPipelineBarrier(m_acquireBarrierCommandBuffersRenderQueue[swapIndex],
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, NULL, 0,
			NULL, 1, &barrier);
		vkEndCommandBuffer(m_acquireBarrierCommandBuffersRenderQueue[swapIndex]);
	}
	return true;
}

void VulkanGraphicContext::setInitialSwapchainLayouts(bool hasDepth, bool hasStencil, uint32_t swapChain, uint32_t swapChainLength)
{
	VkResult res;
	VkCommandBuffer cmdImgLayoutTrans = allocPrimaryCmdBuffer(m_device, m_universalCommandPool);
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	res = vkBeginCommandBuffer(cmdImgLayoutTrans, &cmdBeginInfo);

	bool useDepthStencil = hasDepth || hasStencil;
	for (uint32_t i = 0; i < swapChainLength; ++i)
	{
		// prepare the current swapchain image for writing
		if (i == swapChain)
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, m_pWindow->getScreenFbo()->getColorBuffer()[i]);
		}
		else// set all other swapchains to present so they will be transformed properly later.
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, m_pWindow->getScreenFbo()->getColorBuffer()[i]);
		}
		if (useDepthStencil)
		{
			setImageLayout(cmdImgLayoutTrans, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				VK_IMAGE_ASPECT_DEPTH_BIT | (hasStencil ? VK_IMAGE_ASPECT_STENCIL_BIT : 0),
				0, m_pWindow->getScreenFbo()->getDepthStencilBuffer()[i]);
		}
	}
	vkEndCommandBuffer(cmdImgLayoutTrans);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &cmdImgLayoutTrans;
	submitInfo.commandBufferCount = 1;
	submitInfo.pSignalSemaphores = &m_semaphoreCanBeginRendering[swapChain];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_semaphoreImageAcquired[m_currentImageAcqSem];
	submitInfo.waitSemaphoreCount = 1;

	VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
	submitInfo.pWaitDstStageMask = &pipeStageFlags;

	VkFence fence;
	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkCreateFence(m_device, &fenceInfo, NULL, &fence);
	vkQueueSubmit(m_universalQueue, 1, &submitInfo, fence);
	vkWaitForFences(m_device, 1, &fence, true, uint64_t(-1));
	vkDestroyFence(m_device, fence, NULL);
	vkFreeCommandBuffers(m_device, m_universalCommandPool, 1, &cmdImgLayoutTrans);
}

bool VulkanGraphicContext::init()
{
	// initialise vulkan instance
	if (!initVkInstance())
		return false;

#ifdef DEBUG
	// initialise the debug call backs
	if (!initDebugCallbacks(*_platformContextHandles))
		return false;
#endif

	// initialise physical device
	if (!initPhysicalDevice())
		return false;
	
	// initialise the surfaces used for rendering
	if (!initSurface())
		return false;
	
	if (!initLogicalDevice())
		return false;

	//Todo: how to create command pool when using multithread rendering?
	{
		VkCommandPoolCreateInfo pInfo = {};
		pInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		pInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		pInfo.queueFamilyIndex = m_universalQueueFamily;
		vkCreateCommandPool(m_device, &pInfo, NULL, &m_universalCommandPool);
	}

	// initialize the swap chain images
	uint32_t swapChainLength = 0;
	if (!initSwapChain(true, true, swapChainLength))
		return false;

	// initialize synchronization object
	if (!initSynchronizationObjects(swapChainLength))
		return false;

	// initialize present command buffer
	if (!initPresentCommandBuffer(swapChainLength))
		return false;

	// acquire the first image
	if (vkAcquireNextImageKHR(m_device, m_swapChain, -1, m_semaphoreImageAcquired[m_currentImageAcqSem], VK_NULL_HANDLE, &m_swapIndex) != VK_SUCCESS)
		return false;

	// initialize the swapchain layout
	setInitialSwapchainLayouts(true, true, m_swapIndex, swapChainLength);
	vkResetFences(m_device, 1, &m_fenceRender[m_swapIndex]);

	return false;
}

void VulkanGraphicContext::release()
{
}

void VulkanGraphicContext::waitIdle()
{
}