#include "VulkanDescriptorSetMgr.h"
#include "VulkanRenderable.h"

VulkanDescriptorSetMgr* VulkanDescriptorSetMgr::m_pMgr = nullptr;
VulkanDescriptorSetMgr * VulkanDescriptorSetMgr::get()
{
	if (m_pMgr == nullptr)
		m_pMgr = new VulkanDescriptorSetMgr();

	return m_pMgr;
}

VulkanDescriptorSetMgr::VulkanDescriptorSetMgr()
{
}

VulkanDescriptorSetMgr::~VulkanDescriptorSetMgr()
{
}

VkResult VulkanDescriptorSetMgr::createDescriptorSetLayout(const VkDevice& graphicDevice, const VulkanRenderable* pRenderEntity, std::vector<VkDescriptorSetLayout>& shaderParams)
{
	if (pRenderEntity == nullptr)
		return VK_RESULT_MAX_ENUM;

	VkResult res = VK_RESULT_MAX_ENUM;
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

	// Use the renderable's program information to create the descriptor set layout
	std::shared_ptr<VulkanGpuProgram> rVertexShader = pRenderEntity->getVertexShader();
	if (rVertexShader)
	{
		VkDescriptorSetLayoutBinding vertexUniformBinding;
		vertexUniformBinding.binding = 0;
		vertexUniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		vertexUniformBinding.descriptorCount = 1;
		vertexUniformBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		vertexUniformBinding.pImmutableSamplers = nullptr;

		layoutBindings.emplace_back(vertexUniformBinding);
	}

	/*std::shared_ptr<VulkanGpuProgram> rFragmentShader = pRenderEntity->getFragmentShader();
	if (rFragmentShader)
	{
		VkDescriptorSetLayoutBinding textureBinding;
		textureBinding.binding = 1;
		textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		textureBinding.descriptorCount = 1;
		textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		textureBinding.pImmutableSamplers = nullptr;

		layoutBindings.emplace_back(textureBinding);
	}*/

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.pNext = nullptr;
	layoutInfo.bindingCount = layoutBindings.size();
	layoutInfo.pBindings = layoutBindings.data();

	shaderParams.clear();
	shaderParams.resize(layoutBindings.size());
	res = vkCreateDescriptorSetLayout(graphicDevice, &layoutInfo, nullptr, shaderParams.data());

	return res;
}

VkResult VulkanDescriptorSetMgr::createDescriptorSet(const VkDevice& graphicDevice, VkDescriptorPool& descriptorPool,  std::vector<VkDescriptorSetLayout>& shaderParams, VkDescriptorSet& shaderDescriptorSet)
{
	VkResult res = VK_RESULT_MAX_ENUM;
	
	VkDescriptorSetAllocateInfo allocateInfo = {};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = descriptorPool;
	allocateInfo.descriptorSetCount = 1;
	allocateInfo.pSetLayouts = shaderParams.data();

	res = vkAllocateDescriptorSets(graphicDevice, &allocateInfo, &shaderDescriptorSet);
	return res;
}

VkResult VulkanDescriptorSetMgr::updateDescriptorSetbyUniformBuffer(const VkDevice& graphicDevice, VkDescriptorSet& shaderDescriptorSet, const VkDescriptorBufferInfo& bufferInfo)
{
	VkResult res = VK_RESULT_MAX_ENUM;
	
	VkWriteDescriptorSet bufferUpdateInfo = {};
	bufferUpdateInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	bufferUpdateInfo.pNext = nullptr;
	bufferUpdateInfo.dstSet = shaderDescriptorSet;
	bufferUpdateInfo.descriptorCount = 1;
	bufferUpdateInfo.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bufferUpdateInfo.pBufferInfo = &bufferInfo;
	bufferUpdateInfo.dstArrayElement = 0;
	bufferUpdateInfo.dstBinding = 0;

	vkUpdateDescriptorSets(graphicDevice, 1, &bufferUpdateInfo, 0, nullptr);
	return res;
}
