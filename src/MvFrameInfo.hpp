#pragma once

#include "MvCamera.hpp"

//lib
#include <vulkan/vulkan.h>

struct MvFrameInfo
{
    int frameIndex = 0;
    float deltaTime = 0.f;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    MvCamera &camera;
};
