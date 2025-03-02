//
// Created by Lucas on 2025-01-30.
//
#include "MvWorld.hpp"

#include <filesystem>
#include <iostream>
#include <ostream>
#include <unordered_map>

#include "MvCamera.hpp"
#include "MvRenderer.hpp"

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

    //==================================================================
    //    ===================[PERLIN NOISE]=======================
    //==================================================================


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

void MvWorld::InitCamera(MvWindow &Window, MvRenderer &renderer) {
    m_Camera = std::make_unique<MvCamera>();
    m_Camera->SetUpListeners(Window.GetWindow());
    m_Camera->SetViewTarget(glm::vec3{0.f, -2.f, 2.f}, glm::vec3{0.f, -1.f, -1.f});
    m_Camera->SetPosition({ 0.f, 75.f, 0.f });
    m_Camera->SetAspectRatio(renderer.GetAspectRatio());
}


// Neighbor[0]: Left (-x),
// Neighbor[1]: Right (+x)
// Neighbor[2]: Bottom (-y)
// Neighbor[3]: Top (+y)
// Neighbor[4]: Back (-z)
// Neighbor[5]: Forward (+z)
std::array<glm::ivec3, 6> GetRelativeCords() {
    return std::array<glm::ivec3, 6> ({
        {-1, 0, 0},
        {1, 0, 0},
        {0, -1, 0},
        {0, 1, 0},
        {0, 0, -1},
        {0, 0, 1}
    });
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


int posMod(int value, int modulus) {
    return (value % modulus + modulus) % modulus;
}


void MvWorld::LightPropagate(int x, int y, int z, int lightLevel) {
    if (lightLevel <= 1) return;
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos({x,y,z});
    auto Chunk = GetChunkChunkPos(glm::vec3(ChunkPos));
    if (!Chunk) return;
    if (y % 16 == 0) {
        std::array<glm::ivec3, 6> RelativeCords = GetRelativeCords();
        for (int i = 0; i < 6; ++i) {
            glm::ivec3 RelativePos = ChunkPos + RelativeCords[i];
            auto NeighborChunk = m_LoadedChunks.find(RelativePos);
            if (NeighborChunk != m_LoadedChunks.end()) {
                m_DirtyMeshChunks[RelativePos] = NeighborChunk->second;
            }
        }
    }
    // Use a proper positive mod function
    int localX = posMod(x, 16);
    int localY = posMod(y, 16);
    int localZ = posMod(z, 16);

    if (Chunk->TryPropagateLight(localX, localY, localZ, lightLevel)) {
        sunlightBfsQueue.emplace(LightNode{{x, y, z}});
    }
}

void MvWorld::LightBeGonePropagate(int x, int y, int z, int lightLevel) {
    if (lightLevel <= 1) return;
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos({x,y,z});
    auto Chunk = GetChunkChunkPos(glm::vec3(ChunkPos));
    if (!Chunk) return;
    // if (y % 16 == 0) {
        std::array<glm::ivec3, 6> RelativeCords = GetRelativeCords();
        for (int i = 0; i < 6; ++i) {
            glm::ivec3 RelativePos = ChunkPos + RelativeCords[i];
            auto NeighborChunk = m_LoadedChunks.find(RelativePos);
            if (NeighborChunk != m_LoadedChunks.end()) {
                m_DirtyMeshChunks[RelativePos] = NeighborChunk->second;
            }
        }
    // }
    // Use a proper positive mod function
    int localX = posMod(x, 16);
    int localY = posMod(y, 16);
    int localZ = posMod(z, 16);

    if (Chunk->TryBeGonePropagateLight(localX, localY, localZ, lightLevel)) {
        reverseSunlightBfsQueue.emplace(LightNode{{x, y, z}});
    }
    else {
        sunlightBfsQueue.emplace(LightNode{{x, y, z}});
    }
}

void MvWorld::UpdateLights() {
    //Remove Lights
    while (!reverseSunlightBfsQueue.empty()) {
        LightNode &node = reverseSunlightBfsQueue.front();
        glm::vec3 BlockPos = node.BlockPos;
        reverseSunlightBfsQueue.pop();
        int x = BlockPos.x;
        int y = BlockPos.y;
        int z = BlockPos.z;
        int lightLevel = GetWorldBlockAt({x,y,z}).light;
        LightBeGonePropagate(x-1, y, z, lightLevel);
        LightBeGonePropagate(x+1, y, z , lightLevel);
        LightBeGonePropagate(x, y, z-1, lightLevel);
        LightBeGonePropagate(x, y, z+1, lightLevel);
        LightBeGonePropagate(x, y-1, z, lightLevel);
        LightBeGonePropagate(x, y+1, z, lightLevel);
        SetWorldLightBlockAt({x,y,z}, 0);
    }

    //Fill Lights
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

// glm::vec3 MvWorld::GetHigestPos(glm::vec3 pos) {
//     int MAX_BLOCK_HEIGHT = MAX_CHUNK_HEIGTH * MvChunk::CHUNK_SIZE;
//     while (pos.y < MAX_BLOCK_HEIGHT) {
//         Block block = GetWorldBlockAt(pos);
//         if (block.type != BlockType::AIR && block.type != BlockType::INVALID)
//             return {pos.x, std::floor(pos.y), pos.z};
//         pos.y++;
//     }
//     return pos;
// }

void MvWorld::UpdateWorld(GLFWwindow *window, float frameTime) {

    LoadChunksAtCoordinate(m_Camera->GetPosition(), 4);
    UpdateLights();
    for (auto it = m_DirtyMeshChunks.begin(); it != m_DirtyMeshChunks.end();) {
        MvModel::Builder modelBuilder = it->second->GenerateMesh(GetRelevantBlocks(it->first, it->second), it->first);
        if (it->second->HasMesh()) {
            it->second->SetModel(std::make_unique<MvModel>(m_Device, modelBuilder));
        }
        it = m_DirtyMeshChunks.erase(it);
    }
    CalculateRenderChunks(m_Camera->GetPosition(), m_Camera->GetForward(), 3, m_Camera->GetFovRadians() * 0.8f);

    //Player update
    m_Camera->Update(window, frameTime);
    glm::vec3 curPos = m_Camera->GetPosition();
    glm::vec3 newPos = curPos + m_Camera->moveDirection;
    Block blockBelow = GetWorldBlockAt({curPos.x, std::floor(newPos.y) - 1, curPos.z});
    if (GetWorldBlockAt({newPos.x, newPos.y, newPos.z}).type == BlockType::AIR && blockBelow.type == BlockType::AIR) {
        m_Camera->SetPosition(newPos);
    } else {
        glm::vec3 axisRestrictedPosition = curPos;

        if (GetWorldBlockAt({newPos.x, curPos.y, curPos.z}).type == BlockType::AIR
            && GetWorldBlockAt({newPos.x, curPos.y - 1, curPos.z}).type == BlockType::AIR) {
            axisRestrictedPosition.x = newPos.x;
        }
        if (GetWorldBlockAt({curPos.x, newPos.y, curPos.z}).type == BlockType::AIR
            && GetWorldBlockAt({curPos.x, newPos.y - 1, curPos.z}).type == BlockType::AIR) {
            axisRestrictedPosition.y = newPos.y;
        }
        if (GetWorldBlockAt({curPos.x, curPos.y, newPos.z}).type == BlockType::AIR
            && GetWorldBlockAt({curPos.x, curPos.y - 1, newPos.z}).type == BlockType::AIR) {
            axisRestrictedPosition.z = newPos.z;
        }

        //Special one block height
        if (blockBelow.type != BlockType::AIR) {
            axisRestrictedPosition.y = std::floor(newPos.y) + 1;
        }
        m_Camera->SetPosition({axisRestrictedPosition.x, axisRestrictedPosition.y, axisRestrictedPosition.z});
    }
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
                    chunk->GenerateChunk({x,y,z}, GlobalLightLevel);
                    m_DirtyMeshChunks[{x,y,z}] = chunk;
                    for (auto Neighbor : GetNeighborITChunks({x,y,z})) {
                        if (Neighbor != m_LoadedChunks.end()) {
                            m_DirtyMeshChunks[Neighbor->first] = Neighbor->second;
                            // std::cout << "Neigbour" << Neighbor->first.x << " " << Neighbor->first.y << " " << Neighbor->first.z << std::endl;
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


glm::ivec3 MvWorld::ConvertBlockPosToChunkPos(glm::vec3 blockPos) {
    return {
        std::floor(blockPos.x / (MvChunk::CHUNK_SIZE)),
        std::floor(blockPos.y / (MvChunk::CHUNK_SIZE)),
        std::floor(blockPos.z / (MvChunk::CHUNK_SIZE))
    };
}

std::shared_ptr<MvChunk> MvWorld::GetChunkChunkPos(glm::vec3 position) {
    auto Chunk = m_LoadedChunks.find(glm::vec3(position.x ,position.y, position.z));
    if (Chunk == m_LoadedChunks.end()) {
        return nullptr;
    }
    return Chunk->second;
}


 std::shared_ptr<MvChunk> MvWorld::GetChunkBlockPos(glm::vec3 position) {
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos(position);
    auto Chunk = m_LoadedChunks.find(glm::vec3(ChunkPos));
    if (Chunk == m_LoadedChunks.end()) {
        return nullptr;
    }
    return Chunk->second;
}

Block MvWorld::GetWorldBlockAt(glm::vec3 position) {
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos(position);

    auto Chunk = m_LoadedChunks.find(glm::vec3(ChunkPos));
    if (Chunk == m_LoadedChunks.end()) {
        return Block(BlockType::INVALID, 0);
    }

    glm::ivec3 blockPos = {
        std::floor(position.x - (ChunkPos.x * MvChunk::CHUNK_SIZE)),
        std::floor(position.y - (ChunkPos.y * MvChunk::CHUNK_SIZE)),
        std::floor(position.z - (ChunkPos.z * MvChunk::CHUNK_SIZE)),
    };

    return Chunk->second->GetBlock(blockPos);
}

void MvWorld::SetWorldLightBlockAt(glm::vec3 position, short global_light_level) {
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos(position);

    auto Chunk = m_LoadedChunks.find(glm::vec3(ChunkPos));
    if (Chunk == m_LoadedChunks.end()) {
        return;
    }

    glm::ivec3 blockPos = {
        std::floor(position.x - (ChunkPos.x * MvChunk::CHUNK_SIZE)),
        std::floor(position.y - (ChunkPos.y * MvChunk::CHUNK_SIZE)),
        std::floor(position.z - (ChunkPos.z * MvChunk::CHUNK_SIZE)),
    };

    Chunk->second->SetLight(blockPos, global_light_level);
}

void MvWorld::SetWorldBlockAt(glm::vec3 position, BlockType blockType) {
    glm::ivec3 ChunkPos = ConvertBlockPosToChunkPos(position);


    // Check for valid chunk
    if (m_LoadedChunks.find(glm::vec3(ChunkPos)) == m_LoadedChunks.end()) {
        return;
    }

    // safety LOL XD #C++
    auto &chunk = m_LoadedChunks[glm::vec3(ChunkPos)];
    if (!chunk) {
        return;
    }

    glm::ivec3 blockPos = {
        std::floor(position.x - (ChunkPos.x * MvChunk::CHUNK_SIZE)),
        std::floor(position.y - (ChunkPos.y * MvChunk::CHUNK_SIZE)),
        std::floor(position.z - (ChunkPos.z * MvChunk::CHUNK_SIZE)),
    };
    std::array<std::shared_ptr<MvChunk>, 6> Neighbor = GetNeighborChunks(ChunkPos);
    std::array<glm::ivec3, 6> relativeOffsets = GetRelativeCords();

    m_DirtyMeshChunks[ChunkPos] = chunk;

    //Mark neighbor chunks as dirty
    for (int i = 0; i < 6; ++i) {
        glm::ivec3 offset = relativeOffsets[i];
        glm::ivec3 chunkCoord = {ChunkPos.x + offset.x, ChunkPos.y + offset.y, ChunkPos.z + offset.z};

        if (Neighbor[i]) {
            m_DirtyMeshChunks[chunkCoord] = Neighbor[i]; // Assign neighbor chunk
        }
    }
    chunk->SetBlockAt(blockPos, blockType);

    // update light acces for block
    glm::vec3 LightBlockPos = {position.x, MAX_CHUNK_HEIGTH * MvChunk::CHUNK_SIZE - 1, position.z};
    for (; LightBlockPos.y > 0; LightBlockPos.y--) {
        if (GetWorldBlockAt(LightBlockPos).type == BlockType::AIR) {
            SetWorldLightBlockAt(LightBlockPos, GlobalLightLevel);
            sunlightBfsQueue.emplace(LightNode{{
                        std::floor(LightBlockPos.x),
                         std::floor(LightBlockPos.y),
                         std::floor(LightBlockPos.z)}});
        }
        else
            break;
    }
    //add block beneath to removelight
    if (blockType != BlockType::AIR) {
        for (int y = position.y; y > 0; --y) {
            reverseSunlightBfsQueue.emplace(LightNode{{
                                std::floor(position.x),
                                 y,
                                 std::floor(position.z)}});
        }
    }

    for (int i = 0; i < 6; ++i) {
        sunlightBfsQueue.emplace(LightNode{{
                                        std::floor(position.x + relativeOffsets[i].x),
                                         std::floor(position.y + relativeOffsets[i].y),
                                         std::floor(position.z + relativeOffsets[i].z)}});
    }

}

void MvWorld::CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad) {
    for (auto it = m_RenderChunks.begin(); it != m_RenderChunks.end(); ) {
        glm::vec3 chunkPosition = it->first * glm::vec3( MvChunk::CHUNK_SIZE);
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

