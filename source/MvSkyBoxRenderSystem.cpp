//
// Created by Lucas on 2025-02-03.
//

#include "MvSkyBoxRenderSystem.hpp"
#include <stdexcept>
#include <array>
#include <vector>
#include "glm/gtc/constants.hpp"

#include <iostream>

#include "MvChunk.hpp"


MvSkyBoxRenderSystem::MvSkyBoxRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout SkyBoxSetLayout)
    : m_Device(device)
{
	CreatepipelineLayout(SkyBoxSetLayout);
	CreatePipeline(renderPass);
}

MvSkyBoxRenderSystem::~MvSkyBoxRenderSystem()
{
	if (m_pipelineLayout != VK_NULL_HANDLE) {
		vkDestroyPipelineLayout(m_Device.GetDevice(), m_pipelineLayout, nullptr);
	}
}



void MvSkyBoxRenderSystem::CreatepipelineLayout(VkDescriptorSetLayout SkyBoxSetLayout)
{

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {SkyBoxSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void MvSkyBoxRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline: pipeline before pipeline layout!");
	PipelineConfigInfo PipelineConfig{};
	MvPipeline::DefaultPipelineConfigInfo(PipelineConfig);
	//Special for SkyBox
	PipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
	PipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;

	// PipelineConfigInfo pipelineConfig = MvPipeline::DefaultPipelineConfigInfo(m_swapChain->GetWidth(), m_swapChain->GetHeigth());
	PipelineConfig.renderPass = renderPass;
	PipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<MvPipeline>(m_Device, PipelineConfig, PipelineType::SKYBOX);
}


void MvSkyBoxRenderSystem::RenderSkyBox(const MvFrameInfo &frameInfo, MvModel &SkyBox)
{
	m_pipeline->Bind(frameInfo.commandBuffer);
	vkCmdBindDescriptorSets(
		frameInfo.commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0, 1,
		&frameInfo.globalDescriptorSet,
		0,
		nullptr);

	SkyBox.bind(frameInfo.commandBuffer);
	SkyBox.draw(frameInfo.commandBuffer);

}



