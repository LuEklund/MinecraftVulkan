#pragma once

#include "MvBuffer.hpp"

// Library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



//std
#include <vector>
#include <memory>


class MvUIModel
{
public:
    struct UIVertex
    {
        alignas(16) glm::vec2 position{};
        alignas(16) glm::vec4 color{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();


    };

    struct Builder
    {
        std::vector<UIVertex> vertices{};
        std::vector<uint32_t> indices{};
        
    };
    
    MvUIModel(MvDevice& Device, const MvUIModel::Builder& builder);
    ~MvUIModel();

    MvUIModel(const MvUIModel&) = delete;
    MvUIModel operator=(const MvUIModel&) = delete;


    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);


private:
    void CreateVertexBuffer(const std::vector<UIVertex>& vertices);
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


