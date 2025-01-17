#include "MvChunk.hpp"
#include <iostream>


MvChunk::MvChunk()
{
  std::cout << "MvChunk Constructor" << std::endl;
}


// temporary helper function, creates a 1x1x1 cube centered at offset
std::unique_ptr<MvModel> MvChunk::CreateCubeModel(MvDevice& device, glm::vec3 offset) {
  MvModel::Builder modelBuilder{};
    modelBuilder.vertices = {
        // Left face
        {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}}, // Bottom-left
        {{-.5f, .5f, .5f},  {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}}, // Top-right
        {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}}, // Bottom-right
        {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}}, // Top-left

        // Right face
        {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, .5f},  {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
        {{.5f, -.5f, .5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, .5f, -.5f}, {.8f, .8f, .1f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},

        // Top face
        {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, -.5f, .5f},  {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},

        // Bottom face
        {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, .5f},  {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, .5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
        {{.5f, .5f, -.5f}, {.8f, .1f, .1f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},

        // Front face
        {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, 0.5f},  {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},

        // Back face
        {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
        {{.5f, .5f, -0.5f},  {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
        {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
        {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
    };
  for (auto& v : modelBuilder.vertices) {
    v.position += offset;
  }
  //Debug Vertex
//    modelBuilder.indices = {
//     // Triangle 1
//     0, 1, 1, 2, 2, 0,
//     // Triangle 2
//     0, 3, 3, 1, 1, 0,
//     // Triangle 3
//     4, 5, 5, 6, 6, 4,
//     // Triangle 4
//     4, 7, 7, 5, 5, 4,
//     // Triangle 5
//     8, 9, 9, 10, 10, 8,
//     // Triangle 6
//     8, 11, 11, 9, 9, 8,
//     // Triangle 7
//     12, 13, 13, 14, 14, 12,
//     // Triangle 8
//     12, 15, 15, 13, 13, 12,
//     // Triangle 9
//     16, 17, 17, 18, 18, 16,
//     // Triangle 10
//     16, 19, 19, 17, 17, 16,
//     // Triangle 11
//     20, 21, 21, 22, 22, 20,
//     // Triangle 12
//     20, 23, 23, 21, 21, 20,
// };

  modelBuilder.indices = {0,  1,  2,  0,  3,  1,  4,  5,  6,  4,  7,  5,  8,  9,  10, 8,  11, 9,
                          12, 13, 14, 12, 15, 13, 16, 17, 18, 16, 19, 17, 20, 21, 22, 20, 23, 21};
 
  return std::make_unique<MvModel>(device, modelBuilder);
}


void MvChunk::GenerateMesh(MvDevice &device, std::vector<MvGameObject>  &m_gameObjects)
{

  MvModel::Builder modelBuilder{};
  
  int size = 0;

  glm::ivec3 start = {m_ChunkPosition.x * CHUNK_SIZE, m_ChunkPosition.y * CHUNK_SIZE, m_ChunkPosition.z * CHUNK_SIZE};
  
  for (int x = 0; x < CHUNK_SIZE; x++)
  {
      for (int y = 0; y < CHUNK_SIZE; y++)
      {
        for (int z = 0; z < CHUNK_SIZE; z++)
        {
          data[x][y][z] = 1;

          //block start base
					float bx = x + start.x;
					float by = y + start.y;
					float bz = z + start.z;

        //Top face
        // if (y == CHUNK_SIZE - 1)
        // {
          modelBuilder.indices.push_back(size + 1);
          modelBuilder.indices.push_back(size);
          modelBuilder.indices.push_back(size + 2);
          modelBuilder.indices.push_back(size + 1);
          modelBuilder.indices.push_back(size + 2);
          modelBuilder.indices.push_back(size + 3);



          // Postion, Color, Normal, UV
          // left-top-back
          modelBuilder.vertices.push_back({ { bx, by + 1, bz + 1 },{ 1, 0, 0 }, {1.0f, 0.0f, 0.0f}, {1, 1} });
          // left-top-front
          modelBuilder.vertices.push_back({ { bx, by + 1, bz },{ 0, 1, 0 }, {1.0f, 0.0f, 0.0f}, {1, 1} });
          // right-top-back
          modelBuilder.vertices.push_back({ { bx + 1, by + 1, bz + 1 },{ 0, 0, 1 }, {1.0f, 0.0f, 0.0f},{1, 1} });
          // right-top-front
          modelBuilder.vertices.push_back({ { bx + 1, by + 1, bz },{ 1, 1, 1 }, {1.0f, 0.0f, 0.0f},{1, 1} });
          size += 4;
        // }
      }
    }
  }

  auto gameObject = MvGameObject::createGameObject();
  gameObject.model = std::make_unique<MvModel>(device, modelBuilder);
  // gameObject.model = CreateCubeModel(device, {m_ChunkPosition.x, m_ChunkPosition.y, m_ChunkPosition.z});
  m_gameObjects.push_back(std::move(gameObject));
}
