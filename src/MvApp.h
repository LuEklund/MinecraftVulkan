#pragma once

#include "MvWindow.h"
#include "MvDevice.hpp"
#include "MvChunk.hpp"
#include "MvRenderer.hpp"
#include "MvDescriptors.hpp"
#include "MvTexture.hpp"

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
	std::vector<MvChunk> m_chunks;
	std::vector<MvGameObject> m_gameObjects;
    // std::shared_ptr<MvModel> m_cubeModel;

	//Texture
	std::unique_ptr<MvTexture> m_texture;


	//5. Descriptors
	std::unique_ptr<MvDescriptorPool> m_GlobalPool;

};

