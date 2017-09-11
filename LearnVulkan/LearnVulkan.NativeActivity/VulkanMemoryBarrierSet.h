#pragma once
#include <vector>
#include "common\vulkan_wrapper.h"

class VulkanMemoryBarrierSet
{
public:
	VulkanMemoryBarrierSet();
	~VulkanMemoryBarrierSet();

	VulkanMemoryBarrierSet(const VulkanMemoryBarrierSet&) = delete;
	VulkanMemoryBarrierSet& operator=(const VulkanMemoryBarrierSet&) = delete;

	void clearBarriers();

	void clearMemoryBarriers();

	void clearBufferBarriees();

	void clearImageBarriers();

	VulkanMemoryBarrierSet& emplaceBarrier(VkMemoryBarrier memoryBarrier);

	VulkanMemoryBarrierSet& emplaceBarrier(VkBufferMemoryBarrier bufferBarrier);

	VulkanMemoryBarrierSet& emplaceBarrier(VkImageMemoryBarrier imageBarrier);

	const std::vector<VkMemoryBarrier>& getMemoryBarriers() const;

	const std::vector<VkBufferMemoryBarrier>& getBufferBarriers() const;

	const std::vector<VkImageMemoryBarrier>& getImageBarriers() const;

private:
	std::vector<VkMemoryBarrier> mMemoryBarriers;
	std::vector<VkBufferMemoryBarrier> mBufferBarriers;
	std::vector<VkImageMemoryBarrier> mImageBarriers;

};

