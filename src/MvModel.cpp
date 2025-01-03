#include "MvModel.hpp"

// std lib headers
#include "cassert"
#include "cstring"

MvModel::MvModel(MvDevice &Device, const std::vector<Vertex> &vertices)
: m_device(Device)
{
    createVertexBuffer(vertices);
}

MvModel::~MvModel()
{
    
    vkDestroyBuffer(m_device.GetDevice(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), m_vertexBufferMemory, nullptr);
}


void MvModel::createVertexBuffer(const std::vector <Vertex> &vertices)
{
    vetrtexCount = static_cast<uint32_t>(vertices.size());
    assert(vetrtexCount >= 3 && "Vertex count must be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vetrtexCount;

    m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
       m_vertexBuffer,
       m_vertexBufferMemory);

    void *data;
    vkMapMemory(m_device.GetDevice(), m_vertexBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device.GetDevice(), m_vertexBufferMemory);
}

void MvModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void MvModel::draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, vetrtexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> MvModel::Vertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescptors(1);
    bindingDescptors[0].binding = 0;
    bindingDescptors[0].stride = sizeof(Vertex);
    bindingDescptors[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescptors;
}

std::vector<VkVertexInputAttributeDescription> MvModel::Vertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}


