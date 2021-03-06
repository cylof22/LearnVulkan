#pragma once
#include <memory>
#include <vector>
#include "common\vulkan_wrapper.h"

class VulkanHardwareVertexBuffer;
class VulkanHardwareIndexBuffer;
class VulkanHardwareUniformBuffer;
class VulkanHardwareTextureBuffer;
class VulkanGpuProgram;

class VulkanRenderable
{
public:
	VulkanRenderable();
	~VulkanRenderable();

	void setVertexBuffer(std::shared_ptr<VulkanHardwareVertexBuffer> vertexBuffer) { m_rVertexBuf = vertexBuffer; }
	void setIndexBuffer(std::shared_ptr<VulkanHardwareIndexBuffer> indexBuffer) { m_rIndexBuf = indexBuffer; }
	void setUniformBuffer(std::shared_ptr<VulkanHardwareUniformBuffer> uniformBuffer) { m_rUniformBuffer = uniformBuffer; }
	void setTextureBuffer(std::shared_ptr<VulkanHardwareTextureBuffer> textureBuffer) { m_rTextureBuffer = textureBuffer; }

	void setVertexShader(std::shared_ptr<VulkanGpuProgram> vertexShader) { m_rVertexShader = vertexShader; }
	void setFragmentShader(std::shared_ptr<VulkanGpuProgram> fragShader) { m_rFragmentShader = fragShader; }
	void setTopologyType(const VkPrimitiveTopology type) { m_topologyType = type; }
	
	std::shared_ptr<VulkanGpuProgram> getVertexShader() const { return m_rVertexShader; }
	std::shared_ptr<VulkanGpuProgram> getFragmentShader() const { return m_rFragmentShader; }

	void setDescriptorSet(const VkDescriptorSet& descriptorSet) { m_descriptorSet = descriptorSet; }
	void setDescriptorSetLayout(const std::vector<VkDescriptorSetLayout> descriptorSetLayouts) { m_descriptorSetLayouts = descriptorSetLayouts; }
	const VkDescriptorSet* getDescriptorSet() const { return &m_descriptorSet; }
	const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayout() const { return m_descriptorSetLayouts; }

	void addPushConstant(VkShaderStageFlags stage, uint32_t offset, uint32_t size);
	const std::vector<VkPushConstantRange>& getPushConstant() const;

	const std::shared_ptr<VulkanHardwareVertexBuffer> getVertexBuffer() const { return m_rVertexBuf; }
	const std::shared_ptr<VulkanHardwareIndexBuffer> getIndexBuffer() const { return m_rIndexBuf; }
	const std::shared_ptr<VulkanHardwareUniformBuffer> getUniformBuffer() const { return m_rUniformBuffer; }
	const std::shared_ptr<VulkanHardwareTextureBuffer> getTextureBuffer() const { return m_rTextureBuffer; }

	VkPrimitiveTopology getTopologyType() const { return m_topologyType; };
	bool isIndexRestart() const { return m_bIsIndexRestart; }

	bool getShaderStageInfo(std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfo) const;
private:
	std::shared_ptr<VulkanHardwareVertexBuffer> m_rVertexBuf;
	std::shared_ptr<VulkanHardwareIndexBuffer> m_rIndexBuf;

	VkPrimitiveTopology m_topologyType;
	bool m_bIsIndexRestart;

	std::shared_ptr<VulkanGpuProgram> m_rVertexShader;
	std::shared_ptr<VulkanGpuProgram> m_rTesslleationControlShader;
	std::shared_ptr<VulkanGpuProgram> m_rTesslleationEvalutionShader;
	std::shared_ptr<VulkanGpuProgram> m_rGeometryShader;
	std::shared_ptr<VulkanGpuProgram> m_rFragmentShader;

	VkDescriptorSet m_descriptorSet;
	std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;

	std::vector<VkPushConstantRange> m_pushConstantSet;

	std::shared_ptr<VulkanHardwareUniformBuffer> m_rUniformBuffer;
	std::shared_ptr<VulkanHardwareTextureBuffer> m_rTextureBuffer;
};