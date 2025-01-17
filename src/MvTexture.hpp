#include "MvBuffer.hpp"


class MvTexture
{
public:
    MvTexture(MvDevice& Device);
    ~MvTexture();

    VkImageView GetTextureImageView() const { return textureImageView; }
    VkSampler GetTextureSampler() const { return textureSampler; }
private:
    void CreateImageTexture(const std::string& texturePath);

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void createTextureImageView();
    void createTextureSampler();

    MvDevice& m_device;
};

