#include "pch.h"
#include "common\vulkan_wrapper.h"
#include "VulkanGraphicContext.h"
#include "VulkanGraphicCapbility.h"


VulkanGraphicCapbility::VulkanGraphicCapbility(VulkanGraphicContext* pContext)
	: m_pContext(pContext)
{
	vkGetPhysicalDeviceFeatures(m_pContext->getGPU(), &m_deviceFeatures);
	vkGetPhysicalDeviceProperties(m_pContext->getGPU(), &m_deviceProperties);
}


VulkanGraphicCapbility::~VulkanGraphicCapbility()
{
}

bool VulkanGraphicCapbility::isSupportGeometryShader() const
{
	return m_deviceFeatures.geometryShader;
}

bool VulkanGraphicCapbility::isSupportTessellationShader() const
{
	return m_deviceFeatures.tessellationShader;
}

bool VulkanGraphicCapbility::isSupportSparseBinding() const
{
	return m_deviceFeatures.sparseBinding;
}

bool VulkanGraphicCapbility::isSupportVertexPipelineStoresAndAtomics() const
{
	return m_deviceFeatures.vertexPipelineStoresAndAtomics;
}

bool VulkanGraphicCapbility::isSupportFragmentStoresAndAtomics() const
{
	return m_deviceFeatures.fragmentStoresAndAtomics;
}

bool VulkanGraphicCapbility::isSupportMutipleDrawIndirect() const
{
	return m_deviceFeatures.multiDrawIndirect;
}

bool VulkanGraphicCapbility::isSupportDrawIndirectInstance() const
{
	return m_deviceFeatures.drawIndirectFirstInstance;
}

bool VulkanGraphicCapbility::isSupportMultipleViewport() const
{
	return m_deviceFeatures.multiViewport;
}

uint32_t VulkanGraphicCapbility::maximalViewports() const
{
	return m_deviceProperties.limits.maxViewports;
}

bool VulkanGraphicCapbility::isSupportShaderDouble() const
{
	return m_deviceFeatures.shaderFloat64;
}

bool VulkanGraphicCapbility::isSupportShaderLong() const
{
	return m_deviceFeatures.shaderInt64;
}

bool VulkanGraphicCapbility::isIntegratedGPU() const
{
	return m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
}

bool VulkanGraphicCapbility::isDiscreteGPU() const
{
	return m_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
}

uint32_t VulkanGraphicCapbility::maximalColorBuffers() const
{
	return m_deviceProperties.limits.maxColorAttachments;
}

uint32_t VulkanGraphicCapbility::maximalPushConstants() const
{
	return m_deviceProperties.limits.maxPushConstantsSize;
}

uint32_t VulkanGraphicCapbility::maximalDescriptorSets() const
{
	return m_deviceProperties.limits.maxBoundDescriptorSets;
}

float VulkanGraphicCapbility::minimalLineWidth() const
{
	return m_deviceProperties.limits.lineWidthRange[0];
}

float VulkanGraphicCapbility::maximalLineWidth() const
{
	return m_deviceProperties.limits.lineWidthRange[1];
}

float VulkanGraphicCapbility::minimalPointSize() const
{
	return m_deviceProperties.limits.pointSizeRange[0];
}

float VulkanGraphicCapbility::maximalPointSize() const
{
	return m_deviceProperties.limits.pointSizeRange[1];
}
