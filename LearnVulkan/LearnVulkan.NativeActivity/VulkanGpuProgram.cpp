#include "VulkanGpuProgram.h"
#include <iostream>
#include <string>

VulkanGpuProgram::VulkanGpuProgram(const VkDevice& graphicDevice, const std::string& fileName, shaderc_shader_kind kind, const std::string& data)
{
	// convert the file to Vulkan spv format
	shaderc::Compiler compiler;
	shaderc::CompileOptions options;

	// Like -DMY_DEFINE=1
	options.AddMacroDefinition("MY_DEFINE", "1");

	shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
		data.c_str(), data.size(), kind, fileName.c_str(), options);

	if (module.GetCompilationStatus() !=
		shaderc_compilation_status_success) {
		std::cerr << module.GetErrorMessage();
	}

	std::vector<uint32_t> result(module.cbegin(), module.cend());

	VkResult res;
	// create the shaderModule
	VkShaderModuleCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	info.pNext = nullptr;
	info.flags = 0;
	info.codeSize = result.size() * sizeof(uint32_t);
	info.pCode = result.data();

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	res = vkCreateShaderModule(graphicDevice, &info, nullptr, &shaderModule);

	m_shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	m_shaderStage.pNext = nullptr;
	m_shaderStage.flags = 0;
	m_shaderStage.pSpecializationInfo = nullptr;
	m_shaderStage.pName = "main";
	m_shaderStage.stage = convertShaderType2ShaderStageFlag(kind);
	m_shaderStage.module = shaderModule;
} 

VulkanGpuProgram::~VulkanGpuProgram()
{
}

const VkPipelineShaderStageCreateInfo & VulkanGpuProgram::getShaderStage()
{
	return m_shaderStage;
}

VkShaderStageFlagBits VulkanGpuProgram::convertShaderType2ShaderStageFlag(const shaderc_shader_kind & kind)
{
	VkShaderStageFlagBits shaderFlag;
	switch (kind)
	{
	case shaderc_glsl_vertex_shader:
	case shaderc_glsl_default_vertex_shader:
		shaderFlag = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case shaderc_glsl_tess_control_shader:
	case shaderc_glsl_default_tess_control_shader:
		shaderFlag = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case shaderc_glsl_tess_evaluation_shader:
	case shaderc_glsl_default_tess_evaluation_shader:
		shaderFlag = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case shaderc_glsl_geometry_shader:
	case shaderc_glsl_default_geometry_shader:
		shaderFlag = VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case shaderc_glsl_fragment_shader:
	case shaderc_glsl_default_fragment_shader:
		shaderFlag = VK_SHADER_STAGE_FRAGMENT_BIT;
	case shaderc_glsl_compute_shader:
	case shaderc_glsl_default_compute_shader:
		shaderFlag = VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	default: // unknow shader type
		shaderFlag = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
	return shaderFlag;
}
