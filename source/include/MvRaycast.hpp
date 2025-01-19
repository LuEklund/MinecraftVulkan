//
// Created by Lucas on 2025-01-19.
//

#pragma once

#include <glm\glm.hpp>

#include "Ref.hpp"

class MvChunk;

namespace std {
    template<>
    struct hash<glm::vec3> {
        std::size_t operator()(const glm::vec3 &v) const {
            std::size_t seed = 0;
            // Combine the hashes of the vector's components
            seed ^= std::hash<float>{}(v.x) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<float>{}(v.y) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= std::hash<float>{}(v.z) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

struct MvRaycastResult {
    bool Hit;
    float Distance;
    glm::vec3 HitPos;
};

class MvRaycast {
public:
    static MvRaycastResult CastRay(std::unordered_map<glm::vec3, Ref<MvChunk>> &chunks, glm::vec3 origin, glm::vec3 direction, float maxDistance);
};


