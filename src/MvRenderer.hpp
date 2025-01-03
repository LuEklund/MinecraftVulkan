#pragma once

#include "MvWindow.h"
#include "MvDevice.hpp"
#include "MvSwapChain.hpp"
#include "MvModel.hpp"

#include <memory>
#include <cassert>


class MvRenderer
{
public:
	MvRenderer(MvWindow &window, MvDevice &device);
	~MvRenderer();

	MvRenderer(const MvRenderer&) = delete;
	MvRenderer& operator=(const MvRenderer&) = delete;

    VkRenderPass GetSwapChainRenderPass() const { return m_swapChain->GetRenderPass(); }

    bool            IsFrameInProgress() const { return m_FrameStarted; }
    VkCommandBuffer GetCurrentCommandBuffer() const {
        assert(m_FrameStarted && "Cannot get command buffer when frame not in progress.");
        return m_CommandBuffers[m_currentFrameIndex];
    }
    
    VkCommandBuffer BeginFrame();
    void            EndFrame();
    void            BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void            EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
    int GetFrameIndex() const {
        assert(m_FrameStarted && "Cannot get frame index when frame not in progress.");
        return m_currentFrameIndex; }

private:

	//1. Device
	MvDevice &m_Device;

	//2. Pipeline
	std::vector<VkCommandBuffer> m_CommandBuffers;
	void	CreateCommandBuffers();
	void	FreeCommandBuffers();

	//3. SwapChain
	std::unique_ptr<MvSwapChain> m_swapChain;
	void RecreateSwapChain();

	//4. Window
	MvWindow &m_window;

    uint32_t m_currentImageIndex;
    int m_currentFrameIndex = 0;
    bool m_FrameStarted = false;
};

