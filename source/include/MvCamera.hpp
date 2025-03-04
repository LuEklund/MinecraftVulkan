#pragma once

// Library
#include "MvWindow.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class MvCamera
{
   
public:
    MvCamera();
    ~MvCamera();

    //Camera setup
    void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void SetPerspectiveProjection(float fovYRadians, float aspect, float near, float far);

    void SetViewDirection(const glm::vec3 position, const glm::vec3 direction, const glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
    void SetViewTarget(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up = glm::vec3(0.f, 1.f, 0.f));
    void SetViewYXZ();

    const glm::mat4& GetProjectionMatrix() const { return m_projectionMatrix; }
    const glm::mat4& GetViewMatrix() const { return m_viewMatrix; }

    void SetUpListeners(GLFWwindow *window);

    void Update(GLFWwindow *window, float deltaTime);

    //camera movement
    void MoveInPlaneXZ(GLFWwindow *window, float deltaTime);
    void SetPosition(const glm::vec3 &position);

    //Getters
    glm::vec3 GetPosition() const {return position;}

    void SetAspectRatio(float inAspectRatio) {AspectRatio = inAspectRatio;};
    glm::vec3 GetRotation() const {return rotation;};
    glm::vec3 GetForward() const;
    glm::vec3 GetRight() const;
    float GetFovRadians() const {return FovRadians;};

    //flags
    glm::vec3 moveDirection;
    bool bInAir = true;

private:
    //properties
    float FovRadians;
    float AspectRatio;
    glm::vec3 position{};
    glm::vec3 rotation{};
    float MoveSpeed = 3.f;
    float RotateSpeed = 1.5f;
    float Gravity = -10.f;

    // view
    glm::mat4 m_projectionMatrix{1.f};
    glm::mat4 m_viewMatrix{1.f};

};

