#pragma once

#include "MvCamera.hpp"
#include "MvPipeline.hpp"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"
#include "MvFrameInfo.hpp"

#include <memory>



class MvSkyBoxRenderSystem
{
public:
    MvSkyBoxRenderSystem(MvDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout SkyBoxSetLayout);
    ~MvSkyBoxRenderSystem();


    MvSkyBoxRenderSystem(const MvSkyBoxRenderSystem&) = delete;
    MvSkyBoxRenderSystem& operator=(const MvSkyBoxRenderSystem&) = delete;

    void RenderSkyBox(const MvFrameInfo & frame_info, MvModel &SkyBox);

private:

    //1. Device
    MvDevice &m_Device;

    //2. Pipeline
    std::unique_ptr<MvPipeline>m_pipeline;
    // std::unique_ptr<MvPipeline> m_pipeline;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    void	CreatepipelineLayout(VkDescriptorSetLayout SkyBoxSetLayout);
    void	CreatePipeline(VkRenderPass renderPass);

};



