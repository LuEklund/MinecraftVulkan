#include "MvApp.hpp"

#include "MvController.hpp"
#include "MvCamera.hpp"
#include "MvRenderSystem.hpp"
#include "MvSkyBoxRenderSystem.hpp"
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



struct GlobalUbo {
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, 0.f, 0.f});
};




MvApp::MvApp() {
    m_window = std::make_unique<MvWindow>(WIDTH, HEIGHT, "MC Vulkan");
    m_Device = std::make_unique<MvDevice>(*m_window);
    m_renderer = std::make_unique<MvRenderer>(*m_window, *m_Device);
    m_texture = std::make_unique<MvTexture>(*m_Device);
    m_CubeMap = std::make_unique<MvCubeMap>(*m_Device);
    m_GlobalPool = MvDescriptorPool::Builder(*m_Device)
            .setMaxSets(MvSwapChain::MAX_FRAMES_IN_FLIGHT * 2)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MvSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MvSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MvSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
    // LoadBlocks();
    m_World = std::make_unique<MvWorld>(*m_Device);


    MvModel::Builder builder;
    builder.vertices = {
        // Front face
        {{-1.0f, -1.0f,  1.0f}},
        {{ 1.0f, -1.0f,  1.0f}},
        {{ 1.0f,  1.0f,  1.0f}},
        {{-1.0f,  1.0f,  1.0f}},

        // Back face
        {{-1.0f, -1.0f, -1.0f}},
        {{ 1.0f, -1.0f, -1.0f}},
        {{ 1.0f,  1.0f, -1.0f}},
        {{-1.0f,  1.0f, -1.0f}},
    };
    builder.indices = {
        // Front face
        0, 1, 2,  2, 3, 0,
        // Right face
        1, 5, 6,  6, 2, 1,
        // Back face
        5, 4, 7,  7, 6, 5,
        // Left face
        4, 0, 3,  3, 7, 4,
        // Top face
        3, 2, 6,  6, 7, 3,
        // Bottom face
        4, 5, 1,  1, 0, 4
    };
    m_SkyBox = std::make_unique<MvModel>(*m_Device, builder);
    glfwSetWindowUserPointer(m_window->GetWindow(), this);
}

MvApp::~MvApp() {
}

void MvApp::Run() {
    //init stuff
    //Uniform buffer Chunks
    std::vector<std::unique_ptr<MvBuffer> > uboBuffers(MvSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < uboBuffers.size(); i++) {
        uboBuffers[i] = std::make_unique<MvBuffer>(
            *m_Device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffers[i]->map();
    }
    //Uniform buffer SkyBox
    std::vector<std::unique_ptr<MvBuffer> > SkyBoxBuffer(MvSwapChain::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < SkyBoxBuffer.size(); i++) {
        SkyBoxBuffer[i] = std::make_unique<MvBuffer>(
            *m_Device,
            sizeof(CameraVectors),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        SkyBoxBuffer[i]->map();
    }


    auto globalSetLayout = MvDescriptorSetLayout::Builder(*m_Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    auto skySetLayout = MvDescriptorSetLayout::Builder(*m_Device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();


    std::vector<VkDescriptorSet> globalDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::vector<VkDescriptorSet> SkyBoxDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};

    for (size_t i = 0; i < globalDescriptorSets.size(); i++) {
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
    for (size_t i = 0; i < SkyBoxDescriptorSets.size(); i++) {
        auto bufferInfo = SkyBoxBuffer[i]->descriptorInfo();
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_CubeMap->GetTextureImageView();
        imageInfo.sampler = m_CubeMap->GetTextureSampler();

        MvDescriptorWriter(*skySetLayout, *m_GlobalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(SkyBoxDescriptorSets[i]);
    }


    MvRenderSystem renderSystem(*m_Device,
                                m_renderer->GetSwapChainRenderPass(),
                                globalSetLayout->getDescriptorSetLayout());

    MvSkyBoxRenderSystem SkyBoxRenderSystem(*m_Device,
                            m_renderer->GetSwapChainRenderPass(),
                            skySetLayout->getDescriptorSetLayout());



    m_Camera = std::make_unique<MvCamera>();
    m_Camera->SetUpListeners(m_window->GetWindow());
    m_Camera->SetViewTarget(glm::vec3{0.f, -2.f, 2.f}, glm::vec3{0.f, -1.f, -1.f});
    auto currentTime = std::chrono::high_resolution_clock::now();


    //Update
    while (!m_window->ShouldClose()) {
        glfwPollEvents();

        if (auto CommandBuffer = m_renderer->BeginFrame()) {
            // m_Camera->SetUpListeners(m_window->GetWindow());

            // glm::vec4 cam_vec_forwards = {1.0f, 0.0f, 0.0f, 0.0f};
            // glm::vec4 cam_vec_right = {0.0f, -1.0f, 0.0f, 0.0f};
            // glm::vec4 cam_vec_up = {0.0f, 0.0f, 1.0f, 0.0f};

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            m_Camera->MoveInPlaneXZ(m_window->GetWindow(), frameTime);
            m_Camera->SetViewYXZ();

            float aspect = m_renderer->GetAspectRatio();
            m_Camera->SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

            // m_World->LoadChunksAtCoordinate(m_Camera->GetPosition());

            int frameIndex = m_renderer->GetFrameIndex();
            MvFrameInfo frameInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
                *m_Camera.get(),
                globalDescriptorSets[frameIndex]
            };
            MvFrameInfo SkyframeInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
                *m_Camera.get(),
                SkyBoxDescriptorSets[frameIndex]
            };

            //update
            GlobalUbo ubo{};
            ubo.projectionView = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();


            CameraVectors CameraVectors{
                glm::vec4(m_Camera->GetForward(), 0.f),
                glm::vec4(m_Camera->GetRight(), 0.f),
                glm::vec4(glm::cross(m_Camera->GetRight(), m_Camera->GetForward()), 0.f)  // Recalculate up
            };
            SkyBoxBuffer[frameIndex]->writeToBuffer(&CameraVectors);
            SkyBoxBuffer[frameIndex]->flush();


            //render
            m_renderer->BeginSwapChainRenderPass(CommandBuffer);
            m_World->CalculateRenderChunks(m_Camera->GetPosition(), m_Camera->GetForward(), 3, m_Camera->GetFovRadians() * 0.8f);
            SkyBoxRenderSystem.RenderSkyBox(SkyframeInfo, *m_SkyBox);
            renderSystem.RenderChunks(frameInfo, m_World->GetChunks());
            m_renderer->EndSwapChainRenderPass(CommandBuffer);
            m_renderer->EndFrame();
        }

    }
    vkDeviceWaitIdle(m_Device->GetDevice());
}



