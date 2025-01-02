#pragma once

#include "MvWindow.h"
#include "MvPipeline.h"
#include "MvDevice.hpp"
#include "MvSwapChain.hpp"
#include "MvModel.hpp"

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

	//1. Device
	std::unique_ptr<MvDevice> m_Device;

	//2. Pipeline
	std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	std::vector<VkCommandBuffer> m_CommandBuffers;
	void CreatepipelineLayout();
	void CreatePipeline();
	void CreateCommandBuffers();

	//3. SwapChain
	std::unique_ptr<MvSwapChain> m_swapChain;

	//4. Window
	std::unique_ptr<MvWindow> m_window;
	static constexpr int WIDTH = 800;
	static constexpr int HEIGHT = 600;
	void DrawFrame();

	//5. Model
	void LoadModels();
	std::unique_ptr<MvModel> m_model;

};

