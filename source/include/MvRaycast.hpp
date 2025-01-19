//
// Created by Lucas on 2025-01-19.
//

#pragma once

#include "Hash.hpp"

#include "Ref.hpp"

class MvChunk;


struct MvRaycastResult {
    bool Hit;
    float Distance;
    glm::vec3 HitPos;
};

class MvRaycast {
public:
    static MvRaycastResult CastRay(std::unordered_map<glm::vec3, Ref<MvChunk>> &chunks, glm::vec3 origin, glm::vec3 direction, float maxDistance);
};


