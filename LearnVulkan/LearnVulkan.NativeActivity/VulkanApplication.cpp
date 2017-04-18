#include "pch.h"
#include "common\vulkan_wrapper.h"
#include "VulkanInstance.h"
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
	if (m_instance == nullptr)
		m_instance = new VulkanInstance;

	VkResult res = m_instance->createInstance(layers, extensions, "VulkanEngine");
	return res == VK_SUCCESS;
}
