﻿//
// Created by Lucas on 2025-01-30.
//

#ifndef MVWORLD_HPP
#define MVWORLD_HPP

#include "MvChunk.hpp"
#include "Hash.hpp"
#include <unordered_map>
#include "FastNoiseLite.h"
#include "MvCamera.hpp"
#include "MvRenderer.hpp"


class MvWorld {
  public:
  static constexpr int MAX_CHUNK_HEIGTH = 8;
  short GlobalLightLevel = 15;


  MvWorld(MvDevice &device);

  void InitCamera(MvWindow &Window, MvRenderer &renderer);

  std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_RenderChunks;}

  void GenerateMeshForChunk(const std::shared_ptr<MvChunk> & shared);


  std::array<std::array<std::array<Block , MvChunk::CHUNK_SIZE + 2>,MvChunk::CHUNK_SIZE + 2>,MvChunk::CHUNK_SIZE + 2> GetRelevantBlocks(const glm::vec<3, float> vec, const std::shared_ptr<MvChunk> & shared);

  void UpdateLights();
  void UpdateLights(const glm::vec<3, float> vec);



  void UpdateWorld(GLFWwindow *window, float frameTime);;

  std::array<std::unordered_map<glm::vec<3, float>, std::shared_ptr<MvChunk>>::iterator, 6> GetNeighborITChunks(glm::ivec3 vec);
    std::array<std::shared_ptr<MvChunk>, 6> GetNeighborChunks(glm::ivec3 vec);

  void LoadChunksAtCoordinate(glm::vec3 position, int radius = 3);
  std::shared_ptr<MvChunk> GetChunkChunkPos(glm::vec3 position);
  std::shared_ptr<MvChunk> GetChunkBlockPos(glm::vec3 position);

  Block GetWorldBlockAt(glm::vec3 position);
  glm::ivec3 ConvertBlockPosToChunkPos(glm::vec3 blockPos);

  void SetWorldLightBlockAt(glm::vec3 vec, short global_light_level);

  // void SetWorldBlockLightAt(glm::ivec3 position, short lightLevel);
  void SetWorldBlockAt(glm::vec3 vec, BlockType blockType);
  void CalculateRenderChunks(glm::vec3 origin, glm::vec3 direction, int maxChunkDistance, float fovRad);

  static float GetNoise(float x, float y);
  // static float SplineInterpolation(float x, const std::vector<glm::vec2> &points);
  static double GetContinentalness(double x);
  static double GetErosion(double x);
  static double GetPeaksAndValleys(double x);
  static double GetPeaksNoise(double x, double y);
  static double GetDetailNoise(double x, double y);

  glm::vec3 GetHigestPos(glm::vec3 pos);
  MvCamera &GetCamera() {return *m_Camera;};

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

    std::queue<LightNode> sunlightBfsQueue;
    std::queue<LightNode> reverseSunlightBfsQueue;
    void LightPropagate(int x, int y, int z, int lightLevel);
    void LightBeGonePropagate(int x, int y, int z, int lightLevel);

  bool HasDirectSkyLight(glm::ivec3 BlockPos);

    std::unique_ptr<MvCamera> m_Camera{};

    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_RenderChunks;
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_LoadedChunks;
    std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_DirtyMeshChunks;
};


#endif //MVWORLD_HPP
