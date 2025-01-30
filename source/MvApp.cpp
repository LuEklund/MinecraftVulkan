#include "MvApp.hpp"

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



struct GlobalUbo {
    glm::mat4 projectionView{1.f};
    glm::vec3 lightDirection = glm::normalize(glm::vec3{1.f, 0.f, 0.f});
};


MvApp::MvApp() {
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
    glfwSetWindowUserPointer(m_window->GetWindow(), this);
}

MvApp::~MvApp() {
}

void MvApp::Run() {
    //init stuff
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


    auto globalSetLayout = MvDescriptorSetLayout::Builder(*m_Device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

    std::vector<VkDescriptorSet> globalDescriptorSets{MvSwapChain::MAX_FRAMES_IN_FLIGHT};

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
    MvRenderSystem renderSystem(*m_Device,
                                m_renderer->GetSwapChainRenderPass(),
                                globalSetLayout->getDescriptorSetLayout());
    m_Camera = std::make_unique<MvCamera>();
    m_Camera->SetUpListeners(m_window->GetWindow());
    m_Camera->SetViewTarget(glm::vec3{0.f, -2.f, 2.f}, glm::vec3{0.f, -1.f, -1.f});
    auto currentTime = std::chrono::high_resolution_clock::now();


    //Update
    while (!m_window->ShouldClose()) {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        m_Camera->MoveInPlaneXZ(m_window->GetWindow(), frameTime);
        m_Camera->SetViewYXZ();

        float aspect = m_renderer->GetAspectRatio();
        m_Camera->SetPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 200.f);

        if (auto CommandBuffer = m_renderer->BeginFrame()) {
            int frameIndex = m_renderer->GetFrameIndex();
            MvFrameInfo frameInfo{
                frameIndex,
                frameTime,
                CommandBuffer,
                *m_Camera.get(),
                globalDescriptorSets[frameIndex]
            };

            //update
            GlobalUbo ubo{};
            ubo.projectionView = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
            uboBuffers[frameIndex]->writeToBuffer(&ubo);
            uboBuffers[frameIndex]->flush();

            //render
            m_renderer->BeginSwapChainRenderPass(CommandBuffer);
            renderSystem.RenderChunks(frameInfo, m_chunks);
            m_renderer->EndSwapChainRenderPass(CommandBuffer);
            m_renderer->EndFrame();
        }
    }
    vkDeviceWaitIdle(m_Device->GetDevice());
}



//Make chunks
void MvApp::LoadBlocks() {
    // m_cubeModel = CreateCubeModel(*m_Device, {0.f, 0.f, 0.f});
    int size = 6;
    for (int x = 0; x < size; ++x) {
        for (int y = 0; y < 4; ++y) {
            for (int z = 0; z < size; ++z) {
                Ref<MvChunk> chunk = CreateRef<MvChunk>();
                chunk->SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                chunk->GenerateChunk();
                chunk->GenerateMesh(*m_Device);
                m_chunks[chunk->GetPosition()] = chunk;
                // m_chunks.push_back(std::move(chunk));
            }
        }
    }
}

void MvApp::SetWorldBlockAt(glm::ivec3 position, int blockType) {
    //TODO: make this for godsaken code to a helper function
    int chunkX = position.x >= 0 ? position.x / MvChunk::CHUNK_SIZE : std::floor(position.x / (MvChunk::CHUNK_SIZE - 1));
    int chunkZ = position.z >= 0 ? position.z / MvChunk::CHUNK_SIZE : std::floor(position.z / (MvChunk::CHUNK_SIZE - 1));
    int chunkY = position.y >= 0 ? position.y / MvChunk::CHUNK_SIZE : std::floor(position.y / (MvChunk::CHUNK_SIZE - 1));

    // Check for valid chunk
    if (m_chunks.find({chunkX, chunkY, chunkZ}) == m_chunks.end()) {
        return;
    }

    // safety LOL XD #C++
    auto &chunk = m_chunks[{chunkX, chunkY, chunkZ}];
    if (!chunk) {
        return;
    }
    // std::cout << chunkX << ", " << chunkY << ", " << chunkZ << std::endl;

    glm::ivec3 blockPos = {
        std::floor(position.x - chunkX * MvChunk::CHUNK_SIZE),
        std::floor(position.y - chunkY * MvChunk::CHUNK_SIZE),
        std::floor(position.z - chunkZ * MvChunk::CHUNK_SIZE),
    };
    chunk->SetBlockAt(blockPos, blockType);
    chunk->GenerateMesh(*m_Device);
    // int blockType = chunk->GetBlock(blockPos.x, blockPos.y, blockPos.z);
}
