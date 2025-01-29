#include "MvPipeline.hpp"
#include "MvModel.hpp"

#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "ShaderCompiler.hpp"


MvPipeline::MvPipeline(MvDevice& eDevice, const PipelineConfigInfo& ConfigInfo)
	: m_Device(eDevice)
{
	createGraphicsPipeline(ConfigInfo);
}

MvPipeline::~MvPipeline()
{
	vkDestroyShaderModule(m_Device.GetDevice(), m_vertexShaderModule, nullptr);
	vkDestroyShaderModule(m_Device.GetDevice(), m_fragmentShaderModule, nullptr);
	vkDestroyPipeline(m_Device.GetDevice(), m_graphicsPipeline, nullptr);
}

std::vector<char> MvPipeline::readFile(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file: " + filePath);
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void MvPipeline::createGraphicsPipeline(const PipelineConfigInfo& configInfo)
{
	assert(
		configInfo.pipelineLayout != VK_NULL_HANDLE &&
		"Cannot create graphics pipeline: no pipelineLayout provided in configInfo");
	assert(
		configInfo.renderPass != VK_NULL_HANDLE &&
		"Cannot create graphics pipeline: no renderPass provided in configInfo");

	// std::vector<char>	vertShaderCode = readFile(m_vertexShader);
	// std::vector<char>	fragShaderCode = readFile(m_fragmentShader);


	std::vector<uint32_t> vertexShaderCode;
	std::vector<uint32_t> fragShaderCode;
	if (!CompileShader("shaders/block.vert", Vertex, vertexShaderCode))
	{
		throw std::runtime_error("Failed to compile vertex shader");
	}

	if (!CompileShader("shaders/block.frag", Fragment, fragShaderCode))
	{
		throw std::runtime_error("Failed to compile fragment shader");
	}


	CreateShaderModule(vertexShaderCode, &m_vertexShaderModule);
	CreateShaderModule(fragShaderCode, &m_fragmentShaderModule);

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = m_vertexShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = m_fragmentShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;


	auto bindingDescriptions = MvModel::Vertex::getBindingDescriptions();
	auto attributeDescriptions = MvModel::Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
	pipelineInfo.pViewportState = &configInfo.viewportInfo;
	pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
	pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
	pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
	pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
	pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

	pipelineInfo.layout = configInfo.pipelineLayout;
	pipelineInfo.renderPass = configInfo.renderPass;
	pipelineInfo.subpass = configInfo.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	if (vkCreateGraphicsPipelines(m_Device.GetDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create graphics pipeline!");
	}

	std::cout << "Vertex Shader: " << vertexShaderCode.size() << " bytes" << std::endl;
	std::cout << "Fragment Shader: " << fragShaderCode.size() << " bytes" << std::endl;
}


void MvPipeline::CreateShaderModule(const std::vector<uint32_t>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size() * sizeof(uint32_t);
	createInfo.pCode = code.data();

	if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create shader module!");
	}
}

void MvPipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
}

void MvPipeline::DefaultPipelineConfigInfo(PipelineConfigInfo &ConfigInfo)
{
	ConfigInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	ConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	// ConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	// ConfigInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	ConfigInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	ConfigInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	ConfigInfo.viewportInfo.viewportCount = 1;
	ConfigInfo.viewportInfo.pViewports = nullptr;
	ConfigInfo.viewportInfo.scissorCount = 1;
	ConfigInfo.viewportInfo.pScissors = nullptr;

	ConfigInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	ConfigInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
	ConfigInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	ConfigInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	ConfigInfo.rasterizationInfo.lineWidth = 1.f;
	ConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	ConfigInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	ConfigInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
	ConfigInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;  // Optional
	ConfigInfo.rasterizationInfo.depthBiasClamp = 0.0f;           // Optional
	ConfigInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;     // Optional

	ConfigInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	ConfigInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
	ConfigInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	ConfigInfo.multisampleInfo.minSampleShading = 1.0f;           // Optional
	ConfigInfo.multisampleInfo.pSampleMask = nullptr;             // Optional
	ConfigInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;  // Optional
	ConfigInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;       // Optional

	ConfigInfo.colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	ConfigInfo.colorBlendAttachment.blendEnable = VK_FALSE;
	ConfigInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	ConfigInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	ConfigInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
	ConfigInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
	ConfigInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
	ConfigInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

	ConfigInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	ConfigInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
	ConfigInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;  // Optional
	ConfigInfo.colorBlendInfo.attachmentCount = 1;
	ConfigInfo.colorBlendInfo.pAttachments = &ConfigInfo.colorBlendAttachment;
	ConfigInfo.colorBlendInfo.blendConstants[0] = 0.0f;  // Optional
	ConfigInfo.colorBlendInfo.blendConstants[1] = 0.0f;  // Optional
	ConfigInfo.colorBlendInfo.blendConstants[2] = 0.0f;  // Optional
	ConfigInfo.colorBlendInfo.blendConstants[3] = 0.0f;  // Optional

	ConfigInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	ConfigInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
	ConfigInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
	ConfigInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	ConfigInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	ConfigInfo.depthStencilInfo.minDepthBounds = 0.0f;  // Optional
	ConfigInfo.depthStencilInfo.maxDepthBounds = 1.0f;  // Optional
	ConfigInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
	ConfigInfo.depthStencilInfo.front = {};  // Optional
	ConfigInfo.depthStencilInfo.back = {};   // Optional

	ConfigInfo.dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	ConfigInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	ConfigInfo.dynamicStateInfo.pDynamicStates = ConfigInfo.dynamicStateEnables.data();
	ConfigInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(ConfigInfo.dynamicStateEnables.size());
	ConfigInfo.dynamicStateInfo.flags = 0;

}
