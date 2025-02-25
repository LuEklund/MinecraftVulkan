#include <array>
#include <queue>

#include "MvGameObject.hpp"


class MvWorld;

typedef struct s_block {
    int8_t type;
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
    bool bDirty = true;
    std::queue<LightNode> sunlightBfsQueue;

    static constexpr int CHUNK_SIZE = 16;

    //TODO: Change class Enum of blocks

    static const int AIR = 0;
    static const int STONE = 1;
    static const int DIRT = 2;
    static const int GRASS = 3;
    static const int GRASS_2 = 4;

    const glm::vec4 BLOCK_UVS[5] =
	{
        CalculateUV(0, 0), // ---
        CalculateUV(0, 0), //STONE
        CalculateUV(1, 0), //DIRT
        CalculateUV(0, 1), //GRASS-TOP
        CalculateUV(1, 1)  //GRASS-SIDE
    };



    MvChunk(MvWorld &world);

    Block DATA[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    glm::vec4 CalculateUV(int x, int y);
    void GenerateChunk();


    // float* CalculateAmbientOcclusions(int x, int y, int z);
    // float CalculateAmbientOcclusion(glm::ivec3 UpLeft, glm::ivec3 UpMiddle, glm::ivec3 UpRight);
    float CalculateAmbientOcclusion(glm::ivec3 Side1, glm::ivec3 Corner, glm::ivec3 Side2, const std::shared_ptr<MvChunk>& ChunkNeighbor);

    // float CalculateAmbientOcclusion(Block Side1, Block Corner, Block Side2);

    MvModel::Builder GenerateMesh(const std::array<std::shared_ptr<MvChunk>, 6>& ChunkNeighbors);
    Block GetBlock(glm::ivec3 vec, const std::shared_ptr<MvChunk>& ChunkNeighbor);

    void CalculateLight();

    void SetPosition(const glm::vec3 &position) { m_ChunkPosition = position; }
    const glm::vec3 &GetPosition() const { return m_ChunkPosition; }

    std::shared_ptr<MvModel> &GetModel() {return m_model;}

    Block GetBlock(glm::ivec3 vec);

    void DestroyBlockAt(glm::ivec3 vec);;
    void SetBlockAt(glm::ivec3 vec, int blockType);
    void SetModel(std::shared_ptr<MvModel> model);
    bool HasMesh(){return bHasMesh;}
    bool GetRender(){return bRender;}
    void SetRender(bool render){bRender = render;}

    void GenerateMeshForBlock(glm::ivec3 BlockPos, std::array<std::array<std::array<Block,3>,3>,3> Blocks, MvModel::Builder& modelBuilder, int& size) ;

    MvWorld& World;




private:
    float Continentalness(float x);
    void LightPropagate(int x, int y, int z, int lightLevel);

    bool    bHasMesh = false;
    bool    bRender = false;
    glm::vec3 m_ChunkPosition;
    std::shared_ptr<MvModel> m_model{};





    // MvGameObject m_GameObject;
	// std::vector<MvGameObject> m_GameObjects;
};

