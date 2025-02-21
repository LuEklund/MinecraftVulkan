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
    uint32_t mipLevels;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void GenerateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    void createTextureImageView();
    void createTextureSampler();

    MvDevice& m_device;
};

