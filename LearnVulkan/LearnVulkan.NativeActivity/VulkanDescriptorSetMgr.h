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

	VkResult createDescriptorSetLayout(const VkDevice& graphicDevice, const VulkanRenderable* pRenderEntity, std::vector<VkDescriptorSetLayout>& shaderParams);

	VkResult createDescriptorSet(const VkDevice& graphicDevice, VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSetLayout>& shaderParams, VkDescriptorSet& shaderDescriptorSet);

	VkResult updateDescriptorSetbyRenderableEntity(const VkDevice& graphicDevice, const VkDescriptorSet& shaderDescriptorSet, const VulkanRenderable* pRenderable);

private:

	static VulkanDescriptorSetMgr* m_pMgr;
};