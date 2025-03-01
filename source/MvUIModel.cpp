#include "MvUIModel.hpp"

// std lib headers
#include "cassert"



MvUIModel::MvUIModel(MvDevice &Device, const MvUIModel::Builder& builder)
: m_device(Device)
{
    CreateVertexBuffer(builder.vertices);
    CreateIndexBuffer(builder.indices);
}

MvUIModel::~MvUIModel()
{

}


void MvUIModel::CreateVertexBuffer(const std::vector <UIVertex> &vertices)
{
    m_vertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_vertexCount >= 3 && "Vertex count must be at least 3");

    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    MvBuffer stagingBuffer{
        m_device,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    m_vertexBuffer = std::make_unique<MvBuffer>(
        m_device,
        vertexSize,
        m_vertexCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_device.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), bufferSize);
}

void MvUIModel::CreateIndexBuffer(const std::vector <uint32_t> &indices)
{
    m_indexCount = static_cast<uint32_t>(indices.size());
    hasIndexBuffer = m_indexCount > 0;
    if (!hasIndexBuffer)
        return;
    VkDeviceSize bufferSize = sizeof(indices[0]) * m_indexCount;
    uint32_t indexSize = sizeof(indices[0]);

    MvBuffer    stagingBuffer{
        m_device,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    m_indexBuffer = std::make_unique<MvBuffer>(
        m_device,
        indexSize,
        m_indexCount,
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_device.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), bufferSize);
}



void MvUIModel::bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_vertexBuffer->getBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    if (hasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void MvUIModel::draw(VkCommandBuffer commandBuffer)
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

std::vector<VkVertexInputBindingDescription> MvUIModel::UIVertex::getBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescptors(1);
    bindingDescptors[0].binding = 0;
    bindingDescptors[0].stride = sizeof(UIVertex);
    bindingDescptors[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescptors;
}

std::vector<VkVertexInputAttributeDescription> MvUIModel::UIVertex::getAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(UIVertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(UIVertex, UV);

    return attributeDescriptions;
}


