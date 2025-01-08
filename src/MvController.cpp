#include "MvController.hpp"

MvController::MvController()
{
}

MvController::~MvController()
{
}

void MvController::MoveInPlaneXZ(GLFWwindow *window, MvGameObject &gameObject, float deltaTime)
{
    glm::vec3 rotation{0};

    if (glfwGetKey(window, Keys.ROTATE_RIGHT) == GLFW_PRESS)
    {
        rotation.y += 1.f;
    }
    if (glfwGetKey(window, Keys.ROTATE_LEFT) == GLFW_PRESS)
    {
        rotation.y -= 1.f;
    }
    if (glfwGetKey(window, Keys.ROTATE_UP) == GLFW_PRESS)
    {
        rotation.x += 1.f;
    }
    if (glfwGetKey(window, Keys.ROTATE_DOWN) == GLFW_PRESS)
    {
        rotation.x -= 1.f;
    }
    if (glm::dot(rotation, rotation) > std::numeric_limits<float>::epsilon())
    {
        gameObject.transform.rotation += RotateSpeed * deltaTime * glm::normalize(rotation);
    }

    // prevent flipping over
    gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -glm::half_pi<float>(), glm::half_pi<float>());

    //prevent overflow of rotation
    gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

    float yaw = gameObject.transform.rotation.y;
    const glm::vec3 forwardDitrection = {glm::sin(yaw), 0.f, glm::cos(yaw)};
    const glm::vec3 rightDirection = {forwardDitrection.z, 0.f, -forwardDitrection.x};
    const glm::vec3 upDirection = {0.f, -1.f, 0.f};

    glm::vec3 moveDirection{0.f};
    if (glfwGetKey(window, Keys.MOVE_FORWARD) == GLFW_PRESS)
    {
        moveDirection += forwardDitrection;
    }
    if (glfwGetKey(window, Keys.MOVE_BACKWARD) == GLFW_PRESS)
    {
        moveDirection -= forwardDitrection;
    }
    if (glfwGetKey(window, Keys.MOVE_RIGHT) == GLFW_PRESS)
    {
        moveDirection += rightDirection;
    }
    if (glfwGetKey(window, Keys.MOVE_LEFT) == GLFW_PRESS)
    {
        moveDirection -= rightDirection;
    }
    if (glfwGetKey(window, Keys.MOVE_UP) == GLFW_PRESS)
    {
        moveDirection += upDirection;
    }
    if (glfwGetKey(window, Keys.MOVE_DOWN) == GLFW_PRESS)
    {
        moveDirection -= upDirection;
    }

    if (glm::dot(moveDirection, moveDirection) > std::numeric_limits<float>::epsilon())
    {
        gameObject.transform.translation += MoveSpeed * deltaTime * glm::normalize(moveDirection);
    }

}