#include "MvGameObject.hpp"

class MvChunk
{
public:
    static constexpr int CHUNK_SIZE = 16;

    MvChunk();
    ~MvChunk();

    MvChunk& operator=(const MvChunk&) = delete;
    MvChunk(MvChunk&&) noexcept = default;
    MvChunk& operator=(MvChunk&&) noexcept = default;
    

    std::vector<MvGameObject> &GetGameObjects() { return m_GameObjects; }

    void SetPosition(const glm::vec3 &position) { m_ChunkPosition = position; }
    const glm::vec3 &GetPosition() const { return m_ChunkPosition; }
    
private:
    glm::vec3 m_ChunkPosition;
	std::vector<MvGameObject> m_GameObjects;
};


