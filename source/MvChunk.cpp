#include <iostream>
#include "MvWorld.hpp"

short MvChunk::GlobalLightLevel = 15;

MvChunk::MvChunk(MvWorld& in_world)
    : World(in_world)
{

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

            // for lights
            for (int y = CHUNK_SIZE - 1; y >= 0; y--) {
                if (DATA[x][y][z].type == AIR) {
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
        && DATA[x][y][z].type == AIR && DATA[x][y][z].light + 2 <= lightLevel)
        {
        DATA[x][y][z].light = lightLevel - 1;
        short index = x * CHUNK_SIZE * CHUNK_SIZE + y * CHUNK_SIZE + z;
        sunlightBfsQueue.emplace(index);
    }
}



float MvChunk::CalculateAmbientOcclusion(glm::ivec3 Side1, glm::ivec3 Corner, glm::ivec3 Side2, const std::shared_ptr<MvChunk>& ChunkNeighbor) {
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


// void MvChunk::GenerateMeshForBlock(glm::ivec3 BlockPos, std::array<std::array<std::array<Block,3>,3>,3> Blocks, MvModel::Builder& modelBuilder, int& size) {
//     //Top face
//     //Get block above current block, take chunk above block if requried
//     // Block blockAbove = World.GetWorldBlockAt({cx,cy + 1,cz});
//     if (Blocks[1][2][1].type == AIR){
//
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 3);
//
//
//         // left-top-forward
//         float light = static_cast<float>(Blocks[1][2][1].light);
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y+1, z }, { x-1, y+1, z+1 }, { x, y+1, z+1 }, ChunkNeighbors[3])
//         });
//
//         // left-top-back
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x, y+1, z-1 }, { x-1, y+1, z-1 }, { x-1, y+1, z }, ChunkNeighbors[3])
//         });
//         // right-top-back
//         modelBuilder.vertices.push_back({
//             {bx + 1, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y+1, z }, { x+1, y+1, z+1 }, { x, y+1, z+1 }, ChunkNeighbors[3])
//         });
//         // right-top-front
//         modelBuilder.vertices.push_back({
//             {bx + 1, by + 1, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x, y+1, z-1 }, { x+1, y+1, z-1 }, { x+1, y+1, z }, ChunkNeighbors[3])
//         });
//         size += 4;
//
//
//     }
//     //Bottom face
//     // Block blockBottom = World.GetWorldBlockAt({bx,by - 1,bz});
//     Block blockBottom = (y == 0 && ChunkNeighbors[2] != nullptr) ? ChunkNeighbors[2]->GetBlock({x,CHUNK_SIZE - 1,z}) : GetBlock({x,y - 1,z});
//     // if (y == 0 || DATA[x][y - 1][z].type == AIR) {
//     if (blockBottom.type == AIR){
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 2);
//
//         float light = static_cast<float>(blockBottom.light);
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x, y -1, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z }, ChunkNeighbors[2])
//
//         });
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y -1, z }, { x+1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[2])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y -1, z }, { x-1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[2])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x, y -1, z-1}, { x-1, y -1, z-1 }, { x-1, y-1, z}, ChunkNeighbors[2])
//         });
//         size += 4;
//     }
//
//     // ====================================================
//     //Front face
//     // Block blockFront = World.GetWorldBlockAt({bx,by,bz+1});
//     Block blockFront = (z == CHUNK_SIZE - 1 && ChunkNeighbors[5] != nullptr) ? ChunkNeighbors[5]->GetBlock({x,y,0}) : GetBlock({x,y,z + 1});
//     if (blockFront.type == AIR){
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size + 1);
//
//         float light = static_cast<float>(blockFront.light);
//         // Left-Bottom
//         modelBuilder.vertices.push_back({
//             {bx, by, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[5])
//         });
//         // Left-Top
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y +1, z+1 }, { x, y+1, z+1}, ChunkNeighbors[5])
//         });
//         // Right-Bottom
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz + 1},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[5])
//         });
//         // Right-Top
//         modelBuilder.vertices.push_back({
//             {bx + 1, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y +1, z+1 }, { x, y+1, z+1}, ChunkNeighbors[5])
//         });
//         size += 4;
//     }
//
//     //Back face
//     // Block blockBack = World.GetWorldBlockAt({bx,by,bz-1});
//     Block blockBack = (z == 0 && ChunkNeighbors[4] != nullptr) ? ChunkNeighbors[4]->GetBlock({x,y,CHUNK_SIZE - 1}) : GetBlock({x,y,z - 1});
//     // if (z == 0 || DATA[x][y][z - 1].type == AIR) {
//     if (blockBack.type == AIR){
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size);
//
//         float light = static_cast<float>(blockBack.light);
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[4])
//         });
//         modelBuilder.vertices.push_back({
//             {bx + 1, by + 1, bz},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y +1, z-1 }, { x, y+1, z-1}, ChunkNeighbors[4])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[4])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y +1, z-1 }, { x, y+1, z-1}, ChunkNeighbors[4])
//         });
//         size += 4;
//     }
//
//     //Right face
//     // Block blockRight = World.GetWorldBlockAt({bx+1,by,bz});
//     Block blockRight = (x == CHUNK_SIZE - 1 && ChunkNeighbors[1] != nullptr) ? ChunkNeighbors[1]->GetBlock({0,y,z}) : GetBlock({x + 1,y,z});
//     if (blockRight.type == AIR){
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 1);
//         modelBuilder.indices.push_back(size);
//
//         float light = static_cast<float>(blockRight.light);
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z }, ChunkNeighbors[1])
//         });
//         modelBuilder.vertices.push_back({
//             {bx + 1, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y +1, z+1 }, { x+1, y+1, z }, ChunkNeighbors[1])
//         });
//         modelBuilder.vertices.push_back({
//             {bx + 1, by, bz},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y -1, z-1 }, { x+1, y-1, z }, ChunkNeighbors[1])
//         });
//         modelBuilder.vertices.push_back({
//            {bx + 1, by + 1, bz},
//            {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//            {light},
//             CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y+1, z-1 }, { x+1, y+1, z }, ChunkNeighbors[1])
//         });
//         size += 4;
//     }
//
//     //Left face
//     // Block blockLeft = World.GetWorldBlockAt({bx-1,by,bz});
//     Block blockLeft = (x == 0 && ChunkNeighbors[0] != nullptr) ? ChunkNeighbors[0]->GetBlock({CHUNK_SIZE - 1,y,z}) : GetBlock({x - 1,y,z});
//     if (blockLeft.type == AIR) {
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 2);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size);
//         modelBuilder.indices.push_back(size + 3);
//         modelBuilder.indices.push_back(size + 1);
//
//         float light = static_cast<float>(blockLeft.light);
//         modelBuilder.vertices.push_back({
//             {bx, by, bz},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y -1, z-1 }, { x-1, y-1, z }, ChunkNeighbors[0])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz},
//             {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y+1, z-1 }, { x-1, y+1, z }, ChunkNeighbors[0])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by, bz + 1},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y -1, z+1 }, { x-1, y-1, z }, ChunkNeighbors[0])
//         });
//         modelBuilder.vertices.push_back({
//             {bx, by + 1, bz + 1},
//             {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
//             {light},
//             CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y +1, z+1 }, { x-1, y+1, z }, ChunkNeighbors[0])
//         });
//         size += 4;
//     }
// }



MvModel::Builder MvChunk::GenerateMesh(const std::array<std::shared_ptr<MvChunk>, 6>& ChunkNeighbors) {
    MvModel::Builder modelBuilder{};

    int size = 0;

    glm::ivec3 VertStart = {
        m_ChunkPosition.x * (CHUNK_SIZE - 1), m_ChunkPosition.y * (CHUNK_SIZE - 1), m_ChunkPosition.z * (CHUNK_SIZE - 1)
    };

    //TODO: should be written into helper functions AND maybe moved to world class some functionality?
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block block = DATA[x][y][z];

                if (block.type == 0)
                    continue;

                //block start base
                float bx = static_cast<float>(x + VertStart.x);
                float by = static_cast<float>(y + VertStart.y);
                float bz = static_cast<float>(z + VertStart.z);



                //Top face
                //Get block above current block, take chunk above block if requried
                // Block blockAbove = World.GetWorldBlockAt({cx,cy + 1,cz});
                Block blockAbove = (y == CHUNK_SIZE - 1 && ChunkNeighbors[3] != nullptr) ? ChunkNeighbors[3]->GetBlock({x,0,z}) : GetBlock({x,y + 1,z});
                if (blockAbove.type == AIR){

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
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y+1, z }, { x-1, y+1, z+1 }, { x, y+1, z+1 }, ChunkNeighbors[3])
                    });

                    // left-top-back
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x, y+1, z-1 }, { x-1, y+1, z-1 }, { x-1, y+1, z }, ChunkNeighbors[3])
                    });
                    // right-top-back
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y+1, z }, { x+1, y+1, z+1 }, { x, y+1, z+1 }, ChunkNeighbors[3])
                    });
                    // right-top-front
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x, y+1, z-1 }, { x+1, y+1, z-1 }, { x+1, y+1, z }, ChunkNeighbors[3])
                    });
                    size += 4;


                }
                //Bottom face
                // Block blockBottom = World.GetWorldBlockAt({bx,by - 1,bz});
                Block blockBottom = (y == 0 && ChunkNeighbors[2] != nullptr) ? ChunkNeighbors[2]->GetBlock({x,CHUNK_SIZE - 1,z}) : GetBlock({x,y - 1,z});
                // if (y == 0 || DATA[x][y - 1][z].type == AIR) {
                if (blockBottom.type == AIR){
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);

                    float light = static_cast<float>(blockBottom.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x, y -1, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z }, ChunkNeighbors[2])

                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y -1, z }, { x+1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[2])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y -1, z }, { x-1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[2])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x, y -1, z-1}, { x-1, y -1, z-1 }, { x-1, y-1, z}, ChunkNeighbors[2])
                    });
                    size += 4;
                }

                // ====================================================
                //Front face
                // Block blockFront = World.GetWorldBlockAt({bx,by,bz+1});
                Block blockFront = (z == CHUNK_SIZE - 1 && ChunkNeighbors[5] != nullptr) ? ChunkNeighbors[5]->GetBlock({x,y,0}) : GetBlock({x,y,z + 1});
                if (blockFront.type == AIR){
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
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[5])
                    });
                    // Left-Top
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z+1}, { x-1, y +1, z+1 }, { x, y+1, z+1}, ChunkNeighbors[5])
                    });
                    // Right-Bottom
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y -1, z+1 }, { x, y-1, z+1}, ChunkNeighbors[5])
                    });
                    // Right-Top
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z+1}, { x+1, y +1, z+1 }, { x, y+1, z+1}, ChunkNeighbors[5])
                    });
                    size += 4;
                }

                //Back face
                // Block blockBack = World.GetWorldBlockAt({bx,by,bz-1});
                Block blockBack = (z == 0 && ChunkNeighbors[4] != nullptr) ? ChunkNeighbors[4]->GetBlock({x,y,CHUNK_SIZE - 1}) : GetBlock({x,y,z - 1});
                // if (z == 0 || DATA[x][y][z - 1].type == AIR) {
                if (blockBack.type == AIR){
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    float light = static_cast<float>(blockBack.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[4])
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z-1}, { x+1, y +1, z-1 }, { x, y+1, z-1}, ChunkNeighbors[4])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y -1, z-1 }, { x, y-1, z-1}, ChunkNeighbors[4])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z-1}, { x-1, y +1, z-1 }, { x, y+1, z-1}, ChunkNeighbors[4])
                    });
                    size += 4;
                }

                //Right face
                // Block blockRight = World.GetWorldBlockAt({bx+1,by,bz});
                Block blockRight = (x == CHUNK_SIZE - 1 && ChunkNeighbors[1] != nullptr) ? ChunkNeighbors[1]->GetBlock({0,y,z}) : GetBlock({x + 1,y,z});
                if (blockRight.type == AIR){
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    float light = static_cast<float>(blockRight.light);
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y -1, z+1 }, { x+1, y-1, z }, ChunkNeighbors[1])
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z+1 }, { x+1, y +1, z+1 }, { x+1, y+1, z }, ChunkNeighbors[1])
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y -1, z-1 }, { x+1, y-1, z }, ChunkNeighbors[1])
                    });
                    modelBuilder.vertices.push_back({
                       {bx + 1, by + 1, bz},
                       {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                       {light},
                        CalculateAmbientOcclusion({ x+1, y, z-1 }, { x+1, y+1, z-1 }, { x+1, y+1, z }, ChunkNeighbors[1])
                    });
                    size += 4;
                }

                //Left face
                // Block blockLeft = World.GetWorldBlockAt({bx-1,by,bz});
                Block blockLeft = (x == 0 && ChunkNeighbors[0] != nullptr) ? ChunkNeighbors[0]->GetBlock({CHUNK_SIZE - 1,y,z}) : GetBlock({x - 1,y,z});
                if (blockLeft.type == AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    float light = static_cast<float>(blockLeft.light);
                    modelBuilder.vertices.push_back({
                        {bx, by, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y -1, z-1 }, { x-1, y-1, z }, ChunkNeighbors[0])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz},
                        {BLOCK_UVS[block.type].x, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z-1 }, { x-1, y+1, z-1 }, { x-1, y+1, z }, ChunkNeighbors[0])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].w},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y -1, z+1 }, { x-1, y-1, z }, ChunkNeighbors[0])
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1},
                        {BLOCK_UVS[block.type].z, BLOCK_UVS[block.type].y},
                        {light},
                        CalculateAmbientOcclusion({ x-1, y, z+1 }, { x-1, y +1, z+1 }, { x-1, y+1, z }, ChunkNeighbors[0])
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
                if (DATA[x][y][z].type == AIR) {

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

void MvChunk::DestroyBlockAt(glm::ivec3 vec) {
    DATA[vec.x][vec.y][vec.z].type = 0;
    bDirty = true;
    CalculateLight();
}

void MvChunk::SetBlockAt(glm::ivec3 vec, int blockType) {
    DATA[vec.x][vec.y][vec.z].type = blockType;
    bDirty = true;
    CalculateLight();
}

void MvChunk::SetModel(std::shared_ptr<MvModel> model) {
    m_model = model;
}

Block MvChunk::GetBlock(glm::ivec3 vec, const std::shared_ptr<MvChunk>& ChunkNeighbor) {
    // return GetBlock(vec);
    if (vec.x >= 0 && vec.x < CHUNK_SIZE &&
        vec.y >= 0 && vec.y < CHUNK_SIZE &&
        vec.z >= 0 && vec.z < CHUNK_SIZE) {
        return GetBlock({vec.x,vec.y,vec.z});
        }

    vec.x = vec.x < 0 ? CHUNK_SIZE + vec.x : vec.x;
    vec.z = vec.z < 0 ? CHUNK_SIZE + vec.z : vec.z;
    vec.y = vec.y < 0 ? CHUNK_SIZE + vec.y : vec.y;

    return ChunkNeighbor ? ChunkNeighbor->GetBlock({vec.x % 16,vec.y % 16,vec.z % 16}) : Block{-1,0};
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
