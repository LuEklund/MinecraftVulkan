#pragma once

#include "MvDevice.hpp"

#include <vector>
#include <string>

struct PipelineConfigInfo
{
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
	VkPipelineRasterizationStateCreateInfo rasterizationInfo;
	VkPipelineMultisampleStateCreateInfo multisampleInfo;
	VkPipelineColorBlendAttachmentState colorBlendAttachment;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class MvPipeline
{
public:
	MvPipeline(MvDevice &eDevice, const PipelineConfigInfo &ConfigInfo);
	~MvPipeline();

	MvPipeline(const MvPipeline&) = delete;
	MvPipeline& operator=(const MvPipeline&) = delete;

	void Bind(VkCommandBuffer commandBuffer);

	static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t heigth);

private:
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

	MvDevice& m_Device;
	VkPipeline m_graphicsPipeline;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;

	std::string m_vertexShader = "shaders/vert.spv";
	std::string m_fragmentShader = "shaders/frag.spv";

	static std::vector<char> readFile(const std::string &filePath);

	void createGraphicsPipeline(const PipelineConfigInfo& configInfo);
};

