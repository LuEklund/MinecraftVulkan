#include "MvModel.hpp"

// std lib headers
#include "cassert"
#include "cstring"

MvModel::MvModel(MvDevice &Device, const MvModel::Builder& builder)
: m_device(Device)
{
    CreateVertexBuffer(builder.vertices);
    CreateIndexBuffer(builder.indices);
}

MvModel::~MvModel()
{
    
    vkDestroyBuffer(m_device.GetDevice(), m_vertexBuffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), m_vertexBufferMemory, nullptr);
    if (hasIndexBuffer)
    {
        vkDestroyBuffer(m_device.GetDevice(), m_indexBuffer, nullptr);
        vkFreeMemory(m_device.GetDevice(), m_indexBufferMemory, nullptr);
    }
}


void MvModel::CreateVertexBuffer(const std::vector <Vertex> &vertices)
{
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
       stagingBuffer,
       stagingBufferMemory);

    void *data;
    vkMapMemory(m_device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device.GetDevice(), stagingBufferMemory);

        m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
       m_vertexBuffer,
       m_vertexBufferMemory);

    m_device.copyBuffer(stagingBuffer, m_vertexBuffer, bufferSize);

    vkDestroyBuffer(m_device.GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), stagingBufferMemory, nullptr);
}

void MvModel::CreateIndexBuffer(const std::vector <uint32_t> &indices)
{
    m_indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = m_indexCount > 0;
    if (!hasIndexBuffer)
        return;
    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
       stagingBuffer,
       stagingBufferMemory);

    void *data;
    vkMapMemory(m_device.GetDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_device.GetDevice(), stagingBufferMemory);

        m_device.createBuffer(
        bufferSize,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
       m_indexBuffer,
       m_indexBufferMemory);

    m_device.copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(m_device.GetDevice(), stagingBuffer, nullptr);
    vkFreeMemory(m_device.GetDevice(), stagingBufferMemory, nullptr);
}

void MvModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    }
}

void MvModel::draw(VkCommandBuffer commandBuffer)
{
    if (hasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, m_indexCount, 1, 0, 0, 0);
        return;
    }
    else
    {
        vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
    }
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


