#pragma once

#include "MvDevice.hpp"

// Library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

//std
#include <vector>

class MvModel
{
public:
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 color;

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
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    uint32_t    m_vertexCount = 0;

    bool hasIndexBuffer = false;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
    uint32_t    m_indexCount = 0;
};


