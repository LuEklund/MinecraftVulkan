#pragma once
#include "MvModel.hpp"

// std
#include <memory>

struct Transform2dComponent {
  glm::vec2 translation{};  // (position offset)
  glm::vec2 scale{1.f, 1.f};
  float rotation;
  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotMatrix{{c, s}, {-s, c}};
    glm::mat2 scaleMat{{scale.x, .0f}, {.0f, scale.y}};
    return rotMatrix * scaleMat;
  }
};

class MvGameObject
{
public:

  using id_t = unsigned int;
  static MvGameObject createGameObject() {
    static id_t currentId = 0;
    return MvGameObject{currentId++};
  }

    MvGameObject(const MvGameObject &) = delete;
    MvGameObject &operator=(const MvGameObject &) = delete;
    MvGameObject(MvGameObject &&) = default;
    MvGameObject &operator=(MvGameObject &&) = default;

    std::shared_ptr<MvModel> model{};
    glm::vec3 color{};
    Transform2dComponent transform2d{};

private:
  MvGameObject(id_t objId) : id{objId} {}
  id_t id;
};

