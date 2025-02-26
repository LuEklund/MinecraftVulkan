#include <array>
#include <queue>

#include "MvGameObject.hpp"


class MvWorld;

enum class BlockType : int8_t{
    AIR,
    STONE,
    DIRT,
    GRASS,
    GRASS_2,
};

// constexpr glm::vec4 CalculateUV(int x, int y);
constexpr glm::vec4 CalculateUV(int x, int y) {
    float step = 1.0 / 2.f;
    return glm::vec4(step * x, step * y, step * x + step, step * y + step);
}

constexpr glm::vec4 BLOCK_UVS[5] =
{
    CalculateUV(0, 0), // ---
    CalculateUV(0, 0), //STONE
    CalculateUV(1, 0), //DIRT
    CalculateUV(0, 1), //GRASS-TOP
    CalculateUV(1, 1)  //GRASS-SIDE
};
constexpr const glm::vec4& GetUV(BlockType blockType) {
    return BLOCK_UVS[static_cast<std::int8_t>(blockType)];
}




typedef struct s_block {
    BlockType type;
    int8_t light;
} Block;

typedef struct s_LightNode {
    // s_LightNode(short indx) : index(indx){}
    short index; //this is the x y z coordinate!
} LightNode;



class MvChunk
{
public:
    static short GlobalLightLevel;
    std::queue<LightNode> sunlightBfsQueue;

    static constexpr int CHUNK_SIZE = 16;





    MvChunk();

    Block DATA[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {};

    void GenerateChunk(glm::vec3 ChunkPos);
    MvModel::Builder GenerateMesh(const std::array<std::array<std::array<Block , CHUNK_SIZE + 2>,CHUNK_SIZE + 2>,CHUNK_SIZE + 2> &Blocks, glm::vec3 ChunkPos);

    float CalculateAmbientOcclusion(Block Side1, Block Corner, Block Side2);
    void CalculateLight();

    Block GetBlock(glm::ivec3 vec);
    void SetBlockAt(glm::ivec3 vec, BlockType blockType);

    std::shared_ptr<MvModel> &GetModel() {return m_model;}
    void SetModel(std::shared_ptr<MvModel> model);
    bool HasMesh(){return bHasMesh;}
    bool GetRender(){return bRender;}
    void SetRender(bool render){bRender = render;}





private:
    float Continentalness(float x);
    void LightPropagate(int x, int y, int z, int lightLevel);

    bool    bHasMesh = false;
    bool    bRender = false;
    std::shared_ptr<MvModel> m_model{};



};

