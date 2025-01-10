#include "MvDescriptors.hpp"

// std
#include <cassert>
#include <stdexcept>
// *************** Descriptor Set Layout Builder *********************
 
MvDescriptorSetLayout::Builder &MvDescriptorSetLayout::Builder::addBinding(
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}
 
std::unique_ptr<MvDescriptorSetLayout> MvDescriptorSetLayout::Builder::build() const {
  return std::make_unique<MvDescriptorSetLayout>(m_Device, bindings);
}


// *************** Descriptor Set Layout *********************
 
MvDescriptorSetLayout::MvDescriptorSetLayout(
    MvDevice &Device, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : m_Device{Device}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }
 
  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();
 
  if (vkCreateDescriptorSetLayout(
        m_Device.GetDevice(),
        &descriptorSetLayoutInfo,
        nullptr,
        &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}
 
MvDescriptorSetLayout::~MvDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(m_Device.GetDevice(), descriptorSetLayout, nullptr);
}



// *************** Descriptor Pool Builder *********************
 
MvDescriptorPool::Builder &MvDescriptorPool::Builder::addPoolSize(
    VkDescriptorType descriptorType, uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}
 
MvDescriptorPool::Builder &MvDescriptorPool::Builder::setPoolFlags(
    VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
MvDescriptorPool::Builder &MvDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}
 
std::unique_ptr<MvDescriptorPool> MvDescriptorPool::Builder::build() const {
  return std::make_unique<MvDescriptorPool>(m_Device, maxSets, poolFlags, poolSizes);
}
 

 // *************** Descriptor Pool *********************
 
MvDescriptorPool::MvDescriptorPool(
    MvDevice &Device,
    uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : m_Device{Device} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;
 
  if (vkCreateDescriptorPool(m_Device.GetDevice(), &descriptorPoolInfo, nullptr, &descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}
 
MvDescriptorPool::~MvDescriptorPool() {
  vkDestroyDescriptorPool(m_Device.GetDevice(), descriptorPool, nullptr);
}
 
bool MvDescriptorPool::allocateDescriptor(
    const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

    // Might want to create a "DescriptorPoolManager" class that handles this case, and builds
  // a new pool whenever an old pool fills up. But this is beyond our current scope
  if (vkAllocateDescriptorSets(m_Device.GetDevice(), &allocInfo, &descriptor) != VK_SUCCESS) {
    return false;
  }
  return true;
}

void MvDescriptorPool::freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(
      m_Device.GetDevice(),
      descriptorPool,
      static_cast<uint32_t>(descriptors.size()),
      descriptors.data());
}
 
void MvDescriptorPool::resetPool() {
  vkResetDescriptorPool(m_Device.GetDevice(), descriptorPool, 0);
}
 

 // *************** Descriptor Writer *********************
 
MvDescriptorWriter::MvDescriptorWriter(MvDescriptorSetLayout &setLayout, MvDescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}
 
MvDescriptorWriter &MvDescriptorWriter::writeBuffer(
    uint32_t binding, VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
  auto &bindingDescription = setLayout.bindings[binding];
 
  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");
 
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;
 
  writes.push_back(write);
  return *this;
}
 
MvDescriptorWriter &MvDescriptorWriter::writeImage(
    uint32_t binding, VkDescriptorImageInfo *imageInfo) {
  assert(setLayout.bindings.count(binding) == 1 && "Layout does not contain specified binding");
 
  auto &bindingDescription = setLayout.bindings[binding];
 
  assert(
      bindingDescription.descriptorCount == 1 &&
      "Binding single descriptor info, but binding expects multiple");
 
  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pImageInfo = imageInfo;
  write.descriptorCount = 1;
 
  writes.push_back(write);
  return *this;
}
 
bool MvDescriptorWriter::build(VkDescriptorSet &set) {
  bool success = pool.allocateDescriptor(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}
 
void MvDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.m_Device.GetDevice(), writes.size(), writes.data(), 0, nullptr);
}
 