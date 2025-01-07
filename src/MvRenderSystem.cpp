#include "MvRenderSystem.hpp"
#include <stdexcept>
#include <array>
#include <vector>
#include "glm/gtc/constants.hpp"


MvRenderSystem::MvRenderSystem(MvDevice &device, VkRenderPass renderPass)
    : m_Device(device)
{
	CreatepipelineLayout();
	CreatePipeline(renderPass);
}

MvRenderSystem::~MvRenderSystem()
{
	vkDestroyPipelineLayout(m_Device.GetDevice(), m_pipelineLayout, nullptr);
}



void MvRenderSystem::CreatepipelineLayout()
{

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(SimplePushConstantData);

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
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

	//PipelineConfigInfo pipelineConfig = MvPipeline::DefaultPipelineConfigInfo(m_swapChain->GetWidth(), m_swapChain->GetHeigth());
	PipelineConfig.renderPass = renderPass;
	PipelineConfig.pipelineLayout = m_pipelineLayout;
	m_pipeline = std::make_unique<MvPipeline>(m_Device, PipelineConfig);
}





void MvRenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<MvGameObject> &gameObjects,  const MvCamera &camera)
{
	m_pipeline->Bind(commandBuffer);

	auto projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();

	for (auto& obj : gameObjects)
	{
		obj.transform.rotation.y += glm::mod(obj.transform.rotation.y + 0.01f, glm::two_pi<float>());
		obj.transform.rotation.x += glm::mod(obj.transform.rotation.x + 0.005f, glm::two_pi<float>());

		SimplePushConstantData push{};
		push.color = obj.color;
		push.transform = projectionView * obj.transform.mat4();
		vkCmdPushConstants(commandBuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(SimplePushConstantData), &push);
		
		obj.model->bind(commandBuffer);
		obj.model->draw(commandBuffer);
	}
}



