#include "MvApp.h"
#include <stdexcept>

MvApp::MvApp()
{
	m_window = new MvWindow(WIDTH, HEIGHT, "MC Vulkan");
	m_Device = new MvDevice(*m_window);
	m_swapChain = new MvSwapChain(*m_Device, m_window->GetExtent());

	CreatepipelineLayout();
	CreatePipeline();
	CreateCommandBuffers();
	//m_pipeline = new MvPipeline(*m_Device, MvPipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT));
}

MvApp::~MvApp()
{
	vkDestroyPipelineLayout(m_Device->GetDevice(), m_pipelineLayout, nullptr);
	m_pipeline->~MvPipeline();
	delete m_swapChain;
	delete m_window;
	delete m_Device;
}

void MvApp::Run()
{
	while (!m_window->ShouldClose())
	{
		glfwPollEvents();
	}
}

void MvApp::DrawFrame()
{
}

void MvApp::CreatepipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_Device->GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void MvApp::CreatePipeline()
{
	//assert(m_pipelineLayout != VK_NULL_HANDLE && "Cannot create pipeline: pipelineLayout is null");

	PipelineConfigInfo pipelineConfig = MvPipeline::DefaultPipelineConfigInfo(m_swapChain->GetWidth(), m_swapChain->GetHeigth());
	pipelineConfig.renderPass = m_swapChain->GetRenderPass();
	pipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<MvPipeline>(*m_Device, pipelineConfig);
}

void MvApp::CreateCommandBuffers()
{
}
