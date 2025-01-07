#pragma once

#include "MvCamera.hpp"
#include "MvPipeline.h"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"

#include <memory>

struct SimplePushConstantData
{
	glm::mat4 transform{1.f};
	alignas(16) glm::vec3 color;
};

class MvRenderSystem
{
public:
	MvRenderSystem(MvDevice &device, VkRenderPass renderPass);
	~MvRenderSystem();

	MvRenderSystem(const MvRenderSystem&) = delete;
	MvRenderSystem& operator=(const MvRenderSystem&) = delete;

	void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<MvGameObject> &gameObjects, const MvCamera &camera);

private:

	//1. Device
	MvDevice &m_Device;

	//2. Pipeline
	std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	void	CreatepipelineLayout();
	void	CreatePipeline(VkRenderPass renderPass);

};

