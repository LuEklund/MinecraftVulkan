//
// Created by Lucas on 2025-01-30.
//

#ifndef MVWORLD_HPP
#define MVWORLD_HPP

#include "MvChunk.hpp"
#include "Hash.hpp"
#include <unordered_map>




class MvWorld {
  public:
  MvWorld(MvDevice &device);

  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_chunks;};
  void LoadChunksAtCoordinate(glm::vec3 position, int radius = 6);
  void SetWorldBlockAt(glm::ivec3 vec, int blockType);

  private:
  MvDevice& m_Device;
  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_chunks;
};


#endif //MVWORLD_HPP
