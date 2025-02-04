#include "MvBuffer.hpp"


class MvCubeMap
{
public:
    MvCubeMap(MvDevice& Device);
    ~MvCubeMap();

    VkImageView GetTextureImageView() const { return textureImageView; }
    VkSampler GetTextureSampler() const { return textureSampler; }
private:
    void CreateImageTexture();

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void createTextureImageView();
    void createTextureSampler();

    MvDevice& m_device;
};


