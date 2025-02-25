//
// Created by Lucas on 2025-01-19.
//

#include "MvRaycast.hpp"

#include <iostream>
#include <ostream>

#include "MvChunk.hpp"

MvRaycastResult MvRaycast::CastRay(std::unordered_map<glm::vec3, Ref<MvChunk>> &chunks, glm::vec3 origin, glm::vec3 direction, float maxDistance)
{
    MvRaycastResult result{false, -1.f, {0.f,0.f,0.f},{0.f,0.f,0.f}, {0,0,0}, nullptr};

    constexpr float stepSize = 0.01f;

    // std::cout << "origin" << origin.x << ", " << origin.y << ", " << origin.z << std::endl;

    for (float i = 0; i < maxDistance; i += stepSize) {
        glm::vec3 position = origin + direction * i;
        glm::vec3 fPos(position);
        int chunkX = position.x >= 0 ? position.x / MvChunk::CHUNK_SIZE : std::floor(fPos.x / (MvChunk::CHUNK_SIZE));
        int chunkZ = position.z >= 0 ? position.z / MvChunk::CHUNK_SIZE : std::floor(fPos.z / (MvChunk::CHUNK_SIZE));
        int chunkY = position.y >= 0 ? position.y / MvChunk::CHUNK_SIZE : std::floor(fPos.y / (MvChunk::CHUNK_SIZE));

        // Check for valid chunk
        if (chunks.find({chunkX, chunkY, chunkZ}) == chunks.end()) {
            continue;
        }

        // safety LOL XD #C++
        auto &chunk = chunks[{chunkX, chunkY, chunkZ}];
        if (!chunk) {
            continue;
        }
        // std::cout << chunkX << ", " << chunkY << ", " << chunkZ << std::endl;

        glm::ivec3 blockPos = {
            std::floor(fPos.x - chunkX * MvChunk::CHUNK_SIZE),
            std::floor(fPos.y - chunkY * MvChunk::CHUNK_SIZE),
            std::floor(fPos.z - chunkZ * MvChunk::CHUNK_SIZE),
        };
        Block blockType = chunk->GetBlock(blockPos);
        if (blockType.type == MvChunk::AIR)
            continue;

        result.Hit = true;
        result.Distance = distance(origin, position);
        result.HitPos = position;
        result.PrevPos = position - direction * stepSize * 10.f;
        result.BlockPosInChunk = blockPos;
        result.ChunkHit = chunk;
        std::cout << "position result: " << position.x << ", " << position.y << ", " << position.z << std::endl;
        std::cout << "PrevPos result: " << result.PrevPos.x << ", " << result.PrevPos.y << ", " << result.PrevPos.z << std::endl;


        // std::cout << blockPos.x << ", " << blockPos.y << ", " << blockPos.z << " block type " << blockType << std::endl;
        return result;
    }
    return result;
}
