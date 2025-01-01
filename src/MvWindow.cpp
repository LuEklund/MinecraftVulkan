//
// Created by Lucas on 2025-01-01.
//

#include "MvWindow.h"
#include <stdexcept>

MvWindow::MvWindow(int width, int heigth, const std::string& Title)
{
	m_width = width;
	m_heigth = heigth;
	m_title = Title;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_window = glfwCreateWindow(m_width, m_heigth, m_title.c_str(), nullptr, nullptr);
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
