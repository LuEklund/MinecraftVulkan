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

  void LoadChunksAtCoordinate(glm::vec3 position, int radius = 8);
  void SetWorldBlockAt(glm::ivec3 vec, int blockType);
  void CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad);

  static float GetNoise(float x, float y);
  // static float SplineInterpolation(float x, const std::vector<glm::vec2> &points);
  static float GetContinentalness(float x);
  static float GetErosion(float x);
  static float GetPeaksAndValleys(float x);
  static float GetPeaksNoise(float x, float y);
  static float GetDetailNoise(float x, float y);

  private:
    MvDevice& m_Device;
    static std::vector<glm::vec2> Continentalness;
    static std::vector<glm::vec2> Erosion;
    static std::vector<glm::vec2> PeaksAndValleys;
    static FastNoiseLite m_detail_noise_gen;
    static FastNoiseLite m_detail_domain_warp;
    static FastNoiseLite m_NoiseGen;
    static FastNoiseLite DomainWarpGen;
    static FastNoiseLite m_noise_gen_peaks;
    static FastNoiseLite m_domain_warp_peaks;
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_ChunksLoaded;
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_ChunksUnLoaded;
};


#endif //MVWORLD_HPP
