//
// Created by Lucas on 2025-01-01.
//

#include "MvWindow.hpp"
#include <stdexcept>

MvWindow::MvWindow(int width, int heigth, const std::string& Title)
{
	m_width = width;
	m_heigth = heigth;
	m_title = Title;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	m_window = glfwCreateWindow(m_width, m_heigth, m_title.c_str(), nullptr, nullptr);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
}

MvWindow::~MvWindow()
{
	glfwDestroyWindow(m_window);
	glfwTerminate();
}

void MvWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, m_window, nullptr, surface) != VK_SUCCESS)
	{
		throw std::runtime_error("Failed to create window surface!");
	}
}

void MvWindow::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
	MvWindow *Window = reinterpret_cast<MvWindow*>(glfwGetWindowUserPointer(window));
	Window->m_frameBufferResized = true;
	Window->m_width = width;
	Window->m_heigth = height;
}
