#include "MvApp.h"

#include "MvController.hpp"
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
#include <chrono>

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
  auto viewerObject = MvGameObject::createGameObject();
  MvController CameraController{};

  camera.SetViewTarget(glm::vec3{-1.f, -2.f, 2.f}, glm::vec3{0.f, 0.f, 2.5f});
  
  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!m_window->ShouldClose())
	{
		glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    CameraController.MoveInPlaneXZ(m_window->GetWindow(), viewerObject, frameTime);
    camera.SetViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

		if (auto CommandBuffer = m_renderer->BeginFrame())
		{
      float aspect = m_renderer->GetAspectRatio();
      camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

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
  MvModel::Builder modelBuilder{};
  modelBuilder.vertices = {
      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
 
      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
 
      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
 
      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
 
      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
 
      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
  };
  for (auto& v : modelBuilder.vertices) {
    v.position += offset;
  }
  //Debug Vertex
   modelBuilder.indices = {
    // Triangle 1
    0, 1, 1, 2, 2, 0,
    // Triangle 2
    0, 3, 3, 1, 1, 0,
    // Triangle 3
    4, 5, 5, 6, 6, 4,
    // Triangle 4
    4, 7, 7, 5, 5, 4,
    // Triangle 5
    8, 9, 9, 10, 10, 8,
    // Triangle 6
    8, 11, 11, 9, 9, 8,
    // Triangle 7
    12, 13, 13, 14, 14, 12,
    // Triangle 8
    12, 15, 15, 13, 13, 12,
    // Triangle 9
    16, 17, 17, 18, 18, 16,
    // Triangle 10
    16, 19, 19, 17, 17, 16,
    // Triangle 11
    20, 21, 21, 22, 22, 20,
    // Triangle 12
    20, 23, 23, 21, 21, 20,
};
  // modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
  //                         12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
 
  return std::make_unique<MvModel>(device, modelBuilder);
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

  int size = 10;
  for (int x = -1; x < size; x++)
  {
      for (int y = -1; y < size; y++)
      {
        for (int z = -1; z < size; z++)
        {
          auto cube = MvGameObject::createGameObject();
          cube.model = cubeModel;
          // cube.transform.rotation = {0.6f, 1.f, 1.1f};
          cube.transform.translation = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
          cube.transform.scale = {1.f, 1.f, 1.f};
          m_GameObjects.push_back(std::move(cube));
      }
    }
  }
}

