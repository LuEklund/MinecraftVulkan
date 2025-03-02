#include "MvRenderSystem.hpp"
#include <stdexcept>
#include <array>
#include <vector>
#include "glm/gtc/constants.hpp"

#include <iostream>

#include "MvChunk.hpp"


MvRenderSystem::MvRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout)
    : m_Device(device)
{
	CreatepipelineLayout(globalSetLayout);
	CreatePipeline(renderPass);
}

MvRenderSystem::~MvRenderSystem()
{
	vkDestroyPipelineLayout(m_Device.GetDevice(), m_pipelineLayout, nullptr);
}



void MvRenderSystem::CreatepipelineLayout(VkDescriptorSetLayout globalSetLayout)
{

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts = {globalSetLayout};

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

	if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create pipeline layout!");
	}
}

void MvRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
	assert(m_pipelineLayout != nullptr && "Cannot create pipeline: pipeline before pipeline layout!");
	PipelineConfigInfo PipelineConfig{};
	MvPipeline::DefaultPipelineConfigInfo(PipelineConfig);
	// PipelineConfig.rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	// PipelineConfigInfo pipelineConfig = MvPipeline::DefaultPipelineConfigInfo(m_swapChain->GetWidth(), m_swapChain->GetHeigth());
	PipelineConfig.renderPass = renderPass;
	PipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<MvPipeline>(m_Device, PipelineConfig);
}


void MvRenderSystem::RenderChunks(const MvFrameInfo &frameInfo,std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> &chunks)
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

	for (auto& obj : chunks)
	{
		auto& chunk = obj.second;
		if (!chunk->HasMesh() || !chunk->GetRender()) continue;

		SimplePushConstantData push{};
		push.modelMatrix = {{1.f, 0.f, 0.f, 0.f},
							{0.f, 1.f, 0.f, 0.f},
							{0.f, 0.f, 1.f, 0.f},
							{obj.first * float(MvChunk::CHUNK_SIZE), 1.f}};

		vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		chunk->GetModel()->bind(frameInfo.commandBuffer);
		chunk->GetModel()->draw(frameInfo.commandBuffer);
	}


}

void MvRenderSystem::RenderGameObjects(MvFrameInfo &frameInfo, std::vector<MvGameObject> &gameObjects)
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

	// auto projectionView = frameInfo.camera.GetProjectionMatrix() * frameInfo.camera.GetViewMatrix();

	for (auto& obj : gameObjects)
	{
		// auto pos = obj.transform.mat4();
		// std::cout << "Rendering object x-pos: " << obj.transform.translation.x << std::endl;
		// obj.transform.rotation.y += glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
		// obj.transform.rotation.x += glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

		SimplePushConstantData push{};
		push.modelMatrix = obj.transform.mat4();
		// modelMatrix
		// push.modelMatrix = obj.transform.normalMatrix();
		// push.normalMatrix = modelMatrix;

		vkCmdPushConstants(frameInfo.commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		
		obj.model->bind(frameInfo.commandBuffer);
		obj.model->draw(frameInfo.commandBuffer);
	}
}



