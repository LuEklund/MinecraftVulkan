#include "MvApp.h"
#include <stdexcept>
#include <array>
#include <vector>

//MvApp::MvApp()
//{
//	m_window = new MvWindow(WIDTH, HEIGHT, "MC Vulkan");
//	m_Device = new MvDevice(*m_window);
//	m_swapChain = new MvSwapChain(*m_Device, m_window->GetExtent());
//	LoadModels();
//
//	CreatepipelineLayout();
//	CreatePipeline();
//	CreateCommandBuffers();
//	//m_pipeline = new MvPipeline(*m_Device, MvPipeline::DefaultPipelineConfigInfo(WIDTH, HEIGHT));
//}
//
//MvApp::~MvApp()
//{
//	m_pipeline->~MvPipeline();
//	vkDestroyPipelineLayout(m_Device->GetDevice(), m_pipelineLayout, nullptr);
//	delete m_swapChain;
//	delete m_window;
//	delete m_Device;
//}

MvApp::MvApp()
{
	m_window = std::make_unique<MvWindow>(WIDTH, HEIGHT, "MC Vulkan");
	m_Device = std::make_unique<MvDevice>(*m_window);
	m_swapChain = std::make_unique<MvSwapChain>(*m_Device, m_window->GetExtent());
	LoadModels();

	CreatepipelineLayout();
	CreatePipeline();
	CreateCommandBuffers();
}

MvApp::~MvApp()
{
	vkDestroyPipelineLayout(m_Device->GetDevice(), m_pipelineLayout, nullptr);
}

void MvApp::Run()
{
	while (!m_window->ShouldClose())
	{
		glfwPollEvents();
		DrawFrame();
	}
	vkDeviceWaitIdle(m_Device->GetDevice());
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
	m_CommandBuffers.resize(m_swapChain->GetimageCount());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_Device->GetCommandPool();
	allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

	if (vkAllocateCommandBuffers(m_Device->GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to allocate command buffers!");
	}

	for (size_t i = 0; i < m_CommandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(m_CommandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = m_swapChain->GetRenderPass();
		renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(i);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_swapChain->GetSwapChainExtent();
		// renderPassInfo.renderArea.exent = {static_cast<int32_t>(m_swapChain->GetWidth()), static_cast<int32_t>(m_swapChain->GetHeigth())};

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(m_CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
		m_pipeline->Bind(m_CommandBuffers[i]);
		m_model->bind(m_CommandBuffers[i]);
		m_model->draw(m_CommandBuffers[i]);
		//vkCmdDraw(m_CommandBuffers[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(m_CommandBuffers[i]);
		if (vkEndCommandBuffer(m_CommandBuffers[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("Failed to record command buffer!");
		}
	}
}

void MvApp::DrawFrame()
{
	uint32_t imageIndex;
	VkResult res = m_swapChain->acquireNextImage(&imageIndex);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}

	res = m_swapChain->submitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
	if (res != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to acquire swap chain image!");
	}
}

void MvApp::LoadModels()
{
	std::vector<MvModel::Vertex> vertices = {
		{{0.0f, -0.5f}},
		{{0.5f, 0.5f}},
		{{-0.5f, 0.5f}}
	};
	m_model = std::make_unique<MvModel>(*m_Device, vertices);
}
