#pragma once
#include "common\vulkan_wrapper.h"
#include <string>
#include <shaderc\shaderc.hpp>

class VulkanGpuProgram
{
public:
	VulkanGpuProgram(const VkDevice& graphicDevice, const std::string& fileName, shaderc_shader_kind kind, const std::string& data);
	~VulkanGpuProgram();

	const VkPipelineShaderStageCreateInfo& getShaderStage();
protected:
	VkShaderStageFlagBits convertShaderType2ShaderStageFlag(const shaderc_shader_kind& kind);
private:
	VkPipelineShaderStageCreateInfo m_shaderStage;
};