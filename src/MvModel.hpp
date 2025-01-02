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
        glm::vec2 position;

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
    };
    
    MvModel(MvDevice& Device, const std::vector<Vertex>& vertices);
    ~MvModel();

    MvModel(const MvModel&) = delete;
    void operator=(const MvModel&) = delete;

    void bind(VkCommandBuffer commandBuffer);
    void draw(VkCommandBuffer commandBuffer);

    

private:

    void createVertexBuffer(const std::vector<Vertex>& vertices);

    MvDevice& m_device;
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    uint32_t    vetrtexCount = 0;
};


