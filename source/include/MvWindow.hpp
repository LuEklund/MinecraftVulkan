#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class MvWindow
{
public:
	MvWindow(int width, int heigth, const std::string& Title);
	~MvWindow();

	MvWindow(const MvWindow&) = delete;
	MvWindow& operator=(const MvWindow&) = delete;

	bool		ShouldClose() const { return glfwWindowShouldClose(m_window); }
	bool		WasWindowResized() { return m_frameBufferResized; }
	void		ResetWindowResizedFlag() { m_frameBufferResized = false; }
	GLFWwindow	*GetWindow() const { return m_window; }


	VkExtent2D GetExtent() const { return { static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_heigth) }; }

	void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

private:
	static void	FramebufferResizeCallback(GLFWwindow* window, int width, int height);
	GLFWwindow* m_window;
	int m_width = 800;
	int m_heigth = 600;

	bool m_frameBufferResized = false;

	std::string m_title;
};

