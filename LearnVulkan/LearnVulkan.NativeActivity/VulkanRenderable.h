#pragma once
#include <memory>
#include "common\vulkan_wrapper.h"

class VulkanHardwareVertexBuffer;
class VulkanHardwareIndexBuffer;
class VulkanGpuProgram;

class VulkanRenderable
{
public:
	VulkanRenderable();
	~VulkanRenderable();

	const std::shared_ptr<VulkanHardwareVertexBuffer> getVertexBuffer() const { return m_rVertexBuf; }
	const std::shared_ptr<VulkanHardwareIndexBuffer> getIndexBuffer() const { return m_rIndexBuf; }

	VkPrimitiveTopology getTopologyType() const { return m_topologyType; };
	bool isIndexRestart() const { return m_bIsIndexRestart; }

private:
	std::shared_ptr<VulkanHardwareVertexBuffer> m_rVertexBuf;
	std::shared_ptr<VulkanHardwareIndexBuffer> m_rIndexBuf;

	VkPrimitiveTopology m_topologyType;
	bool m_bIsIndexRestart;

	std::shared_ptr<VulkanGpuProgram> m_rVertexShader;
	std::shared_ptr<VulkanGpuProgram> m_rTesslleationControlShader;
	std::shared_ptr<VulkanGpuProgram> m_rTesslleationEvalutionShader;
	std::shared_ptr<VulkanGpuProgram> m_rGeometryShader;
	std::shared_ptr<VulkanGpuProgram> _rFragmentShader;
};