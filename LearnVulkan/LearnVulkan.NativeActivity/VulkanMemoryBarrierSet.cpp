#include "pch.h"
#include "VulkanMemoryBarrierSet.h"


VulkanMemoryBarrierSet::VulkanMemoryBarrierSet()
{
}


VulkanMemoryBarrierSet::~VulkanMemoryBarrierSet()
{
}

void VulkanMemoryBarrierSet::clearBarriers()
{
	mImageBarriers.clear();
	mBufferBarriers.clear();
	mMemoryBarriers.clear();
}

void VulkanMemoryBarrierSet::clearMemoryBarriers()
{
	mMemoryBarriers.clear();
}

void VulkanMemoryBarrierSet::clearBufferBarriees()
{
	mBufferBarriers.clear();
}

void VulkanMemoryBarrierSet::clearImageBarriers()
{
	mImageBarriers.clear();
}

VulkanMemoryBarrierSet & VulkanMemoryBarrierSet::emplaceBarrier(VkMemoryBarrier memoryBarrier)
{
	mMemoryBarriers.emplace_back(memoryBarrier);
	return *this;
}

VulkanMemoryBarrierSet & VulkanMemoryBarrierSet::emplaceBarrier(VkBufferMemoryBarrier bufferBarrier)
{
	mBufferBarriers.emplace_back(bufferBarrier);
	return *this;
}

VulkanMemoryBarrierSet & VulkanMemoryBarrierSet::emplaceBarrier(VkImageMemoryBarrier imageBarrier)
{
	mImageBarriers.emplace_back(imageBarrier);
	return *this;
}

const std::vector<VkMemoryBarrier>& VulkanMemoryBarrierSet::getMemoryBarriers() const
{
	return mMemoryBarriers;
}

const std::vector<VkBufferMemoryBarrier>& VulkanMemoryBarrierSet::getBufferBarriers() const
{
	return mBufferBarriers;
}

const std::vector<VkImageMemoryBarrier>& VulkanMemoryBarrierSet::getImageBarriers() const
{
	return mImageBarriers;
}
