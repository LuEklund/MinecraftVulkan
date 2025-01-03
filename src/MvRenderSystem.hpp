#pragma once

#include "MvPipeline.h"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"

#include <memory>

struct SimplePushConstantData
{
	glm::mat2 transform{1.f};
	glm::vec2 offset;
	alignas(16) glm::vec3 color;
};

class MvRenderSystem
{
public:
	MvRenderSystem(MvDevice &device, VkRenderPass renderPass);
	~MvRenderSystem();

	MvRenderSystem(const MvRenderSystem&) = delete;
	MvRenderSystem& operator=(const MvRenderSystem&) = delete;

	void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<MvGameObject> &gameObjects);

private:

	//1. Device
	MvDevice &m_Device;

	//2. Pipeline
	std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	void	CreatepipelineLayout();
	void	CreatePipeline(VkRenderPass renderPass);

};

