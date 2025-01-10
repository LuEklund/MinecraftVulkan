#pragma once

#include "MvWindow.h"
#include "MvDevice.hpp"
#include "MvGameObject.hpp"
#include "MvRenderer.hpp"
#include "MvDescriptors.hpp"

#include <memory>


class MvApp
{
public:
	MvApp();
	~MvApp();

	MvApp(const MvApp&) = delete;
	MvApp& operator=(const MvApp&) = delete;

	void Run();
private:

	//1. Device
	std::unique_ptr<MvDevice> m_Device;

	//2. Window
	std::unique_ptr<MvWindow> m_window;
	static constexpr int WIDTH = 1600;
	static constexpr int HEIGHT = 900;

	//3. Renderer
	std::unique_ptr<MvRenderer> m_renderer;

	//4. Model
	void LoadBlocks();
	std::unique_ptr<MvModel> CreateCubeModel(MvDevice& device, glm::vec3 offset);
	std::vector<MvGameObject> m_GameObjects;

	//5. Descriptors
	std::unique_ptr<MvDescriptorPool> m_GlobalPool;
};

