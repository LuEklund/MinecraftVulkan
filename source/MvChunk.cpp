#include <iostream>
#include "MvWorld.hpp"

short MvChunk::GlobalLightLevel = 15;

MvChunk::MvChunk()
{

}

// glm::vec4 CalculateUV(int x, int y) {
//     float step = 1.0 / 2.f;
//     return glm::vec4(step * x, step * y, step * x + step, step * y + step);
// }



void MvChunk::GenerateChunk(glm::vec3 ChunkPos) {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {

            const int TotX = CHUNK_SIZE * static_cast<int>(ChunkPos.x) + x;
            const int TotZ = CHUNK_SIZE * static_cast<int>(ChunkPos.z) + z;

            double continentalness_sample = MvWorld::GetNoise(static_cast<float>(TotX), static_cast<float>(TotZ)) * 1.3;
            double base_continents = MvWorld::GetContinentalness(continentalness_sample);

            double peaks_noise_sample = MvWorld::GetPeaksNoise(static_cast<float>(TotX), static_cast<float>(TotZ));
            double peaks_mult = exp(-pow((base_continents - 0.8f) * 10.0f, 2.f));

            double detail_sample = MvWorld::GetDetailNoise(static_cast<float>(TotX), static_cast<float>(TotZ)) + 1.f;

            double scaled_height = base_continents * (1.0f + peaks_mult * peaks_noise_sample);
            scaled_height *= 100;
            scaled_height += detail_sample * 5.f;

            for (int y = 0; y < CHUNK_SIZE; y++) {
                const int TotHeight = CHUNK_SIZE * static_cast<int>(ChunkPos.y) + y;

                if (TotHeight < floor(scaled_height)) {
                    DATA[x][y][z].type = BlockType::STONE;
                }
                else if (TotHeight < floor(scaled_height) + 3) {
                    DATA[x][y][z].type = BlockType::DIRT;
                }
                else if (TotHeight < floor(scaled_height) + 4) {
                    DATA[x][y][z].type = BlockType::GRASS;
                }
                else
                    DATA[x][y][z].type = BlockType::AIR;
            }

            // for lights
            for (int y = CHUNK_SIZE - 1; y >= 0; y--) {
                if (DATA[x][y][z].type == BlockType::AIR) {
                    DATA[x][y][z].light = GlobalLightLevel;
                    short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
                    sunlightBfsQueue.emplace(index);
                }
                else
                    break;
            }
        }
    }
    // lights
    while (!sunlightBfsQueue.empty()) {
        LightNode &node = sunlightBfsQueue.front();
        short index = node.index;
        sunlightBfsQueue.pop();
        int x = index / (CHUNK_SIZE * CHUNK_SIZE);
        int y = (index % (CHUNK_SIZE * CHUNK_SIZE)) / CHUNK_SIZE;
        int z = index % CHUNK_SIZE;
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
        && DATA[x][y][z].type == BlockType::AIR && DATA[x][y][z].light + 2 <= lightLevel)
        {
        DATA[x][y][z].light = lightLevel - 1;
        short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
        sunlightBfsQueue.emplace(index);
    }
}



float MvChunk::CalculateAmbientOcclusion(Block Side1, Block Corner, Block Side2) {
    int count = (Side1.type != BlockType::AIR) + (Corner.type != BlockType::AIR) + (Side2.type != BlockType::AIR);
    switch (count)
    {
        case 0: return 1.0f; // Fully lit
        case 1: return 0.7f; // Some occlusion
        case 2: return 0.4f; // More occlusion
        case 3: return 0.2f; // Very occluded
        default: return 1.0f; // Default fallback
    }
}


MvModel::Builder MvChunk::GenerateMesh(const std::array<std::array<std::array<Block , CHUNK_SIZE + 2>,CHUNK_SIZE + 2>,CHUNK_SIZE + 2> &Blocks, glm::vec3 ChunkPos) {
    MvModel::Builder modelBuilder{};

    int size = 0;

    glm::ivec3 VertStart = {
        ChunkPos.x * (CHUNK_SIZE - 1), ChunkPos.y * (CHUNK_SIZE - 1), ChunkPos.z * (CHUNK_SIZE - 1)
    };

    //TODO: should be written into helper functions AND maybe moved to world class some functionality?
    for (int x1 = 0; x1 < CHUNK_SIZE; x1++) {
        for (int y1 = 0; y1 < CHUNK_SIZE; y1++) {
            for (int z1 = 0; z1 < CHUNK_SIZE; z1++) {
                int x = x1 + 1;
                int y = y1 + 1;
                int z = z1 + 1;
                Block block = Blocks[x][y][z];

                if (block.type == BlockType::AIR)
                    continue;

                //block start base
                float bx = static_cast<float>(x1 + VertStart.x);
                float by = static_cast<float>(y1 + VertStart.y);
                float bz = static_cast<float>(z1 + VertStart.z);



                //Top face
                //Get block above current block, take chunk above block if requried
                // Block blockAbove = World.GetWorldBlockAt({cx,cy + 1,cz});
                Block blockAbove = Blocks[x][y+1][z];
                if (blockAbove.type == BlockType::AIR){

                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);


                    // left-top-forward
                    float light = static_cast<float>(blockAbove.light);
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y+1][z], Blocks[x-1][y+1][z+1], Blocks[x][y+1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x][y+1][z-1], Blocks[x-1][y+1][z-1], Blocks[x-1][y+1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y+1][z], Blocks[x+1][y+1][z+1], Blocks[x][y+1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x][y+1][z-1], Blocks[x+1][y+1][z-1], Blocks[x+1][y+1][z])
                    });

                    size += 4;


                }
                //Bottom face
                // Block blockBottom = World.GetWorldBlockAt({bx,by - 1,bz});
                Block blockBottom = Blocks[x][y-1][z];
                // if (y == 0 || DATA[x][y - 1][z].type == AIR) {
                if (blockBottom.type == BlockType::AIR){
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);

                    float light = static_cast<float>(blockBottom.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x][y-1][z+1], Blocks[x+1][y-1][z+1], Blocks[x+1][y-1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y-1][z], Blocks[x+1][y-1][z-1], Blocks[x][y-1][z-1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y-1][z], Blocks[x-1][y-1][z+1], Blocks[x][y-1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x][y-1][z-1], Blocks[x-1][y-1][z-1], Blocks[x-1][y-1][z])
                    });

                    size += 4;
                }

                // ====================================================
                //Front face
                // Block blockFront = World.GetWorldBlockAt({bx,by,bz+1});
                Block blockFront = Blocks[x][y][z+1];
                if (blockFront.type == BlockType::AIR){
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    float light = static_cast<float>(blockFront.light);
                    // Left-Bottom
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z+1], Blocks[x-1][y-1][z+1], Blocks[x][y-1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z+1], Blocks[x-1][y+1][z+1], Blocks[x][y+1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z+1], Blocks[x+1][y-1][z+1], Blocks[x][y-1][z+1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z+1], Blocks[x+1][y+1][z+1], Blocks[x][y+1][z+1])
                    });
                    size += 4;
                }

                //Back face
                // Block blockBack = World.GetWorldBlockAt({bx,by,bz-1});
                Block blockBack = Blocks[x][y][z-1];
                // if (z == 0 || DATA[x][y][z - 1].type == BlockType::AIR) {
                if (blockBack.type == BlockType::AIR){
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    float light = static_cast<float>(blockBack.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z-1], Blocks[x+1][y-1][z-1], Blocks[x][y-1][z-1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z-1], Blocks[x+1][y+1][z-1], Blocks[x][y+1][z-1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z-1], Blocks[x-1][y-1][z-1], Blocks[x][y-1][z-1])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z-1], Blocks[x-1][y+1][z-1], Blocks[x][y+1][z-1])
                    });

                    size += 4;
                }

                //Right face
                // Block blockRight = World.GetWorldBlockAt({bx+1,by,bz});
                Block blockRight = Blocks[x+1][y][z];
                if (blockRight.type == BlockType::AIR){
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    float light = static_cast<float>(blockRight.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z+1], Blocks[x+1][y-1][z+1], Blocks[x+1][y-1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z+1], Blocks[x+1][y+1][z+1], Blocks[x+1][y+1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z-1], Blocks[x+1][y-1][z-1], Blocks[x+1][y-1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x+1][y][z-1], Blocks[x+1][y+1][z-1], Blocks[x+1][y+1][z])
                    });

                    size += 4;
                }

                //Left face
                // Block blockLeft = World.GetWorldBlockAt({bx-1,by,bz});
                Block blockLeft = Blocks[x-1][y][z];
                if (blockLeft.type == BlockType::AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    float light = static_cast<float>(blockLeft.light);
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {GetUV(block.type).x, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z-1], Blocks[x-1][y-1][z-1], Blocks[x-1][y-1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {GetUV(block.type).x, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z-1], Blocks[x-1][y+1][z-1], Blocks[x-1][y+1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {GetUV(block.type).z, GetUV(block.type).w},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z+1], Blocks[x-1][y-1][z+1], Blocks[x-1][y-1][z])
                    });

                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {GetUV(block.type).z, GetUV(block.type).y},
                        {light},
                        CalculateAmbientOcclusion(Blocks[x-1][y][z+1], Blocks[x-1][y+1][z+1], Blocks[x-1][y+1][z])
                    });

                    size += 4;
                }

                // std::cout << "Info: " << BLOCK_UVS[Block].x << " : " << BLOCK_UVS[Block].y << " : " << BLOCK_UVS[Block].z << " : " << BLOCK_UVS[Block].w << std::endl;
            }
        }
    }
    if (modelBuilder.vertices.size() == 0)
        bHasMesh = false;
    else
        bHasMesh = true;
    return modelBuilder;
}

void MvChunk::CalculateLight() {
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int z = 0; z < CHUNK_SIZE; z++) {
            int y = CHUNK_SIZE - 1;
            for (y; y >= 0; y--) {
                if (DATA[x][y][z].type == BlockType::AIR) {

                    DATA[x][y][z].light = GlobalLightLevel;
                    short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
                    sunlightBfsQueue.emplace(index);
                }
                else
                    break;
            }
            for (y; y >= 0; y--) {
                DATA[x][y][z].light = 0;
            }
        }
    }

    while (!sunlightBfsQueue.empty()) {
        LightNode &node = sunlightBfsQueue.front();
        short index = node.index;
        sunlightBfsQueue.pop();
        int x = index / (CHUNK_SIZE * CHUNK_SIZE);
        int y = (index % (CHUNK_SIZE * CHUNK_SIZE)) / CHUNK_SIZE;
        int z = index % CHUNK_SIZE;
        int lightLevel = DATA[x][y][z].light;
        LightPropagate(x-1, y, z, lightLevel);
        LightPropagate(x+1, y, z , lightLevel);
        LightPropagate(x, y, z-1, lightLevel);
        LightPropagate(x, y, z+1, lightLevel);
        LightPropagate(x, y-1, z, lightLevel);
        LightPropagate(x, y+1, z, lightLevel);
    }
}


void MvChunk::SetBlockAt(glm::ivec3 vec, BlockType blockType) {
    DATA[vec.x][vec.y][vec.z].type = blockType;
    CalculateLight();
}

void MvChunk::SetModel(std::shared_ptr<MvModel> model) {
    m_model = model;
}


Block MvChunk::GetBlock(glm::ivec3 vec) {
    if (vec.x < 0 || vec.x >= CHUNK_SIZE
        || vec.y < 0 || vec.y >= CHUNK_SIZE
        || vec.z < 0 || vec.z >= CHUNK_SIZE) {
        Block invalidBlock{BlockType::AIR, 0};
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
