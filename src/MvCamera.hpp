#pragma once

// Library
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


class MvCamera
{
   
public:
    MvCamera();
    ~MvCamera();

    void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void SetPerspectiveProjection(float fovY, float aspect, float near, float far);

    void SetViewDirection(const glm::vec3 position, const glm::vec3 direction, const glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
    void SetViewTarget(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up = glm::vec3(0.f, -1.f, 0.f));
    void SetViewYXZ(const glm::vec3 position, const glm::vec3 rotation);

    const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }
private:

    glm::mat4 m_projectionMatrix{1.f};
    glm::mat4 m_viewMatrix{1.f};
};

