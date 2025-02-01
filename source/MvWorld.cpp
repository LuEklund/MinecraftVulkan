//
// Created by Lucas on 2025-01-30.
//
#include "MvWorld.hpp"
#include <unordered_map>


MvWorld::MvWorld(MvDevice &device) : m_Device(device)
{
    // int size = 6;
    // for (int x = 0; x < size; ++x) {
    //     for (int y = 0; y < 4; ++y) {
    //         for (int z = 0; z < size; ++z) {
    //             std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
    //             chunk->SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
    //             chunk->GenerateChunk();
    //             chunk->GenerateMesh(m_Device);
    //             m_chunks[chunk->GetPosition()] = chunk;
    //             // m_chunks.push_back(std::move(chunk));
    //         }
    //     }
    // }
}

void MvWorld::LoadChunksAtCoordinate(glm::vec3 position, int radius) {
    // std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> chunks;

    glm::ivec3 Origin = {
        static_cast<int>(position.x)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.y)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.z)/MvChunk::CHUNK_SIZE};

    for (int x = Origin.x - radius; x <= Origin.x + radius; ++x) {
        for (int y = 0; y <= 3; ++y) {
            for (int z = Origin.z - radius; z <= Origin.z + radius; ++z) {
                // if (x >= 0 || z >= 0) {continue;}
                if (m_chunks.find({x, y, z}) == m_chunks.end()) {
                    std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
                    chunk->SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                    chunk->GenerateChunk();
                    chunk->GenerateMesh(m_Device);
                    m_chunks[chunk->GetPosition()] = chunk;
                }
            }
        }
    }


}


void MvWorld::SetWorldBlockAt(glm::ivec3 position, int blockType) {
    //TODO: make this for godsaken code to a helper function
    int chunkX = position.x >= 0 ? position.x / MvChunk::CHUNK_SIZE : std::floor(position.x / (MvChunk::CHUNK_SIZE - 1));
    int chunkZ = position.z >= 0 ? position.z / MvChunk::CHUNK_SIZE : std::floor(position.z / (MvChunk::CHUNK_SIZE - 1));
    int chunkY = position.y >= 0 ? position.y / MvChunk::CHUNK_SIZE : std::floor(position.y / (MvChunk::CHUNK_SIZE - 1));


    auto it = m_chunks.begin();

    // Check for valid chunk
    if (m_chunks.find(glm::vec3(chunkX, chunkY, chunkZ)) == m_chunks.end()) {
        return;
    }

    // safety LOL XD #C++
    auto &chunk = m_chunks[glm::vec3(chunkX, chunkY, chunkZ)];
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
    chunk->GenerateMesh(m_Device);
    // int blockType = chunk->GetBlock(blockPos.x, blockPos.y, blockPos.z);
}

