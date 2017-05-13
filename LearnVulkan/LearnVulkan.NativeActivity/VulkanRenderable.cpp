#include "VulkanRenderable.h"
#include "VulkanGpuProgram.h"

VulkanRenderable::VulkanRenderable() : m_bIsIndexRestart(false)
{
}

VulkanRenderable::~VulkanRenderable()
{
}

bool VulkanRenderable::getShaderStageInfo(std::vector<VkPipelineShaderStageCreateInfo>& shaderStageInfo) const
{
	shaderStageInfo.clear();

	if (m_rVertexShader)
		shaderStageInfo.emplace_back(m_rVertexShader->getShaderStage());

	if (m_rTesslleationControlShader)
		shaderStageInfo.emplace_back(m_rTesslleationControlShader->getShaderStage());

	if (m_rTesslleationEvalutionShader)
		shaderStageInfo.emplace_back(m_rTesslleationEvalutionShader->getShaderStage());

	if (m_rGeometryShader)
		shaderStageInfo.emplace_back(m_rGeometryShader->getShaderStage());

	if (m_rFragmentShader)
		shaderStageInfo.emplace_back(m_rFragmentShader->getShaderStage());

	return !shaderStageInfo.empty();
}
