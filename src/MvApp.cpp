#include "MvApp.h"

#include "MvCamera.hpp"
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
	MvCamera camera{};
  // camera.SetViewDirection(glm::vec3{0.f}, glm::vec3{0.5f, 0.f, 1.f});
  camera.SetViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});
  
  while (!m_window->ShouldClose())
	{
		glfwPollEvents();
		if (auto CommandBuffer = m_renderer->BeginFrame())
		{
      float aspect = m_renderer->GetAspectRatio();
      camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 10.f);
      //Orthographic projection 1 by 1 by 1 cube
      // camera.SetOrthographicProjection(-aspect, aspect, -1.f, 1.f, -1.f, 1.f);

			m_renderer->BeginSwapChainRenderPass(CommandBuffer);
			renderSystem.RenderGameObjects(CommandBuffer, m_GameObjects, camera);
			m_renderer->EndSwapChainRenderPass(CommandBuffer);
			m_renderer->EndFrame();
		}
	}
	vkDeviceWaitIdle(m_Device->GetDevice());
}

// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<MvModel> MvApp::CreateCubeModel(MvDevice& device, glm::vec3 offset) {
  std::vector<MvModel::Vertex> vertices{
 
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
 
      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
 
      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
 
      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
 
      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
 
      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
 
  };
  for (auto& v : vertices) {
    v.position += offset;
  }
  return std::make_unique<MvModel>(device, vertices);
}

void MvApp::LoadBlocks()
{
	std::shared_ptr<MvModel> cubeModel = CreateCubeModel(*m_Device, {0.f, 0.f, 0.f});
	// auto cube = MvGameObject::createGameObject();
	// cube.model = cubeModel;
	// // cube.transform.rotation = {0.6f, 1.f, 1.1f};
	// cube.transform.translation = {0.f, 0.f, 0.5f};
	// cube.transform.scale = {.5f, .5f, .5f};
	// m_GameObjects.push_back(std::move(cube));

	for (int x = -1; x < 2; x++)
	{
		for (int y = -1; y < 2; y++)
		{
			auto cube = MvGameObject::createGameObject();
			cube.model = cubeModel;
			cube.transform.rotation = {0.6f, 1.f, 1.1f};
			cube.transform.translation = {.51f * static_cast<float>(x), .51f * static_cast<float>(y), 2.5f};
			cube.transform.scale = {.5f, .5f, .5f};
			m_GameObjects.push_back(std::move(cube));
		}
	}
}

