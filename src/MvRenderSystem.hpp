#pragma once

#include "MvCamera.hpp"
#include "MvPipeline.h"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"
#include "MvFrameInfo.hpp"

#include <memory>

struct SimplePushConstantData
{
	glm::mat4 transform{1.f};
	glm::mat4 modelMatrix{1.f};
};

class MvRenderSystem
{
public:
	MvRenderSystem(MvDevice &device, VkRenderPass renderPass);
	~MvRenderSystem();

	MvRenderSystem(const MvRenderSystem&) = delete;
	MvRenderSystem& operator=(const MvRenderSystem&) = delete;

	void RenderGameObjects(MvFrameInfo &frameInfo, std::vector<MvGameObject> &gameObjects);

private:

	//1. Device
	MvDevice &m_Device;

	//2. Pipeline
	std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	void	CreatepipelineLayout();
	void	CreatePipeline(VkRenderPass renderPass);

};

