//
// Created by Lucas on 2025-01-29.
//

#include "MvPerlinNoise.hpp"

#include "glm/vec2.hpp"
#include "glm/detail/func_geometric.inl"



MvPerlinNoise::MvPerlinNoise() {
}


glm::vec2 GetConstantVector(int v) {
    // v is the value from the permutation table
    int h = v & 3;
    if(h == 0)
        return glm::vec2(1.0, 1.0);
    else if(h == 1)
        return glm::vec2(-1.0, 1.0);
    else if(h == 2)
        return glm::vec2(-1.0, -1.0);
    else
        return glm::vec2(1.0, -1.0);
}

float Fade(float t) {
    return ((6*t - 15)*t + 10)*t*t*t;
}

float Lerp(float t, float a1, float a2) {
    return a1 + t*(a2-a1);
}

float MvPerlinNoise::Noise(float x, float y) {
    // int Y = (int)y & 255;
    // int X = (int)x & 255;
    int X = ((int)floor(x)) & 255;
    int Y = ((int)floor(y)) & 255;
    // int X = ((int)floor(x) % 256 + 256) % 256;
    // int Y = ((int)floor(y) % 256 + 256) % 256;

    float xf = x - floor(x);
    float yf = y - floor(y);

    glm::vec2 topRight = {xf-1.0, yf-1.0};
    glm::vec2 topLeft = {xf, yf-1.0};
    glm::vec2 bottomRight = {xf-1.0, yf};
    glm::vec2 bottomLeft = {xf, yf};

    // Select a value from the permutation array for each of the 4 corners
    // int valueTopRight = permutation[permutation[X+1]+Y+1];
    // int valueTopLeft = permutation[permutation[X]+Y+1];
    // int valueBottomRight = permutation[permutation[X+1]+Y];
    // int valueBottomLeft = permutation[permutation[X]+Y];
    int valueTopRight = permutation[(permutation[(X + 1) & 255] + (Y + 1)) & 255];
    int valueTopLeft = permutation[(permutation[X & 255] + (Y + 1)) & 255];
    int valueBottomRight = permutation[(permutation[(X + 1) & 255] + Y) & 255];
    int valueBottomLeft = permutation[(permutation[X & 255] + Y) & 255];


    float dotTopRight = dot(topRight, GetConstantVector(valueTopRight));
    float dotTopLeft = dot(topLeft, GetConstantVector(valueTopLeft));
    float dotBottomRight = dot(bottomRight,GetConstantVector(valueBottomRight));
    float dotBottomLeft = dot(bottomLeft,GetConstantVector(valueBottomLeft));

    float u = Fade(xf);
    float v = Fade(yf);

    return Lerp(u,
        Lerp(v, dotBottomLeft, dotTopLeft),
        Lerp(v, dotBottomRight, dotTopRight)
    );

}

