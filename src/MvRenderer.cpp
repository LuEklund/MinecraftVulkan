#include "MvRenderer.hpp"


#include <stdexcept>
#include <array>


MvRenderer::MvRenderer(MvWindow &window, MvDevice &device)
    :   m_Device(device)
    ,   m_window(window)
{
	RecreateSwapChain();
	CreateCommandBuffers();
}

MvRenderer::~MvRenderer()
{
    FreeCommandBuffers();
}



void MvRenderer::RecreateSwapChain()
{
	VkExtent2D extent = m_window.GetExtent();
	while (extent.width == 0 || extent.height == 0)
	{
		extent = m_window.GetExtent();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(m_Device.GetDevice());

	if (m_swapChain == nullptr)
	{
		m_swapChain = std::make_unique<MvSwapChain>(m_Device, extent);
	}
	else
	{
		std::shared_ptr<MvSwapChain> oldSwapChain = std::move(m_swapChain);
		m_swapChain = std::make_unique<MvSwapChain>(m_Device, extent, oldSwapChain);

		if (!oldSwapChain->compareSwapFormats(*m_swapChain))
		{
			throw std::runtime_error("Swap chain image format has changed!");
		}
		
	}
    //TODO: get back
}

void MvRenderer::CreateCommandBuffers()
{
	m_CommandBuffers.resize(MvSwapChain::MAX_FRAMES_IN_FLIGHT);

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_Device.GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	
}

void MvRenderer::FreeCommandBuffers()
{
	vkFreeCommandBuffers(m_Device.GetDevice(), m_Device.GetCommandPool(), static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
	m_CommandBuffers.clear();
}


VkCommandBuffer MvRenderer::BeginFrame()
{
    assert(!m_FrameStarted && "Can't call BeginFrame while frame is in progress.");
	
    VkResult res = m_swapChain->acquireNextImage(&m_currentImageIndex);

	if (res == VK_ERROR_OUT_OF_DATE_KHR)
	{
		RecreateSwapChain();
		return nullptr;
	}

	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

    m_FrameStarted = true;

    auto commandBuffer = GetCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to begin recording command buffer!");
	}
    
    return commandBuffer;
}

void MvRenderer::EndFrame()
{
    assert(m_FrameStarted && "Can't call EndFrame while frame is not in progress.");
    auto commandBuffer = GetCurrentCommandBuffer();
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to record command buffer!");
	}
    
	VkResult res = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
	if (res == VK_ERROR_OUT_OF_DATE_KHR || res == VK_SUBOPTIMAL_KHR || m_window.WasWindowResized())
	{
		m_window.ResetWindowResizedFlag();
		RecreateSwapChain();
	}
	else if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}
    m_FrameStarted = false;
	m_currentFrameIndex = (m_currentFrameIndex + 1) % MvSwapChain::MAX_FRAMES_IN_FLIGHT;
    
}

void MvRenderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_FrameStarted && "Can't begin render pass when frame is not in progress.");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame.");

    VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_swapChain->GetRenderPass();
	renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();
	// renderPassInfo.renderArea.exent = {static_cast<int32_t>(m_swapChain->GetWidth()), static_cast<int32_t>(m_swapChain->GetHeigth())};

	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = static_cast<float>(m_swapChain->GetSwapChainExtent().height);
	viewport.width = static_cast<float>(m_swapChain->GetSwapChainExtent().width);
	viewport.height = -static_cast<float>(m_swapChain->GetSwapChainExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{{0, 0}, m_swapChain->GetSwapChainExtent()};
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}

void MvRenderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_FrameStarted && "Can't end render pass when frame is not in progress.");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame.");
    vkCmdEndRenderPass(commandBuffer);
}
