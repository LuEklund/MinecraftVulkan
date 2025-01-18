#pragma once

#include "MvGameObject.hpp"
#include "MvWindow.hpp"

class MvController
{
private:
public:


    MvController();
    ~MvController();
    
    void MoveInPlaneXZ(GLFWwindow *window, MvGameObject &gameObject, float deltaTime);


    struct KeyMapping
    {
        int MOVE_LEFT = GLFW_KEY_A;
        int MOVE_RIGHT = GLFW_KEY_D;
        int MOVE_FORWARD = GLFW_KEY_W;
        int MOVE_BACKWARD = GLFW_KEY_S;
        int MOVE_UP = GLFW_KEY_SPACE;
        int MOVE_DOWN = GLFW_KEY_LEFT_SHIFT;
        int ROTATE_LEFT = GLFW_KEY_LEFT;
        int ROTATE_RIGHT = GLFW_KEY_RIGHT;
        int ROTATE_UP = GLFW_KEY_UP;
        int ROTATE_DOWN = GLFW_KEY_DOWN;
    };

    KeyMapping Keys{};

    float MoveSpeed = 3.f;
    float RotateSpeed = 1.5f;
};

