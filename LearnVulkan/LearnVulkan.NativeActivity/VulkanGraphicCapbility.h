#pragma once

class VulkanGraphicContext;

class VulkanGraphicCapbility
{
public:
	VulkanGraphicCapbility(VulkanGraphicContext* pContext);
	~VulkanGraphicCapbility();

	bool isSupportGeometryShader() const;
	bool isSupportTessellationShader() const;
	bool isSupportSparseBinding() const;

	bool isSupportVertexPipelineStoresAndAtomics() const;
	bool isSupportFragmentStoresAndAtomics() const;
	
	bool isSupportMutipleDrawIndirect() const;
	bool isSupportDrawIndirectInstance() const;

	bool isSupportMultipleViewport() const;
	uint32_t maximalViewports() const;

	bool isSupportShaderDouble() const;
	bool isSupportShaderLong() const;

	bool isIntegratedGPU() const;
	bool isDiscreteGPU() const;

	uint32_t maximalColorBuffers() const;
	
	uint32_t maximalPushConstants() const;
	uint32_t maximalDescriptorSets() const;

	float minimalLineWidth() const;
	float maximalLineWidth() const;

	float minimalPointSize() const;
	float maximalPointSize() const;

private:
	VulkanGraphicContext* m_pContext;

	VkPhysicalDeviceFeatures m_deviceFeatures;
	VkPhysicalDeviceProperties m_deviceProperties;
};

