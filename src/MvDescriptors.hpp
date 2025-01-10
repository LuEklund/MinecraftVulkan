#pragma once
 
#include "MvDevice.hpp"
 
// std
#include <memory>
#include <unordered_map>
#include <vector>


class MvDescriptorSetLayout {
 public:
  class Builder {
   public:
    Builder(MvDevice &Device) : m_Device{Device} {}
 
    Builder &addBinding(
        uint32_t binding,
        VkDescriptorType descriptorType,
        VkShaderStageFlags stageFlags,
        uint32_t count = 1);
    std::unique_ptr<MvDescriptorSetLayout> build() const;
 
   private:
    MvDevice &m_Device;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };
 
  MvDescriptorSetLayout(
      MvDevice &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~MvDescriptorSetLayout();
  MvDescriptorSetLayout(const MvDescriptorSetLayout &) = delete;
  MvDescriptorSetLayout &operator=(const MvDescriptorSetLayout &) = delete;
 
  VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }
 
 private:
  MvDevice &m_Device;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;
 
  friend class MvDescriptorWriter;
};



class MvDescriptorPool {
 public:
  class Builder {
   public:
    Builder(MvDevice &Device) : m_Device{Device} {}
 
    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<MvDescriptorPool> build() const;
 
   private:
    MvDevice &m_Device;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };
 
  MvDescriptorPool(
      MvDevice &m_Device,
      uint32_t maxSets,
      VkDescriptorPoolCreateFlags poolFlags,
      const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~MvDescriptorPool();
  MvDescriptorPool(const MvDescriptorPool &) = delete;
  MvDescriptorPool &operator=(const MvDescriptorPool &) = delete;
 
  bool allocateDescriptor(
      const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;
 
  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;
 
  void resetPool();
 
 private:
  MvDevice &m_Device;
  VkDescriptorPool descriptorPool;
 
  friend class MvDescriptorWriter;
};

class MvDescriptorWriter {
 public:
  MvDescriptorWriter(MvDescriptorSetLayout &setLayout, MvDescriptorPool &pool);
 
  MvDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
  MvDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);
 
  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);
 
 private:
  MvDescriptorSetLayout &setLayout;
  MvDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};
 