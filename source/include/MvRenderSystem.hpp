#pragma once

#include "MvCamera.hpp"
#include "MvPipeline.hpp"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"
#include "MvFrameInfo.hpp"

#include <memory>

class MvChunk;

struct SimplePushConstantData
{
	glm::mat4 modelMatrix{1.f};
};

class MvRenderSystem
{
public:
	MvRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~MvRenderSystem();


	MvRenderSystem(const MvRenderSystem&) = delete;
	MvRenderSystem& operator=(const MvRenderSystem&) = delete;

	void RenderChunks(const MvFrameInfo & frame_info, std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> &chunks);
	void RenderGameObjects(MvFrameInfo &frameInfo, std::vector<MvGameObject> &gameObjects);

private:

	//1. Device
	MvDevice &m_Device;

	//2. Pipeline
	std::unique_ptr<MvPipeline> m_pipeline;
	// std::unique_ptr<MvPipeline> m_pipeline;
	VkPipelineLayout m_pipelineLayout;
	void	CreatepipelineLayout(VkDescriptorSetLayout globalSetLayout);
	void	CreatePipeline(VkRenderPass renderPass);

};

