#pragma once

#include "MvPipeline.hpp"
#include "MvDevice.hpp"
#include "MvFrameInfo.hpp"
#include "MvUIModel.hpp"

#include <memory>



class MvUIRenderSystem
{
public:
	MvUIRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
	~MvUIRenderSystem();


	MvUIRenderSystem(const MvUIRenderSystem&) = delete;
	MvUIRenderSystem& operator=(const MvUIRenderSystem&) = delete;

	void RenderUI(const MvFrameInfo & frame_info, MvUIModel &UI);

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

