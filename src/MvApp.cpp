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
  m_texture = std::make_unique<MvTexture>(*m_Device);
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

  for (size_t i = 0; i < globalDescriptorSets.size(); i++)
  {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_texture->GetTextureImageView();
    imageInfo.sampler = m_texture->GetTextureSampler();

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

  camera.SetViewTarget(glm::vec3{0.f, -2.f, 2.f}, glm::vec3{0.f, -1.f, -1.f});
  
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
      // for (MvChunk& chunk : m_chunks)
      // {
      renderSystem.RenderGameObjects(frameInfo, m_gameObjects);
      // }
			m_renderer->EndSwapChainRenderPass(CommandBuffer);
			m_renderer->EndFrame();
		}
	}
	vkDeviceWaitIdle(m_Device->GetDevice());
}




//Make chunks
void MvApp::LoadBlocks()
{

  // m_cubeModel = CreateCubeModel(*m_Device, {0.f, 0.f, 0.f});
  int size = 3;
  for (int x = 0; x < size; x++)
  {
      for (int y = 0; y < size; y++)
      {
        for (int z = 0; z < size; z++)
        {
          auto chunk = MvChunk();
          chunk.SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
          chunk.GenerateMesh(*m_Device, m_gameObjects);
          m_chunks.push_back(std::move(chunk));
      }
    }
  }
}

