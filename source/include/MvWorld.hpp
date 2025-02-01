//
// Created by Lucas on 2025-01-30.
//

#ifndef MVWORLD_HPP
#define MVWORLD_HPP

#include "MvChunk.hpp"
#include "Hash.hpp"
#include <unordered_map>
#include "FastNoiseLite.h"




class MvWorld {
  public:
  MvWorld(MvDevice &device);

  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_ChunksLoaded;};

  void LoadChunksAtCoordinate(glm::vec3 position, int radius = 12);
  void SetWorldBlockAt(glm::ivec3 vec, int blockType);
  void CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad);

  static float GetNoise(float x, float y);
  private:
  MvDevice& m_Device;
  static FastNoiseLite m_NoiseGen;
  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_ChunksLoaded;
  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_ChunksUnLoaded;
};


#endif //MVWORLD_HPP
