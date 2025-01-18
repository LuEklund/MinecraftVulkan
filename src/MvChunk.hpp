#include "MvGameObject.hpp"

class MvChunk 
{
public:
    static constexpr int CHUNK_SIZE = 7;

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


	std::unique_ptr<MvModel> CreateCubeModel(MvDevice& device, glm::vec3 offset);
    glm::vec4 CalculateUV(int x, int y);
    void GenerateChunk();
    void GenerateMesh(MvDevice &device, std::vector<MvGameObject> &gameObjects);

    // std::vector<MvGameObject> &GetGameObjects() { return m_GameObjects; }

    void SetPosition(const glm::vec3 &position) { m_ChunkPosition = position; }
    const glm::vec3 &GetPosition() const { return m_ChunkPosition; }
    
private:
    glm::vec3 m_ChunkPosition;
    // MvGameObject m_GameObject;
	// std::vector<MvGameObject> m_GameObjects;
};


