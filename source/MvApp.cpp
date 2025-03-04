#include "MvApp.hpp"

#include "MvController.hpp"
#include "MvCamera.hpp"
#include "MvRenderSystem.hpp"
#include "MvSkyBoxRenderSystem.hpp"
#include "MvUIRenderSystem.hpp"
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



// CreateImageTexture("textures/MvTextures.png");
// CreateImageTexture("textures/MvTexturesLogo.png");
// CreateImageTexture("textures/MvTexturesRibbit.png");
// CreateImageTexture("textures/MvTexturesIris.png");
// CreateImageTexture("textures/Stune.png");
// CreateImageTexture("textures/RibbitCabaggeChad.png");
MvApp::MvApp() {
    m_window = std::make_unique<MvWindow>(WIDTH, HEIGHT, "MC Vulkan");
    m_Device = std::make_unique<MvDevice>(*m_window);
    m_renderer = std::make_unique<MvRenderer>(*m_window, *m_Device);
    m_texture = std::make_unique<MvTexture>(*m_Device, "textures/MvTexturesIris.png", false);
    m_UITexture = std::make_unique<MvTexture>(*m_Device, "textures/UI/HUD-MC-Vulkan.png", true);
    m_CubeMap = std::make_unique<MvCubeMap>(*m_Device);
    m_GlobalPool = MvDescriptorPool::Builder(*m_Device)
            .setMaxSets(MvSwapChain::MAX_FRAMES_IN_FLIGHT * 3)
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


    MvUIModel::Builder UIbuilder;
    UIbuilder.vertices = {
        {{-1.f, -1.f}, {0.0f, 0.0f}}, // Bottom-left (Red)
        {{1.f, -1.f}, {1.0f, 0.0f}},  // Bottom-right (Green)
        {{1.f,  1.f}, {1.0f, 1.0f}},  // Top-right (Blue)
        {{-1.f,  1.f}, {0.0f, 1.0f}}, // Top-left (Cyan)
    };

    UIbuilder.indices = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };
    m_UI = std::make_unique<MvUIModel>(*m_Device, UIbuilder);

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

    auto UISetLayout = MvDescriptorSetLayout::Builder(*m_Device)
    .addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
    .build();


    std::vector<VkDescriptorSet> globalDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::vector<VkDescriptorSet> SkyBoxDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};
    std::vector<VkDescriptorSet> UIDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};

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
    for (size_t i = 0; i < UIDescriptorSets.size(); i++) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_UITexture->GetTextureImageView();
        imageInfo.sampler = m_UITexture->GetTextureSampler();

        MvDescriptorWriter(*UISetLayout, *m_GlobalPool)
                .writeImage(0, &imageInfo)
                .build(UIDescriptorSets[i]);
    }


    MvRenderSystem renderSystem(*m_Device,
                                m_renderer->GetSwapChainRenderPass(),
                                globalSetLayout->getDescriptorSetLayout());

    MvSkyBoxRenderSystem SkyBoxRenderSystem(*m_Device,
                            m_renderer->GetSwapChainRenderPass(),
                            skySetLayout->getDescriptorSetLayout());

    MvUIRenderSystem UIRenderSystem(*m_Device,
                                m_renderer->GetSwapChainRenderPass(),
                                UISetLayout->getDescriptorSetLayout());



    auto currentTime = std::chrono::high_resolution_clock::now();

    m_World->InitCamera(*m_window, *m_renderer);

    //Update
    while (!m_window->ShouldClose()) {
        glfwPollEvents();

        if (auto CommandBuffer = m_renderer->BeginFrame()) {
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;


            //Update World and its properties
            m_World->UpdateWorld(m_window->GetWindow(), frameTime);

            int frameIndex = m_renderer->GetFrameIndex();
            MvFrameInfo frameInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
                m_World->GetCamera(),
                globalDescriptorSets[frameIndex]
            };
            MvFrameInfo SkyframeInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
               m_World->GetCamera(),
                SkyBoxDescriptorSets[frameIndex]
            };

            MvFrameInfo UIframeInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
               m_World->GetCamera(),
                UIDescriptorSets[frameIndex]
            };

            //update
            GlobalUbo ubo{};
            ubo.projectionView = m_World->GetCamera().GetProjectionMatrix() * m_World->GetCamera().GetViewMatrix();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();


            CameraVectors CameraVectors{
                glm::vec4(m_World->GetCamera().GetForward(), 0.f),
                glm::vec4(m_World->GetCamera().GetRight(), 0.f),
                glm::vec4(glm::cross(m_World->GetCamera().GetRight(), m_World->GetCamera().GetForward()), 0.f)  // Recalculate up
            };
            SkyBoxBuffer[frameIndex]->writeToBuffer(&CameraVectors);
            SkyBoxBuffer[frameIndex]->flush();


            //render
            m_renderer->BeginSwapChainRenderPass(CommandBuffer);
            SkyBoxRenderSystem.RenderSkyBox(SkyframeInfo, *m_SkyBox);
            renderSystem.RenderChunks(frameInfo, m_World->GetChunks());
            UIRenderSystem.RenderUI(UIframeInfo, *m_UI);
            m_renderer->EndSwapChainRenderPass(CommandBuffer);
            m_renderer->EndFrame();
        }

    }
    vkDeviceWaitIdle(m_Device->GetDevice());
}



