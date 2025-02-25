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

  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_RenderChunks;}

  void GenerateMeshForChunk(const std::shared_ptr<MvChunk> & shared);

  void UpdateWorld(float frameTime);;

  std::array<std::shared_ptr<MvChunk>, 6> GetNeighborChunks(glm::ivec3 vec);

  void LoadChunksAtCoordinate(glm::vec3 position, int radius = 3);
  Block GetWorldBlockAt(glm::vec3 position);
  void SetWorldBlockAt(glm::vec3 vec, int blockType);
  void CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad);

  static float GetNoise(float x, float y);
  // static float SplineInterpolation(float x, const std::vector<glm::vec2> &points);
  static double GetContinentalness(double x);
  static double GetErosion(double x);
  static double GetPeaksAndValleys(double x);
  static double GetPeaksNoise(double x, double y);
  static double GetDetailNoise(double x, double y);

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

    //TODO: unique ptr chunks
    //TODO: dirty map

    // int RENDER_DISTANCE = 10;
    // MvChunk chunks[RENDER_DISTANCE][RENDER_DISTANCE][RENDER_DISTANCE];
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_RenderChunks;
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_LoadedChunks;
};


#endif //MVWORLD_HPP
