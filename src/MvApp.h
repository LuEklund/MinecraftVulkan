#pragma once

#include "MvWindow.h"
#include "MvPipeline.h"
#include "MvDevice.hpp"
#include "MvSwapChain.hpp"

#include <memory>

class MvApp
{
public:
	MvApp();
	~MvApp();

	MvApp(const MvApp&) = delete;
	MvApp& operator=(const MvApp&) = delete;

	void Run();
private:
	
	//Window
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;
	MvWindow* m_window;
	void DrawFrame();

	//PipeLine
	std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	void CreatepipelineLayout();
	void CreatePipeline();
	void CreateCommandBuffers();

	//SwapChain
	MvSwapChain* m_swapChain;

	//EngineDevice
	MvDevice* m_Device;
};

