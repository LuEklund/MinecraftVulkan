//
// Created by Lucas on 2025-01-30.
//
#include "MvWorld.hpp"

#include <iostream>
#include <ostream>
#include <unordered_map>

FastNoiseLite MvWorld::m_NoiseGen;
FastNoiseLite MvWorld::DomainWarpGen;
FastNoiseLite MvWorld::m_noise_gen_peaks;
FastNoiseLite MvWorld::m_domain_warp_peaks;
FastNoiseLite MvWorld::m_detail_noise_gen;
FastNoiseLite MvWorld::m_detail_domain_warp;

std::vector<glm::vec2> MvWorld::Continentalness = {
    {-1.05f, 0.0f}, // Deep Ocean
    {-0.455f, 0.15f}, // Ocean
    {-0.19f, 0.3f}, // Coast
    {-0.11f, 0.5f}, // Near-Inland
    {0.03f, 0.6f},  // Mid-Inland
    {0.3f, 0.8f},  // Far-Inland
    {1.0f, 1.0f},  // MontaÃ±as
};

std::vector<glm::vec2> MvWorld::Erosion = {
    {10, 0},
    {20, 4},    // 20% erosion of 5 (y = 5 - 1)
    {30, 24},   // 20% erosion of 30 (y = 30 - 6)
    {60, 40},   // 20% erosion of 50 (y = 50 - 10)
    {65, 64},   // 20% erosion of 80 (y = 80 - 16)
    {68, 72},   // 20% erosion of 90 (y = 90 - 18)
    {75, 76.8}, // 20% erosion of 96 (y = 96 - 19.2)
    {100, 80},  // 20% erosion of 100 (y = 100 - 20)
    {200, 131.2} // 20% erosion of 164 (y = 164 - 32.8)
};

std::vector<glm::vec2> MvWorld::PeaksAndValleys = {
    {30, 30},   // Peak
    {60, 50},   // Peak
    {200, 164}, // Peak
    {10, 0}     // Valley
};






MvWorld::MvWorld(MvDevice &device) : m_Device(device)
{
    m_NoiseGen.SetNoiseType(FastNoiseLite::NoiseType_Value);
    m_NoiseGen.SetSeed(123456789);
    m_NoiseGen.SetFrequency(0.0004f);
    m_NoiseGen.SetFractalType(FastNoiseLite::FractalType_FBm);
    m_NoiseGen.SetFractalOctaves(4);
    m_NoiseGen.SetFractalLacunarity(2.73f);
    m_NoiseGen.SetFractalGain(0.14f);
    m_NoiseGen.SetFractalWeightedStrength(-9.51f);

    DomainWarpGen.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    DomainWarpGen.SetDomainWarpAmp(32.5f);
    DomainWarpGen.SetSeed(123456789);
    DomainWarpGen.SetFrequency(0.034f);

    m_noise_gen_peaks.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    m_noise_gen_peaks.SetSeed(123456789);
    m_noise_gen_peaks.SetFrequency(0.002f);
    m_noise_gen_peaks.SetFractalType(FastNoiseLite::FractalType_Ridged);
    m_noise_gen_peaks.SetFractalOctaves(5);
    m_noise_gen_peaks.SetFractalLacunarity(2.48f);
    m_noise_gen_peaks.SetFractalGain(0.28f);
    m_noise_gen_peaks.SetFractalWeightedStrength(0.16f);

    m_domain_warp_peaks.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    m_domain_warp_peaks.SetDomainWarpAmp(31.5f);
    m_domain_warp_peaks.SetSeed(123456789);
    m_domain_warp_peaks.SetFrequency(-0.029f);

    m_detail_noise_gen.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    m_detail_noise_gen.SetSeed(123456789);
    m_detail_noise_gen.SetFrequency(-0.023f);
    m_detail_noise_gen.SetFractalType(FastNoiseLite::FractalType_Ridged);
    m_detail_noise_gen.SetFractalOctaves(5);
    m_detail_noise_gen.SetFractalLacunarity(2.418f);
    m_detail_noise_gen.SetFractalGain(0.440f);
    m_detail_noise_gen.SetFractalWeightedStrength(-0.170f);
    m_detail_noise_gen.SetCellularReturnType(FastNoiseLite::CellularReturnType_Distance);
    m_detail_noise_gen.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    m_detail_noise_gen.SetCellularJitter(1.78f);
    m_detail_domain_warp.SetDomainWarpType(FastNoiseLite::DomainWarpType_OpenSimplex2);
    m_detail_domain_warp.SetDomainWarpAmp(43.5f);
    m_detail_domain_warp.SetSeed(123456789);
    m_detail_domain_warp.SetFrequency(-0.059f);


    // int size = 3;
    // for (int x = 0; x < size; ++x) {
    //     for (int y = 4; y < 6; ++y) {
    //         for (int z = 0; z < size; ++z) {
    //             std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
    //             chunk->SetPosition({static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)});
    //             chunk->GenerateChunk();
    //             chunk->GenerateMesh(m_Device);
    //             m_ChunksLoaded[chunk->GetPosition()] = chunk;
    //             // m_chunks.push_back(std::move(chunk));
    //         }
    //     }
    // }
}

float MvWorld::GetNoise(float x, float y) {
    float warpX = x, warpY = y;
    DomainWarpGen.DomainWarp(warpX, warpY);
    return m_NoiseGen.GetNoise(warpX, warpY);
}

float Interpolation(float a, float b, float t) {
    float f = t * t * (3 - 2 * t);
    return a + f * (b - a);
}
float SplineInterpolation(float x, const  std::vector<glm::vec2> &points) {
    if (x <= points.front().x) return points.front().y;
    if (x >= points.back().x) return points.back().y;

    for (size_t i = 0; i < points.size() - 1; ++i) {
        if (x >= points[i].x && x <= points[i + 1].x) {
            float t = (x - points[i].x) / (points[i + 1].x - points[i].x);
            return Interpolation(points[i].y, points[i + 1].y, t);
        }
    }
    return 0.0f;
}

double MvWorld::GetContinentalness(double x) {
    return SplineInterpolation(x, Continentalness);
}

double MvWorld::GetErosion(double x) {
    return SplineInterpolation(x, Erosion);
}

double MvWorld::GetPeaksAndValleys(double x) {
    return SplineInterpolation(x, PeaksAndValleys);
}


double MvWorld::GetPeaksNoise(double x, double y) {
    double warp_x = x, warp_y = y;
    m_domain_warp_peaks.DomainWarp(warp_x, warp_y);
    return m_noise_gen_peaks.GetNoise(warp_x, warp_y);
}

double MvWorld::GetDetailNoise(double x, double y) {
    double warp_x = x, warp_y = y;
    m_detail_domain_warp.DomainWarp(warp_x, warp_y);
    return m_detail_noise_gen.GetNoise(warp_x, warp_y);
}


void MvWorld::UpdateWorld(float frameTime) {
    // static float TimeElapsed = 0.0f;
    // TimeElapsed += frameTime;
    // MvChunk::GlobalLightLevel = static_cast<short>(7.5f * (std::sin(TimeElapsed) + 1));

    // for (auto it = m_ChunksLoaded.begin(); it != m_ChunksLoaded.end(); ++it) {
    //     it->second->CalculateLight();
    //     it->second->GenerateMesh(m_Device);
    // }
    // std::cout << "Light Level: " << light << std::endl;
    // std::cout << "Light Level2: " << 7.5f * (std::sin(TimeElapsed) + 1) << std::endl;
}

void MvWorld::LoadChunksAtCoordinate(glm::vec3 position, int radius) {
    // std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> chunks;

    glm::ivec3 Origin = {
        static_cast<int>(position.x)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.y)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.z)/MvChunk::CHUNK_SIZE};

    // Load chunks
    for (int x = Origin.x - radius; x <= Origin.x + radius; ++x) {
        for (int y = 0; y <= 15; ++y) {
            for (int z = Origin.z - radius; z <= Origin.z + radius; ++z) {
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
            if (it->first.x < Origin.x - radius
                || it->first.x > Origin.x + radius
                || it->first.z < Origin.z - radius
                || it->first.z > Origin.z + radius) {
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

