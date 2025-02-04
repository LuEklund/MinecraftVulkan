#pragma once

#include "MvDevice.hpp"

#include <vector>
#include <string>

#include "glm/vec4.hpp"

enum class PipelineType {
	GRAPHICS,
	SKYBOX
};

struct CameraVectors {
	glm::vec4 Forward;
	glm::vec4 Right;
	glm::vec4 Up;
};

struct PipelineConfigInfo
{
	//My compiler did not auto generate default constructor
	PipelineConfigInfo(){}

	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	//My compiler did not auto generate default constructor taht is why I init all struct with {}
	VkPipelineViewportStateCreateInfo viewportInfo{};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{};
	VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
	VkPipelineMultisampleStateCreateInfo multisampleInfo{};
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};

	std::vector<VkDynamicState> dynamicStateEnables{};
	VkPipelineDynamicStateCreateInfo dynamicStateInfo{};

	VkPipelineLayout pipelineLayout = nullptr;
	VkRenderPass renderPass = nullptr;
	uint32_t subpass = 0;
};

class MvPipeline
{
public:
	MvPipeline(MvDevice &eDevice, const PipelineConfigInfo &ConfigInfo, PipelineType type = PipelineType::GRAPHICS);
	~MvPipeline();

	MvPipeline(const MvPipeline&) = delete;
	MvPipeline& operator=(const MvPipeline&) = delete;

	void Bind(VkCommandBuffer commandBuffer);

	static void DefaultPipelineConfigInfo(PipelineConfigInfo &ConfigInfo);

private:
	void CreateShaderModule(const std::vector<uint32_t>& code, VkShaderModule* shaderModule);

	MvDevice& m_Device;
	VkPipeline m_Pipeline;
	VkShaderModule m_vertexShaderModule;
	VkShaderModule m_fragmentShaderModule;

	// std::string m_vertexShader = "shaders/vert.spv";
	// std::string m_fragmentShader = "shaders/frag.spv";

	static std::vector<char> readFile(const std::string &filePath);

	void createGraphicsPipeline(const PipelineConfigInfo& configInfo);
	void createSkyBoxPipeline(const PipelineConfigInfo& configInfo);
};

