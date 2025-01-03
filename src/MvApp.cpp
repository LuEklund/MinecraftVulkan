#include "MvApp.h"

#include "MvRenderSystem.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtc/constants.hpp"

// std
#include <stdexcept>
#include <array>
#include <vector>

MvApp::MvApp()
{
	m_window = std::make_unique<MvWindow>(WIDTH, HEIGHT, "MC Vulkan");
	m_Device = std::make_unique<MvDevice>(*m_window);
	m_renderer = std::make_unique<MvRenderer>(*m_window, *m_Device);
	LoadBlocks();
}

MvApp::~MvApp()
{
}

void MvApp::Run()
{
	MvRenderSystem renderSystem(*m_Device, m_renderer->GetSwapChainRenderPass());
	while (!m_window->ShouldClose())
	{
		glfwPollEvents();
		if (auto CommandBuffer = m_renderer->BeginFrame())
		{
			m_renderer->BeginSwapChainRenderPass(CommandBuffer);
			renderSystem.RenderGameObjects(CommandBuffer, m_GameObjects);
			m_renderer->EndSwapChainRenderPass(CommandBuffer);
			m_renderer->EndFrame();
		}
	}
	vkDeviceWaitIdle(m_Device->GetDevice());
}

void MvApp::LoadBlocks()
{
	std::vector<MvModel::Vertex> vertices = {
		{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
	};
	auto m_model = std::make_shared<MvModel>(*m_Device, vertices);
	auto triangle = MvGameObject::createGameObject();
	triangle.model = m_model;
	triangle.color = {0.0f, 0.8f, 0.1f};
	triangle.transform2d.translation.x = 0.2f;
	triangle.transform2d.scale = {2.f, 0.5f};
	triangle.transform2d.rotation = 0.25f * glm::two_pi<float>();
	m_GameObjects.push_back(std::move(triangle));

	// std::vector<MvModel::Vertex> vertices{};
	// sierpinski(vertices, 10, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.0f, -0.5f});
	// m_model = std::make_unique<MvModel>(*m_Device, vertices);
}


void MvApp::sierpinski(
    std::vector<MvModel::Vertex> &vertices, int depth, glm::vec2 left, glm::vec2 right, glm::vec2 top)
	{

	if (depth <= 0) {
		vertices.push_back({top});
		vertices.push_back({right});
		vertices.push_back({left});
	} else {
		auto leftTop = 0.5f * (left + top);
		auto rightTop = 0.5f * (right + top);
		auto leftRight = 0.5f * (left + right);
		sierpinski(vertices, depth - 1, left, leftRight, leftTop);
		sierpinski(vertices, depth - 1, leftRight, right, rightTop);
		sierpinski(vertices, depth - 1, leftTop, rightTop, top);
  }
}


