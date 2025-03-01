#include "MvUIRenderSystem.hpp"
#include <stdexcept>
#include <array>
#include <vector>

#include <iostream>

#include "MvChunk.hpp"


MvUIRenderSystem::MvUIRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : m_Device(device)
{
	CreatepipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);
}

MvUIRenderSystem::~MvUIRenderSystem()
{
	vkDestroyPipelineLayout(m_Device.GetDevice(), m_pipelineLayout, nullptr);
}



void MvUIRenderSystem::CreatepipelineLayout(VkDescriptorSetLayout globalSetLayout)
{

//	VkPushConstantRange pushConstantRange{};
//	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
//	pushConstantRange.offset = 0;
//	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
//	pipelineLayoutInfo.pushConstantRangeCount = 1;
//	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void MvUIRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline: pipeline before pipeline layout!");
	PipelineConfigInfo PipelineConfig{};
	MvPipeline::DefaultPipelineConfigInfo(PipelineConfig);

	PipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;

	//Depth special for UI
	PipelineConfig.depthStencilInfo.depthTestEnable = VK_FALSE;
	PipelineConfig.depthStencilInfo.depthWriteEnable = VK_FALSE;
	PipelineConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_ALWAYS; // Always render UI, no depth check
	PipelineConfig.depthStencilInfo.stencilTestEnable = VK_FALSE; // No stencil test

	//Blend special for UI
	PipelineConfig.colorBlendAttachment.blendEnable = VK_TRUE;
	PipelineConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
	PipelineConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
	PipelineConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	PipelineConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;   // Optional
	PipelineConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;  // Optional
	PipelineConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional



	// PipelineConfigInfo pipelineConfig = MvPipeline::DefaultPipelineConfigInfo(m_swapChain->GetWidth(), m_swapChain->GetHeigth());
	PipelineConfig.renderPass = renderPass;
	PipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<MvPipeline>(m_Device, PipelineConfig, PipelineType::UI);
}

void MvUIRenderSystem::RenderUI(const MvFrameInfo &frameInfo, MvUIModel &UI) {
	m_pipeline->Bind(frameInfo.commandBuffer);
	// vkCmdBindDescriptorSets(
	// 	frameInfo.commandBuffer,
	// 	VK_PIPELINE_BIND_POINT_GRAPHICS,
	// 	m_pipelineLayout,
	// 	0, 1,
	// 	&frameInfo.globalDescriptorSet,
	// 	0,
	// 	nullptr);

	UI.bind(frameInfo.commandBuffer);
	UI.draw(frameInfo.commandBuffer);
}

