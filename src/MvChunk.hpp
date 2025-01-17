#include "MvGameObject.hpp"

class MvChunk 
{
public:
    static constexpr int CHUNK_SIZE = 4;

    MvChunk();
    // MvChunk& operator=(const MvChunk&) = delete;
    // MvChunk(MvChunk&&) noexcept = default;
    // MvChunk& operator=(MvChunk&&) noexcept = default;

    int data[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];

	std::unique_ptr<MvModel> CreateCubeModel(MvDevice& device, glm::vec3 offset);
    void GenerateMesh(MvDevice& device, std::vector<MvGameObject> &gameObjects);
    

    // std::vector<MvGameObject> &GetGameObjects() { return m_GameObjects; }

    void SetPosition(const glm::vec3 &position) { m_ChunkPosition = position; }
    const glm::vec3 &GetPosition() const { return m_ChunkPosition; }
    
private:
    glm::vec3 m_ChunkPosition;
    // MvGameObject m_GameObject;
	// std::vector<MvGameObject> m_GameObjects;
};


