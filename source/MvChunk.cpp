#include "MvChunk.hpp"
#include <iostream>

#include "MvPerlinNoise.hpp"


MvChunk::MvChunk() {
}


glm::vec4 MvChunk::CalculateUV(int x, int y) {
    float step = 1.0 / 2.f;
    return glm::vec4(step * x, step * y, step * x + step, step * y + step);
}



void MvChunk::GenerateChunk() {
    float dotTopRight = dot(glm::vec2(2,2), glm::vec2(2,2));

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int TotHeight = CHUNK_SIZE * m_ChunkPosition.y + y;
                int TotX = CHUNK_SIZE * m_ChunkPosition.x + x;
                int TotZ = CHUNK_SIZE * m_ChunkPosition.z + z;

                float Freq = 0.01;
                float Amp = 10.f;

                float NoiseValue = Amp * MvPerlinNoise::Noise(TotX * Freq, TotZ * Freq) + 10;
                // NoiseValue += Amp/2.f * MvPerlinNoise::Noise(TotX * Freq * 4.f, TotZ * Freq * 4.f);
                if (TotHeight < NoiseValue) {
                    data[x][y][z] = 1;
                }
                else if (TotHeight < NoiseValue + 3) {
                    data[x][y][z] = 2;
                }
                else if (TotHeight < NoiseValue + 4) {
                    data[x][y][z] = 3;
                }
                else
                    data[x][y][z] = 0;

                // int XOffSet = 16 + sin(TotX * Freq) * Amp;
                // int ZOffSet = 16 + sin(TotZ * Freq) * Amp;
                //
                // if (TotHeight < XOffSet + ZOffSet) {
                //     data[x][y][z] = 1;
                //     bHasMesh = true;
                // }
                // else if (TotHeight < 32) {
                //     data[x][y][z] = 4;
                // }
                // else {
                //     data[x][y][z] = 0;
                // }
                // if (TotHeight < 3) {
                //     data[x][y][z] = 1;
                // } else if (TotHeight < 6) {
                //     data[x][y][z] = 2;
                // } else if (TotHeight < 7) {
                //     data[x][y][z] = 3;
                // }
                // else {
                //     data[x][y][z] = 4;
                // }
            }
        }
    }
}

void MvChunk::GenerateMesh(MvDevice &device) {
    MvModel::Builder modelBuilder{};

    int size = 0;

    glm::ivec3 start = {
        m_ChunkPosition.x * (CHUNK_SIZE - 1), m_ChunkPosition.y * (CHUNK_SIZE - 1), m_ChunkPosition.z * (CHUNK_SIZE - 1)
    };

    glm::vec3 RED = glm::vec3(1.f, 0.f, 0.f);
    glm::vec3 GREEN = glm::vec3(0.f, 1.f, 0.f);
    glm::vec3 BLUE = glm::vec3(0.f, 0.f, 1.f);
    glm::vec3 YELLOW = glm::vec3(1.f, 1.f, 0.f);
    glm::vec3 CYAN = glm::vec3(0.f, 1.f, 1.f);
    glm::vec3 MAGENTA = glm::vec3(1.f, 0.f, 1.f);


    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int Block = data[x][y][z];

                if (Block == 0)
                    continue;

                //block start base
                float bx = x + start.x;
                float by = y + start.y;
                float bz = z + start.z;

                //Top face
                if (y == CHUNK_SIZE - 1 || data[x][y + 1][z] == AIR) {
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);


                    // Postion, Color, Normal, UV
                    // left-top-back
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1}, RED, {0, 1, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    // left-top-front
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz}, RED, {0, 1, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
                    });
                    // right-top-back
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1}, RED, {0, 1, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    // right-top-front
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz}, RED, {0, 1, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    size += 4;
                }

                //Bottom face
                if (y == 0 || data[x][y - 1][z] == AIR) {
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1}, GREEN, {0, -1, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz}, GREEN, {0, -1, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1}, GREEN, {0, -1, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz}, GREEN, {0, -1, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    size += 4;
                }

                // ====================================================
                //Front face
                if (z == CHUNK_SIZE - 1 || data[x][y][z + 1] == AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    // Left-Bottom
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1}, BLUE, {0, 0, 1}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    // Left-Top
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1}, BLUE, {0, 0, 1}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    // Right-Bottom
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1}, BLUE, {0, 0, 1}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    // Right-Top
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1}, BLUE, {0, 0, 1}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
                    });
                    size += 4;
                }

                //Back face
                if (z == 0 || data[x][y][z - 1] == AIR) {
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz}, YELLOW, {0, 0, -1}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz}, YELLOW, {0, 0, -1}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz}, YELLOW, {0, 0, -1}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz}, YELLOW, {0, 0, -1}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
                    });
                    size += 4;
                }

                //Right face
                if (x == CHUNK_SIZE - 1 || data[x + 1][y][z] == AIR) {
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 1);
                    modelBuilder.indices.push_back(size);

                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz + 1}, CYAN, {1, 0, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz + 1}, CYAN, {1, 0, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by, bz}, CYAN, {1, 0, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx + 1, by + 1, bz}, CYAN, {1, 0, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
                    });
                    size += 4;
                }

                //Left face
                if (x == 0 || data[x + -1][y][z] == AIR) {
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 2);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size);
                    modelBuilder.indices.push_back(size + 3);
                    modelBuilder.indices.push_back(size + 1);

                    modelBuilder.vertices.push_back({
                        {bx, by, bz}, MAGENTA, {-1, 0, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz}, MAGENTA, {-1, 0, 0}, {BLOCK_UVS[Block].x, BLOCK_UVS[Block].y}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by, bz + 1}, MAGENTA, {-1, 0, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].w}
                    });
                    modelBuilder.vertices.push_back({
                        {bx, by + 1, bz + 1}, MAGENTA, {-1, 0, 0}, {BLOCK_UVS[Block].z, BLOCK_UVS[Block].y}
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
    data[vec.x][vec.y][vec.z] = 0;
}

void MvChunk::SetBlockAt(glm::ivec3 vec, int blockType) {
    data[vec.x][vec.y][vec.z] = blockType;
}
