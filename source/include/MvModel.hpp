#pragma once

#include "MvBuffer.hpp"

// Library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



//std
#include <vector>
#include <memory>


class MvModel
{
public:
    struct Vertex
    {
        alignas(16) glm::vec3 position{};
        alignas(8) glm::vec2 uv{};
        alignas(8) float light{0.f};
        alignas(4) float ambientOcclusion{1.f};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();


    };

    struct Builder 
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};
        
    };
    
    MvModel(MvDevice& Device, const MvModel::Builder& builder);
    ~MvModel();

    MvModel(const MvModel&) = delete;
    MvModel operator=(const MvModel&) = delete;


    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);


private:
    void CreateVertexBuffer(const std::vector<Vertex>& vertices);
    void CreateIndexBuffer(const std::vector<uint32_t>& indices);
    
    MvDevice& m_device;

    // Vertex Buffer
    std::unique_ptr<MvBuffer> m_vertexBuffer;
    uint32_t    m_vertexCount = 0;

    // Index Buffer
    bool hasIndexBuffer = false;
    std::unique_ptr<MvBuffer> m_indexBuffer;
    uint32_t    m_indexCount = 0;
};


