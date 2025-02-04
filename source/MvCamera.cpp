#include "MvCamera.hpp"

//std
#include <cassert>
#include <complex.h>
#include <complex.h>
#include <iostream>
#include <limits>

#include "Hash.hpp"
#include "Hash.hpp"
#include "MvApp.hpp"
#include "MvRaycast.hpp"
#include "glm/gtc/constants.hpp"

MvCamera::MvCamera()
{

}

MvCamera::~MvCamera()
{
}

void MvCamera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
  m_projectionMatrix = glm::mat4{1.0f};
  m_projectionMatrix[0][0] = 2.f / (right - left);
  // m_projectionMatrix[1][1] = 2.f / (bottom - top);
  m_projectionMatrix[1][1] = 2.f / (top - bottom);
  m_projectionMatrix[2][2] = 1.f / (far - near);
  m_projectionMatrix[3][0] = -(right + left) / (right - left);
  m_projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
  m_projectionMatrix[3][2] = -near / (far - near);
}

void MvCamera::SetPerspectiveProjection(float fovYRadians, float aspect, float near, float far)
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFovY = tan(fovYRadians / 2.f);
    m_projectionMatrix = glm::mat4{0.0f};
    m_projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovY);
    m_projectionMatrix[1][1] = 1.f / (tanHalfFovY);
    m_projectionMatrix[2][2] = far / (far - near);
    m_projectionMatrix[2][3] = 1.f;
    m_projectionMatrix[3][2] = -(far * near) / (far - near);
    FovRadians = fovYRadians;
}

void MvCamera::SetViewDirection(const glm::vec3 position, const glm::vec3 direction, const glm::vec3 up)
{
    const glm::vec3 w{glm::normalize(direction)};
    const glm::vec3 u{glm::normalize(glm::cross(w, up))};
    const glm::vec3 v{glm::cross(w, u)};

    m_viewMatrix = glm::mat4{1.f};
    m_viewMatrix[0][0] = u.x;
    m_viewMatrix[1][0] = u.y;
    m_viewMatrix[2][0] = u.z;
    m_viewMatrix[0][1] = v.x;
    m_viewMatrix[1][1] = v.y;
    m_viewMatrix[2][1] = v.z;
    m_viewMatrix[0][2] = w.x;
    m_viewMatrix[1][2] = w.y;
    m_viewMatrix[2][2] = w.z;
    m_viewMatrix[3][0] = -glm::dot(u, position);
    m_viewMatrix[3][1] = -glm::dot(v, position);
    m_viewMatrix[3][2] = -glm::dot(w, position);
}

void MvCamera::SetViewTarget(const glm::vec3 position, const glm::vec3 target, const glm::vec3 up)
{
    SetViewDirection(position, target - position, up);
}

void MvCamera::SetViewYXZ()
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
    m_viewMatrix = glm::mat4{1.f};
    m_viewMatrix[0][0] = u.x;
    m_viewMatrix[1][0] = u.y;
    m_viewMatrix[2][0] = u.z;
    m_viewMatrix[0][1] = v.x;
    m_viewMatrix[1][1] = v.y;
    m_viewMatrix[2][1] = v.z;
    m_viewMatrix[0][2] = w.x;
    m_viewMatrix[1][2] = w.y;
    m_viewMatrix[2][2] = w.z;
    m_viewMatrix[3][0] = -glm::dot(u, position);
    m_viewMatrix[3][1] = -glm::dot(v, position);
    m_viewMatrix[3][2] = -glm::dot(w, position);
}

//Movement

void MvCamera::MoveInPlaneXZ(GLFWwindow *window, float deltaTime)
{
    glm::vec3 newRotation{0};

    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        newRotation.y += 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        newRotation.y -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        newRotation.x -= 1.f;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        newRotation.x += 1.f;
    }
    if (glm::dot(newRotation, newRotation) > std::numeric_limits<float>::epsilon())
    {
        rotation += RotateSpeed * deltaTime * glm::normalize(newRotation);
    }

    // prevent flipping over
    rotation.x = glm::clamp(rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());

    //prevent overflow of rotation
    rotation.y = glm::mod(rotation.y, glm::two_pi<float>());

    float yaw = rotation.y;
    const glm::vec3 forwardDitrection = {glm::sin(yaw), 0.f, glm::cos(yaw)};
    const glm::vec3 rightDirection = {forwardDitrection.z, 0.f, -forwardDitrection.x};
    const glm::vec3 upDirection = {0.f, 1.f, 0.f};

    //SPEED
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        MoveSpeed = 30.f;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_RELEASE) {
        MoveSpeed = 3.f;
    }

    glm::vec3 moveDirection{0.f};
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        moveDirection += forwardDitrection;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        moveDirection -= forwardDitrection;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        moveDirection += rightDirection;
    }
    if (glfwGetKey(window,  GLFW_KEY_A) == GLFW_PRESS)
    {
        moveDirection -= rightDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        moveDirection += upDirection;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        moveDirection -= upDirection;
    }

    if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
    {
        position += MoveSpeed * deltaTime * glm::normalize(moveDirection);
    }

}

glm::vec3 MvCamera::GetForward() const {
    glm::vec3 forwardDirection = {
        glm::cos(rotation.x) * glm::sin(rotation.y), // X
        -glm::sin(rotation.x),                 // Y
        glm::cos(rotation.x) * glm::cos(rotation.y)  // Z
    };
    return forwardDirection;
    // return glm::vec3{glm::cos(rotation.y), 0.f, glm::sin(rotation.y)};
}
glm::vec3 MvCamera::GetRight() const {
    glm::vec3 rightDirection = {
        glm::cos(rotation.y), // X
        0.f,                  // Y
        -glm::sin(rotation.y) // Z
    };
    return rightDirection;
}

// glm::vec3 MvCamera::GetRight() const {
//     glm::vec3 forward = GetForward();
//     glm::vec3 up{0.f, 1.f, 0.f}; // Global up direction in Y-up
//     return glm::normalize(glm::cross(forward, up));
// }

void MvCamera::SetUpListeners(GLFWwindow *window) {
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (action == GLFW_PRESS) {
            if (key == GLFW_KEY_ENTER || key == GLFW_KEY_P) {
                auto app = static_cast<MvApp *>(glfwGetWindowUserPointer(window));
                glm::vec3 camPos = app->GetCamera().GetPosition();
                glm::vec3 camRot = app->GetCamera().GetRotation();
                float yaw = camRot.y;   // Rotation around Y-axis (horizontal)
                float pitch = camRot.x; // Rotation around X-axis (vertical)
                // Calculate forward direction based on yaw and pitch
                glm::vec3 forwardDirection = app->GetCamera().GetForward();
                // std::cout << "pitch: " << pitch << std::endl;
                MvRaycastResult HitRes = MvRaycast::CastRay(app->GetWorld().GetChunks(), camPos, forwardDirection, 4.f);

                //TODO: needs to be moved into update func
                if (key == GLFW_KEY_ENTER) {
                    if (HitRes.Hit) {
                        HitRes.ChunkHit->DestroyBlockAt(HitRes.BlockPosInChunk);
                        HitRes.ChunkHit->GenerateMesh(app->GetDevice());
                    }
                }
                else {
                    if (HitRes.Hit) {
                         app->GetWorld().SetWorldBlockAt(HitRes.PrevPos, 1);
                    }
                }


                //std::cout << "Hit " << HitRes.Hit << std::endl;
            }
        }
    });
}

