#include "MvApp.h"

#include "MvController.hpp"
#include "MvCamera.hpp"
#include "MvRenderSystem.hpp"
#include "MvBuffer.hpp"

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
#include <numeric>
#include <iostream>

struct GlobalUbo
{
  glm::mat4 projectionView{1.f};
  glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, 0.f, 0.f});
};


MvApp::MvApp()
{
	m_window = std::make_unique<MvWindow>(WIDTH, HEIGHT, "MC Vulkan");
	m_Device = std::make_unique<MvDevice>(*m_window);
	m_renderer = std::make_unique<MvRenderer>(*m_window, *m_Device);
  m_GlobalPool = MvDescriptorPool::Builder(*m_Device)
      .setMaxSets(MvSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MvSwapChain::MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MvSwapChain::MAX_FRAMES_IN_FLIGHT)
      .build();
LoadBlocks();
}

MvApp::~MvApp()
{
}

void MvApp::Run()
{
  std::vector<std::unique_ptr<MvBuffer>> uboBuffers(MvSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (size_t i = 0; i < uboBuffers.size(); i++)
  {
    uboBuffers[i] = std::make_unique<MvBuffer>(
        *m_Device,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
  }


  auto globalSetLayout = MvDescriptorSetLayout::Builder(*m_Device)
      .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
      .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
      .build();

  std::vector<VkDescriptorSet> globalDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};

  for (int i = 0; i < globalDescriptorSets.size(); i++)
  {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_cubeModel->GetTextureImageView();
    imageInfo.sampler = m_cubeModel->GetTextureSampler();

    MvDescriptorWriter(*globalSetLayout, *m_GlobalPool)
      .writeBuffer(0, &bufferInfo)
      .writeImage(1, &imageInfo)
      .build(globalDescriptorSets[i]);
  }
  

	MvRenderSystem renderSystem(*m_Device,
      m_renderer->GetSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout());

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
      int frameIndex = m_renderer->GetFrameIndex();
      MvFrameInfo frameInfo{frameIndex,
                            frameTime,
                            CommandBuffer,
                            camera,
                            globalDescriptorSets[frameIndex]};

      //update
      GlobalUbo ubo{};
      ubo.projectionView = camera.GetProjectionMatrix() * camera.GetViewMatrix();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      //render
      float aspect = m_renderer->GetAspectRatio();
      camera.SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 50.f);

			m_renderer->BeginSwapChainRenderPass(CommandBuffer);
      for (MvChunk& chunk : m_chunks)
      {
        renderSystem.RenderGameObjects(frameInfo, chunk.GetGameObjects());
      }
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
        // Left face
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
        {{-.5f, .5f, .5f},  {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // Top-right
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom-right
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Top-left

        // Right face
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, .5f},  {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        // Top face
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, -.5f, .5f},  {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},

        // Bottom face
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, .5f},  {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},

        // Front face
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, 0.5f},  {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},

        // Back face
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, -0.5f},  {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    };
  for (auto& v : modelBuilder.vertices) {
    v.position += offset;
  }
  //Debug Vertex
//    modelBuilder.indices = {
//     // Triangle 1
//     0, 1, 1, 2, 2, 0,
//     // Triangle 2
//     0, 3, 3, 1, 1, 0,
//     // Triangle 3
//     4, 5, 5, 6, 6, 4,
//     // Triangle 4
//     4, 7, 7, 5, 5, 4,
//     // Triangle 5
//     8, 9, 9, 10, 10, 8,
//     // Triangle 6
//     8, 11, 11, 9, 9, 8,
//     // Triangle 7
//     12, 13, 13, 14, 14, 12,
//     // Triangle 8
//     12, 15, 15, 13, 13, 12,
//     // Triangle 9
//     16, 17, 17, 18, 18, 16,
//     // Triangle 10
//     16, 19, 19, 17, 17, 16,
//     // Triangle 11
//     20, 21, 21, 22, 22, 20,
//     // Triangle 12
//     20, 23, 23, 21, 21, 20,
// };

  modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                          12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
 
  return std::make_unique<MvModel>(device, modelBuilder);
}



void MvApp::LoadBlocks()
{
  m_cubeModel = CreateCubeModel(*m_Device, {0.f, 0.f, 0.f});
  int size = 2;
  for (int x = 0; x < size; x++)
  {
      for (int y = 0; y < size; y++)
      {
        for (int z = 0; z < size; z++)
        {
          // auto cube = MvGameObject::createGameObject();
          auto chunk = MvChunk();
          for (auto &cube : chunk.GetGameObjects())
          {
            cube.model = m_cubeModel;
          }
          chunk.SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
          
          // cube.transform.translation = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
          // cube.transform.scale = {1.f, 1.f, 1.f};
          m_chunks.push_back(std::move(chunk));
      }
    }
  }
}

