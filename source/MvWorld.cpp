﻿//
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


    // int size = 5;
    // for (int x = 0; x < size; ++x) {
    //     for (int y = 4; y < MAX_CHUNK_HEIGTH; ++y) {
    //         for (int z = 0; z < size; ++z) {
    //             std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
    //             chunk->GenerateChunk({x,y,z});
    //             m_RenderChunks[{x,y,z}] = chunk;
    //             m_LoadedChunks[{x,y,z}] = chunk;
    //             m_DirtyChunks[{x,y,z}] = chunk;
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


std::array<std::array<std::array<Block, MvChunk::CHUNK_SIZE + 2>, MvChunk::CHUNK_SIZE + 2>, MvChunk::CHUNK_SIZE + 2>
MvWorld::GetRelevantBlocks(const glm::vec<3, float> vec, const std::shared_ptr<MvChunk> &shared) {
    std::array<std::array<std::array<Block, MvChunk::CHUNK_SIZE + 2>, MvChunk::CHUNK_SIZE + 2>, MvChunk::CHUNK_SIZE + 2> Blocks;
    int ChunkX = vec.x * MvChunk::CHUNK_SIZE;
    int ChunkY = vec.y * MvChunk::CHUNK_SIZE;
    int ChunkZ = vec.z * MvChunk::CHUNK_SIZE;
    for (int x = - 1; x < MvChunk::CHUNK_SIZE + 1; ++x) {
        for (int y = - 1; y < MvChunk::CHUNK_SIZE + 1; ++y) {
            for (int z = - 1; z < MvChunk::CHUNK_SIZE + 1; ++z) {
                Blocks[x + 1][y + 1][z + 1] = GetWorldBlockAt({ChunkX + x,ChunkY + y, ChunkZ + z});
            }
        }
    }
    return Blocks;
}

// void MvWorld::LightPropagate(int x, int y, int z, int lightLevel) {
//     Block Block = GetWorldBlockAt({x,y,z});
//     if (Block.type == BlockType::AIR && Block.light + 2 <= lightLevel)
//     {
//         Block.light = lightLevel - 1;
//         SetWorldBlockAt({x,y,z}, Block);
//         sunlightBfsQueue.emplace(LightNode{{x,y,z}});
//     }
// }

void MvWorld::LightPropagate(int x, int y, int z, int lightLevel) {
    if (lightLevel <= 1) return;
    auto Chunk = GetChunkBlockPos(glm::vec3(x,y,z));
    if (!Chunk) return;
    if (Chunk->TryPropagateLight(abs(x % 16), abs(y % 16), abs(z % 16), lightLevel))
        sunlightBfsQueue.emplace(LightNode{{x,y,z}});
}

void MvWorld::UpdateLights() {
    while (!sunlightBfsQueue.empty()) {
        LightNode &node = sunlightBfsQueue.front();
        glm::vec3 BlockPos = node.BlockPos;
        sunlightBfsQueue.pop();
        int x = BlockPos.x;
        int y = BlockPos.y;
        int z = BlockPos.z;
        int lightLevel = GetWorldBlockAt({x,y,z}).light;
        LightPropagate(x-1, y, z, lightLevel);
        LightPropagate(x+1, y, z , lightLevel);
        LightPropagate(x, y, z-1, lightLevel);
        LightPropagate(x, y, z+1, lightLevel);
        LightPropagate(x, y-1, z, lightLevel);
        LightPropagate(x, y+1, z, lightLevel);
    }
}

bool MvWorld::HasDirectSkyLight(glm::ivec3 BlockPos) {
    int MAX_BLOCK_HEIGHT = MAX_CHUNK_HEIGTH * MvChunk::CHUNK_SIZE;
    while (BlockPos.y < MAX_BLOCK_HEIGHT) {
        if (GetWorldBlockAt(BlockPos).type != BlockType::AIR)
            return false;
        BlockPos.y++;
    }
    return true;
}


void MvWorld::ResetLight(std::unordered_map<glm::vec<3, float>, std::shared_ptr<MvChunk>>::iterator it) {

    // std::shared_ptr<MvChunk> AboveChunk = GetChunkChunkPos(glm::vec3(it->first.x, it->first.y + 1, it->first.z));
    glm::ivec3 FirstBlockPos = {it->first.x * MvChunk::CHUNK_SIZE, it->first.y * MvChunk::CHUNK_SIZE, it->first.z * MvChunk::CHUNK_SIZE};
    for (int x = 0; x < MvChunk::CHUNK_SIZE; x++) {
        for (int z = 0; z < MvChunk::CHUNK_SIZE; z++) {
            int y = MvChunk::CHUNK_SIZE - 1;
            // Block BlockAbove = AboveChunk ? AboveChunk->GetBlock({x,0,z}) : Block(BlockType::AIR, 15);
            if (HasDirectSkyLight(
                    {FirstBlockPos.x + x,
                               FirstBlockPos.y + y,
                               FirstBlockPos.z + z})) {
                for (y; y >= 0; y--) {
                    if (it->second->GetBlock({x,y,z}).type == BlockType::AIR) {
                        it->second->SetLight({x,y,z}, GlobalLightLevel);
                        sunlightBfsQueue.emplace(LightNode{{
                                    FirstBlockPos.x + x,
                                     FirstBlockPos.y + y,
                                     FirstBlockPos.z + z}});
                    }
                    else
                        break;
                }
            }
            for (y; y >= 0; y--) {
                it->second->SetLight({x,y,z}, 0);
            }
        }
    }
}

void MvWorld::UpdateWorld(float frameTime) {
    for (auto it = m_DirtyChunks.begin(); it != m_DirtyChunks.end();) {
        ResetLight(it);
        it++;
    }
    UpdateLights();
    for (auto it = m_DirtyChunks.begin(); it != m_DirtyChunks.end();) {
        MvModel::Builder modelBuilder = it->second->GenerateMesh(GetRelevantBlocks(it->first, it->second), it->first);
        if (it->second->HasMesh()) {
            it->second->SetModel(std::make_unique<MvModel>(m_Device, modelBuilder));
        }
        it = m_DirtyChunks.erase(it);
    }
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

// 0 = Left -x, 1 = Right +x, 2 = Bottom -y, 3 = Top +y, 4 = Back -z, 5 = Forward +z
std::array<std::unordered_map<glm::vec<3, float>, std::shared_ptr<MvChunk>>::iterator, 6> MvWorld::GetNeighborITChunks(glm::ivec3 ChunkPos) {
    std::array<std::unordered_map<glm::vec<3, float>, std::shared_ptr<MvChunk>>::iterator, 6> Neighbors;
    std::array<glm::ivec3, 6> RelativePos {
            {{-1, 0, 0},{1, 0, 0},{0, -1, 0}, {0, 1, 0},{0, 0, -1}, {0, 0, 1}
            }};

    for (int i = 0; i < 6; ++i) {
        glm::ivec3 NeighborPos = ChunkPos + RelativePos[i];
        auto NeighborChunk = m_LoadedChunks.find(NeighborPos);
        Neighbors[i] = NeighborChunk;
    }
    return Neighbors;
}

// 0 = Left -x, 1 = Right +x, 2 = Bottom -y, 3 = Top +y, 4 = Back -z, 5 = Forward +z
std::array<std::shared_ptr<MvChunk>, 6> MvWorld::GetNeighborChunks(glm::ivec3 vec) {
    std::array<std::shared_ptr<MvChunk>, 6> Neighbors;
    Neighbors[0] = m_LoadedChunks.find({vec.x - 1, vec.y, vec.z}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x - 1, vec.y, vec.z}) : nullptr;
    Neighbors[1] = m_LoadedChunks.find({vec.x + 1, vec.y, vec.z}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x + 1, vec.y, vec.z}) : nullptr;
    Neighbors[2] = m_LoadedChunks.find({vec.x, vec.y - 1, vec.z}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x, vec.y - 1, vec.z}) : nullptr;
    Neighbors[3] = m_LoadedChunks.find({vec.x, vec.y + 1, vec.z}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x, vec.y + 1, vec.z}) : nullptr;
    Neighbors[4] = m_LoadedChunks.find({vec.x, vec.y, vec.z - 1}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x, vec.y, vec.z - 1}) : nullptr;
    Neighbors[5] = m_LoadedChunks.find({vec.x, vec.y, vec.z + 1}) != m_LoadedChunks.end() ? m_LoadedChunks.at({vec.x, vec.y, vec.z + 1}) : nullptr;
    return Neighbors;
}

void MvWorld::LoadChunksAtCoordinate(glm::vec3 position, int radius) {
    // std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> chunks;

    glm::ivec3 Origin = {
        static_cast<int>(position.x)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.y)/MvChunk::CHUNK_SIZE,
        static_cast<int>(position.z)/MvChunk::CHUNK_SIZE};

    // Load chunks
    for (int x = Origin.x - radius; x <= Origin.x + radius; ++x) {
        for (int y = 0; y < MAX_CHUNK_HEIGTH; ++y) {
            for (int z = Origin.z - radius; z <= Origin.z + radius; ++z) {
                // if (x >= 0 || z >= 0) {continue;}
                if (m_RenderChunks.find({x, y, z}) == m_RenderChunks.end()) {
                    auto it = m_LoadedChunks.find({x, y, z});
                    if (it != m_LoadedChunks.end()) {
                        it->second->SetRender(false);
                        m_RenderChunks[it->first] = it->second;
                        continue ;
                    }
                    std::shared_ptr<MvChunk> chunk = std::make_shared<MvChunk>();
                    chunk->GenerateChunk({x,y,z});
                    m_DirtyChunks[{x,y,z}] = chunk;
                    for (auto Neighbor : GetNeighborITChunks({x,y,z})) {
                        if (Neighbor != m_LoadedChunks.end()) {
                            m_DirtyChunks[Neighbor->first] = Neighbor->second;;
                        }
                    }
                    m_RenderChunks[{x,y,z}] = chunk;
                    m_LoadedChunks[{x,y,z}] = chunk;
                }

            }
        }
        //unload chunks
        for (auto it = m_RenderChunks.begin(); it != m_RenderChunks.end();) {
            if (it->first.x < Origin.x - radius
                || it->first.x > Origin.x + radius
                || it->first.z < Origin.z - radius
                || it->first.z > Origin.z + radius) {
                if (m_LoadedChunks.find(it->first) == m_LoadedChunks.end())
                    m_LoadedChunks[it->first] = std::move(it->second);
                it = m_RenderChunks.erase(it);
            }
            else {
                ++it;
            }
        }
    }





}

std::shared_ptr<MvChunk> MvWorld::GetChunkChunkPos(glm::vec3 position) {
    auto Chunk = m_LoadedChunks.find(glm::vec3(position.x ,position.y, position.z));
    if (Chunk == m_LoadedChunks.end()) {
        return nullptr;
    }
    return Chunk->second;
}


 std::shared_ptr<MvChunk> MvWorld::GetChunkBlockPos(glm::vec3 position) {
    int chunkX = std::floor(position.x / (MvChunk::CHUNK_SIZE));
    int chunkZ = std::floor(position.z / (MvChunk::CHUNK_SIZE));
    int chunkY = std::floor(position.y / (MvChunk::CHUNK_SIZE));
    auto Chunk = m_LoadedChunks.find(glm::vec3(chunkX, chunkY, chunkZ));
    if (Chunk == m_LoadedChunks.end()) {
        return nullptr;
    }
    return Chunk->second;
}

Block MvWorld::GetWorldBlockAt(glm::vec3 position) {
    int chunkX = std::floor(position.x / (MvChunk::CHUNK_SIZE));
    int chunkY = std::floor(position.y / (MvChunk::CHUNK_SIZE));
    int chunkZ = std::floor(position.z / (MvChunk::CHUNK_SIZE));

    auto Chunk = m_LoadedChunks.find(glm::vec3(chunkX, chunkY, chunkZ));
    if (Chunk == m_LoadedChunks.end()) {
        return Block(BlockType::INVALID, 0);
    }

    glm::ivec3 blockPos = {
        std::floor(position.x - (chunkX * MvChunk::CHUNK_SIZE)),
        std::floor(position.y - (chunkY * MvChunk::CHUNK_SIZE)),
        std::floor(position.z - (chunkZ * MvChunk::CHUNK_SIZE)),
    };

    return Chunk->second->GetBlock(blockPos);
}

void MvWorld::SetWorldBlockAt(glm::vec3 position, BlockType blockType) {
    int chunkX = std::floor(position.x / (MvChunk::CHUNK_SIZE));
    int chunkZ = std::floor(position.z / (MvChunk::CHUNK_SIZE));
    int chunkY = std::floor(position.y / (MvChunk::CHUNK_SIZE));

    // Check for valid chunk
    if (m_RenderChunks.find(glm::vec3(chunkX, chunkY, chunkZ)) == m_RenderChunks.end()) {
        return;
    }

    // safety LOL XD #C++
    auto &chunk = m_RenderChunks[glm::vec3(chunkX, chunkY, chunkZ)];
    if (!chunk) {
        return;
    }

    glm::ivec3 blockPos = {
        std::floor(position.x - (chunkX * MvChunk::CHUNK_SIZE)),
        std::floor(position.y - (chunkY * MvChunk::CHUNK_SIZE)),
        std::floor(position.z - (chunkZ * MvChunk::CHUNK_SIZE)),
    };
    std::array<std::shared_ptr<MvChunk>, 6> Neighbor = GetNeighborChunks({chunkX, chunkY, chunkZ});
    if (Neighbor[0]) m_DirtyChunks[{chunkX - 1, chunkY, chunkZ}] = Neighbor[0];
    if (Neighbor[1]) m_DirtyChunks[{chunkX + 1, chunkY, chunkZ}] = Neighbor[1];
    if (Neighbor[2]) m_DirtyChunks[{chunkX, chunkY - 1, chunkZ}] = Neighbor[2];
    if (Neighbor[3]) m_DirtyChunks[{chunkX, chunkY + 1, chunkZ}] = Neighbor[3];
    if (Neighbor[4]) {
        // for (int x = 0; x < 16; ++x) {
        //     for (int y = 0; y < 16; ++y) {
        //         for (int z = 0; z < 16; ++z) {
        //             Neighbor[4]->SetBlockAt({x,y,z}, BlockType::GRASS_2);
        //         }
        //     }
        // }
        m_DirtyChunks[{chunkX, chunkY, chunkZ - 1}] = Neighbor[4];
    }
    if (Neighbor[5]) m_DirtyChunks[{chunkX, chunkY, chunkZ + 1}] = Neighbor[5];


    m_DirtyChunks[{chunkX, chunkY, chunkZ}] = chunk;
    chunk->SetBlockAt(blockPos, blockType);
}

void MvWorld::CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad) {
    for (auto it = m_RenderChunks.begin(); it != m_RenderChunks.end(); ) {
        glm::vec3 chunkPosition = it->first * glm::vec3( MvChunk::CHUNK_SIZE); // Assuming this is chunk's world position
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

