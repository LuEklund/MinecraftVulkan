//
// Created by Lucas on 2025-01-19.
//

#include "MvRaycast.hpp"

#include <iostream>
#include <ostream>

#include "MvChunk.hpp"

MvRaycastResult MvRaycast::CastRay(std::unordered_map<glm::vec3, Ref<MvChunk>> &chunks, glm::vec3 origin, glm::vec3 direction, float maxDistance)
{
    MvRaycastResult result{false, -1.f, {0.f,0.f,0.f}};

    constexpr float stepSize = 0.01f;

    for (float i = 0; i < maxDistance; i += stepSize) {
        glm::vec3 position = origin + direction * i;
        int chunkX = position.x >= 0 ? position.x / MvChunk::CHUNK_SIZE : position.x / MvChunk::CHUNK_SIZE - 1;
        int chunkZ = position.z >= 0 ? position.x / MvChunk::CHUNK_SIZE : position.x / MvChunk::CHUNK_SIZE - 1;
        int chunkY = 0;
        // int chunky = position.y >= 0 ? position.x / MvChunk::CHUNK_SIZE : position.x / MvChunk::CHUNK_SIZE - 1;


        auto &chunk = chunks[{chunkX, chunkY, chunkZ}];
        if (!chunk) {
            continue;
        }
        std::cout << position.x << " " << position.z << std::endl;




    }

    return result;
}
