#include "pch.h"
#include "VulkanFrameBuffer.h"


VulkanFrameBuffer::VulkanFrameBuffer(VulkanGraphicContext* pContext)
{
}


VulkanFrameBuffer::~VulkanFrameBuffer()
{
}

void VulkanFrameBuffer::colorBufferSize(uint32_t colorSize)
{
	m_colorBuffer.resize(colorSize);
	m_colorBufferView.resize(colorSize);
}

void VulkanFrameBuffer::depthStencilSize(uint32_t depthSize)
{
	m_depthStencilBuffer.resize(depthSize);
	m_depthStencilMemory.resize(depthSize);
	m_depthStencilBufferView.resize(depthSize);
}

VkImage * VulkanFrameBuffer::getColorBuffer()
{
	return m_colorBuffer.empty() ? nullptr : m_colorBuffer.data();
}

VkImageView * VulkanFrameBuffer::getColorBufferView()
{
	return m_colorBufferView.empty() ? nullptr : m_colorBufferView.data();
}

VkImage * VulkanFrameBuffer::getDepthStencilBuffer()
{
	return m_depthStencilBuffer.empty() ? nullptr : m_depthStencilBuffer.data();
}

VkDeviceMemory * VulkanFrameBuffer::getDepthStencilMemory()
{
	return m_depthStencilBuffer.empty() ? nullptr : m_depthStencilMemory.data();
}

VkImageView * VulkanFrameBuffer::getDepthStencilBufferView()
{
	return m_depthStencilBufferView.empty() ? nullptr : m_depthStencilBufferView.data();
}

const VkFormat VulkanFrameBuffer::getColorFormat() const
{
	return m_colorFormat;
}

void VulkanFrameBuffer::setColorFormat(VkFormat format)
{
	m_colorFormat = format;
}

const VkFormat VulkanFrameBuffer::getDepthFormat() const
{
	return m_depthFormat;
}

void VulkanFrameBuffer::setDepthFormat(VkFormat depthFormat)
{
	m_depthFormat = depthFormat;
}
