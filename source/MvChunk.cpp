#include <iostream>
#include "MvWorld.hpp"


MvChunk::MvChunk() {
}


glm::vec4 MvChunk::CalculateUV(int x, int y) {
    float step = 1.0 / 2.f;
    return glm::vec4(step * x, step * y, step * x + step, step * y + step);
}



void MvChunk::GenerateChunk() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {

            const int TotX = CHUNK_SIZE * static_cast<int>(m_ChunkPosition.x) + x;
            const int TotZ = CHUNK_SIZE * static_cast<int>(m_ChunkPosition.z) + z;

            double continentalness_sample = MvWorld::GetNoise(static_cast<float>(TotX), static_cast<float>(TotZ)) * 1.3;
            double base_continents = MvWorld::GetContinentalness(continentalness_sample);

            double peaks_noise_sample = MvWorld::GetPeaksNoise(static_cast<float>(TotX), static_cast<float>(TotZ));
            double peaks_mult = exp(-pow((base_continents - 0.8f) * 10.0f, 2.f));

            double detail_sample = MvWorld::GetDetailNoise(static_cast<float>(TotX), static_cast<float>(TotZ)) + 1.f;

            double scaled_height = base_continents * (1.0f + peaks_mult * peaks_noise_sample);
            scaled_height *= 100;
            scaled_height += detail_sample * 5.f;

            for (int y = 0; y < CHUNK_SIZE; y++) {
                const int TotHeight = CHUNK_SIZE * static_cast<int>(m_ChunkPosition.y) + y;

                if (TotHeight < floor(scaled_height)) {
                    DATA[x][y][z].type = 1;
                }
                else if (TotHeight < floor(scaled_height) + 3) {
                    DATA[x][y][z].type = 2;
                }
                else if (TotHeight < floor(scaled_height) + 4) {
                    DATA[x][y][z].type = 3;
                }
                else
                    DATA[x][y][z].type = 0;
            }
            for (int y = CHUNK_SIZE - 1; y >= 0; y--) {
                if (DATA[x][y][z].type == AIR) {
                    DATA[x][y][z].light = 15;
                    short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
                    sunlightBfsQueue.emplace(index);
                }
                else
                    break;
            }
        }
    }
    while (!sunlightBfsQueue.empty()) {
        LightNode &node = sunlightBfsQueue.front();
        short index = node.index;
        sunlightBfsQueue.pop();
        int x = index % CHUNK_SIZE;
        int y = index / (CHUNK_SIZE * CHUNK_SIZE);
        int z = (index % (CHUNK_SIZE * CHUNK_SIZE) ) / CHUNK_SIZE;
        int lightLevel = DATA[x][y][z].light;
        LightPropagate(x-1, y, z, lightLevel);
        LightPropagate(x+1, y, z , lightLevel);
        LightPropagate(x, y, z-1, lightLevel);
        LightPropagate(x, y, z+1, lightLevel);
        LightPropagate(x, y-1, z, lightLevel);
        LightPropagate(x, y+1, z, lightLevel);
    }
}

void MvChunk::LightPropagate(int x, int y, int z, int lightLevel) {
    if (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE
        && DATA[x][y][z].type == AIR && DATA[x][y][z].light + 2 <= lightLevel)
        {
        DATA[x][y][z].light = lightLevel - 1;
        short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
        sunlightBfsQueue.emplace(index);
    }
}



float MvChunk::CalculateAmbientOcclusion(glm::ivec3 Side1, glm::ivec3 Corner, glm::ivec3 Side2) {
    int count = (GetBlock(Side1).type > 0) + (GetBlock(Corner).type > 0) + (GetBlock(Side2).type > 0);
    switch (count)
    {
        case 0: return 1.0f; // Fully lit
        case 1: return 0.7f; // Some occlusion
        case 2: return 0.4f; // More occlusion
        case 3: return 0.2f; // Very occluded
        default: return 1.0f; // Default fallback
    }
}

void MvChunk::GenerateMesh(MvDevice &device) {
    //TODO: PASS neighbour chunks to Chunk builder so we can use them for Ambient Occlusion
    MvModel::Builder modelBuilder{};

    int size = 0;

    glm::ivec3 start = {
        m_ChunkPosition.x * (CHUNK_SIZE - 1), m_ChunkPosition.y * (CHUNK_SIZE - 1), m_ChunkPosition.z * (CHUNK_SIZE - 1)
    };




    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block block = DATA[x][y][z];

                if (block.type == 0)
                    continue;

                //block start base
                float bx = static_cast<float>(x + start.x);
                float by = static_cast<float>(y + start.y);
                float bz = static_cast<float>(z + start.z);

                //Top face
                if (y == CHUNK_SIZE - 1 || DATA[x][y + 1][z].type == AIR) {
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);


                    // Postion, Color, Normal, UV
                    // left-top-forward
                    Block blockTop = GetBlock({x,y+1,z});
                    float light = static_cast<float>(blockTop.light);
                    // float light = static_cast<float>(15);
                    // std::cout << light << std::endl;
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y +1, z }, { x-1, y +1, z+1 }, { x, y + 1, z + 1 })
                    });

                    // left-top-back
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x, y +1, z-1 }, { x-1, y +1, z-1 }, { x-1, y + 1, z })
                    });
                    // right-top-back
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y +1, z }, { x+1, y +1, z+1 }, { x, y + 1, z + 1 })
                    });
                    // right-top-front
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x, y +1, z-1 }, { x+1, y +1, z-1 }, { x+1, y + 1, z })
                    });
                    size += 4;


                }
                //Bottom face
                if (y == 0 || DATA[x][y - 1][z].type == AIR) {
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x, y -1, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z })

                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y -1, z }, { x+1, y -1, z-1 }, { x, y-1, z-1})
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y -1, z }, { x-1, y -1, z+1 }, { x, y-1, z+1})
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x, y -1, z-1}, { x-1, y -1, z-1 }, { x-1, y-1, z})
                    });
                    size += 4;
                }

                // ====================================================
                //Front face
                if (z == CHUNK_SIZE - 1 || DATA[x][y][z + 1].type == AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    // Left-Bottom
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y -1, z+1 }, { x, y-1, z+1})
                    });
                    // Left-Top
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y +1, z+1 }, { x, y+1, z+1})
                    });
                    // Right-Bottom
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y -1, z+1 }, { x, y-1, z+1})
                    });
                    // Right-Top
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y +1, z+1 }, { x, y+1, z+1})
                    });
                    size += 4;
                }

                //Back face
                if (z == 0 || DATA[x][y][z - 1].type == AIR) {
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y -1, z-1 }, { x, y-1, z-1})
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y +1, z-1 }, { x, y+1, z-1})
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y -1, z-1 }, { x, y-1, z-1})
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y +1, z-1 }, { x, y+1, z-1})
                    });
                    size += 4;
                }

                //Right face
                if (x == CHUNK_SIZE - 1 || DATA[x + 1][y][z].type == AIR) {
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z })
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y +1, z+1 }, { x+1, y+1, z })
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y -1, z-1 }, { x+1, y-1, z })
                    });
                    modelBuilder.vertices.push_back({
                       {bx + 1, by + 1, bz},
                       {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                       {0},
                        CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y+1, z-1 }, { x+1, y+1, z })
                    });
                    size += 4;
                }

                //Left face
                if (x == 0 || DATA[x + -1][y][z].type == AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y -1, z-1 }, { x-1, y-1, z })
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y+1, z-1 }, { x-1, y+1, z })
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y -1, z+1 }, { x-1, y-1, z })
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {0},
                        CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y +1, z+1 }, { x-1, y+1, z })
                    });
                    size += 4;
                }

                // std::cout << "Info: " << BLOCK_UVS[Block].x << " : " << BLOCK_UVS[Block].y << " : " << BLOCK_UVS[Block].z << " : " << BLOCK_UVS[Block].w << std::endl;
            }
        }
    }
    if (modelBuilder.vertices.size() == 0) {
        bHasMesh = false;
        return;
    }
    bHasMesh = true;


    // auto gameObject = MvGameObject::createGameObject();
    m_model = std::make_unique<MvModel>(device, modelBuilder);
    // gameObject.model = CreateCubeModel(device, {m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z});
    // m_gameObjects.push_back(std::move(gameObject));
}



void MvChunk::DestroyBlockAt(glm::ivec3 vec) {
    DATA[vec.x][vec.y][vec.z].type = 0;
}

void MvChunk::SetBlockAt(glm::ivec3 vec, int blockType) {
    DATA[vec.x][vec.y][vec.z].type = blockType;
    for (int y = vec.y; y >= 0; y--) {
        DATA[vec.x][y][vec.z].light = 0;
    }
}

Block MvChunk::GetBlock(glm::ivec3 vec) {
    if (vec.x < 0 || vec.x >= CHUNK_SIZE
        || vec.y < 0 || vec.y >= CHUNK_SIZE
        || vec.z < 0 || vec.z >= CHUNK_SIZE) {
        Block invalidBlock{-1, 0};
        return invalidBlock;
    }
    return DATA[vec.x][vec.y][vec.z];
}

float MvChunk::Continentalness(float x) {
    if (x < 0)
        return 0;
    else if (x >= 0 && x < 8)
        return 0;
    else if (x >= 8 && x < 9.6f)
        return pow(x - 8, 4);
    else
        return log(x-9.365f) + 8;
}
