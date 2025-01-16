#include "MvChunk.hpp"
#include <iostream>

MvChunk::MvChunk()
{
  

	// auto cube = MvGameObject::createGameObject();
	// cube.model = cubeModel;
	// // cube.transform.rotation = {0.6f, 1.f, 1.1f};
	// cube.transform.translation = {0.f, 0.f, 0.5f};
	// cube.transform.scale = {.5f, .5f, .5f};
	// m_GameObjects.push_back(std::move(cube));

  int size = 16;
  for (int x = 0; x < size; x++)
  {
      for (int y = 0; y < size; y++)
      {
        for (int z = 0; z < size; z++)
        {
          auto cube = MvGameObject::createGameObject();
          // cube.transform.rotation = {0.6f, 1.f, 1.1f };
          cube.transform.translation = {static_cast<float>(x) + m_ChunkPosition.x * static_cast<float>(CHUNK_SIZE),
                                        static_cast<float>(y) + m_ChunkPosition.y * static_cast<float>(CHUNK_SIZE),
                                        static_cast<float>(z) + m_ChunkPosition.z * static_cast<float>(CHUNK_SIZE)};
          
		// std::cout << "Rendering object x-pos: " << cube.transform.translation.x << std::endl;
        // std::cout << "Rendering object y-pos: " << cube.transform.translation.y << std::endl;
        // std::cout << "Rendering object z-pos: " << cube.transform.translation.z << std::endl;
          
          cube.transform.scale = {1.f, 1.f, 1.f};
          m_GameObjects.push_back(std::move(cube));
      }
    }
  }
}

MvChunk::~MvChunk()
{
}
