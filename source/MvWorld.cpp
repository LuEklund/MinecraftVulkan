﻿//
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

    glm::ivec3 origin = {
        static_cast<int>(position.x)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.y)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.z)/MvChunk::CHUNK_SIZE};

    // Load chunks
    for (int x = origin.x - radius; x <= origin.x + radius; ++x) {
        for (int y = 0; y <= 3; ++y) {
            for (int z = origin.z - radius; z <= origin.z + radius; ++z) {
                // if (x >= 0 || z >= 0) {continue;}
                if (m_ChunksLoaded.find({x, y, z}) == m_ChunksLoaded.end()) {
                    auto it = m_ChunksUnLoaded.find({x, y, z});
                    if (it != m_ChunksUnLoaded.end()) {
                        it->second->SetRender(false);
                        m_ChunksLoaded[it->first] = std::move(it->second);
                        m_ChunksUnLoaded.erase(it);
                        continue ;
                    }
                    std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
                    chunk->SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
                    chunk->GenerateChunk();
                    chunk->GenerateMesh(m_Device);
                    m_ChunksLoaded[chunk->GetPosition()] = chunk;
                }

            }
        }
        //unload chunks
        for (auto it = m_ChunksLoaded.begin(); it != m_ChunksLoaded.end();) {
            if (it->first.x < origin.x - radius
                || it->first.x > origin.x + radius
                || it->first.z < origin.z - radius
                || it->first.z > origin.z + radius) {
                m_ChunksUnLoaded[it->first] = std::move(it->second);
                it = m_ChunksLoaded.erase(it);
            }
            else {
                ++it;
            }
        }
    }





}


void MvWorld::SetWorldBlockAt(glm::ivec3 position, int blockType) {
    //TODO: make this for godsaken code to a helper function
    int chunkX = position.x >= 0 ? position.x / MvChunk::CHUNK_SIZE : std::floor(position.x / (MvChunk::CHUNK_SIZE - 1));
    int chunkZ = position.z >= 0 ? position.z / MvChunk::CHUNK_SIZE : std::floor(position.z / (MvChunk::CHUNK_SIZE - 1));
    int chunkY = position.y >= 0 ? position.y / MvChunk::CHUNK_SIZE : std::floor(position.y / (MvChunk::CHUNK_SIZE - 1));


    auto it = m_ChunksLoaded.begin();

    // Check for valid chunk
    if (m_ChunksLoaded.find(glm::vec3(chunkX, chunkY, chunkZ)) == m_ChunksLoaded.end()) {
        return;
    }

    // safety LOL XD #C++
    auto &chunk = m_ChunksLoaded[glm::vec3(chunkX, chunkY, chunkZ)];
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

void MvWorld::CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad) {
    for (auto it = m_ChunksLoaded.begin(); it != m_ChunksLoaded.end(); ) {
        glm::vec3 chunkPosition = it->second->GetPosition() * glm::vec3( MvChunk::CHUNK_SIZE); // Assuming this is chunk's world position
        glm::vec3 chunkCenter = chunkPosition + MvChunk::CHUNK_SIZE * 0.5f;

        glm::vec3 toChunk = glm::normalize(chunkCenter - origin + direction * glm::vec3(MvChunk::CHUNK_SIZE));
        float dotProduct = glm::dot(toChunk, glm::normalize(direction));

        if (dotProduct >= fovRad) {
            it->second->SetRender(true);
        }
        else
            it->second->SetRender(false);
        ++it;
    }
}

