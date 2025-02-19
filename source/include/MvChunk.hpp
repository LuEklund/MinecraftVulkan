#include "MvGameObject.hpp"


class MvChunk 
{
public:
    static constexpr int CHUNK_SIZE = 16;

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



    MvChunk();

    int data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE] = {0};

    glm::vec4 CalculateUV(int x, int y);
    void GenerateChunk();

    float* CalculateAmbientOcclusions(int x, int y, int z);

    void GenerateMesh(MvDevice &device);

    // std::vector<MvGameObject> &GetGameObjects() { return m_GameObjects; }

    void SetPosition(const glm::vec3 &position) { m_ChunkPosition = position; }
    const glm::vec3 &GetPosition() const { return m_ChunkPosition; }

    std::shared_ptr<MvModel> &GetModel() {return m_model;}

    int GetBlock(int x, int y, int z) {return data[x][y][z];}

    void DestroyBlockAt(glm::ivec3 vec);;
    void SetBlockAt(glm::ivec3 vec, int blockType);
    bool HasMesh(){return bHasMesh;}
    bool GetRender(){return bRender;}
    void SetRender(bool render){bRender = render;}


private:
    float Continentalness(float x);

    bool    bHasMesh = false;
    bool    bRender = false;
    glm::vec3 m_ChunkPosition;
    std::shared_ptr<MvModel> m_model{};


    // MvGameObject m_GameObject;
	// std::vector<MvGameObject> m_GameObjects;
};
