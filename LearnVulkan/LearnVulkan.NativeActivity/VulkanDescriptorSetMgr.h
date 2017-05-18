#pragma once
#include <vector>
#include "common\vulkan_wrapper.h"
class VulkanRenderable;

class VulkanDescriptorSetMgr
{
public:
	VulkanDescriptorSetMgr();
	~VulkanDescriptorSetMgr();

	static VulkanDescriptorSetMgr* get();

	VkResult createDescriptorSetLayout(VkDevice& graphicDevice, const VulkanRenderable* pRenderEntity, std::vector<VkDescriptorSetLayout>& shaderParams);

	VkResult createDescriptorSet(VkDevice& graphicDevice, VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSetLayout>& shaderParams, VkDescriptorSet& shaderDescriptorSet);

	VkResult updateDescriptorSetbyUniformBuffer(VkDevice& graphicDevice, VkDescriptorSet& shaderDescriptorSet, const VkDescriptorBufferInfo& bufferInfo);

private:

	static VulkanDescriptorSetMgr* m_pMgr;
};