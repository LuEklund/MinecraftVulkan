#pragma once

#include "Hash.hpp"
#include "MvWindow.hpp"
#include "MvDevice.hpp"
#include "MvChunk.hpp"
#include "MvRenderer.hpp"
#include "MvDescriptors.hpp"
#include "MvTexture.hpp"
#include "MvCamera.hpp"

#include <memory>

#include "Ref.hpp"




class MvApp
{
public:
	MvApp();
	~MvApp();

	MvApp(const MvApp&) = delete;
	MvApp& operator=(const MvApp&) = delete;

	void Run();

	//A getter for app instance
	static MvApp* GetInstance();


	MvDevice &GetDevice() {return *m_Device;};

	//World
	std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_chunks;};

	//Camera
	const MvCamera &GetCamera() {return *m_Camera;};

private:
	//singleton
	// inline static Ref<MvApp> instance = nullptr;

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
	std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_chunks;

	//camera
	std::unique_ptr<MvCamera> m_Camera{};
	//Texture
	std::unique_ptr<MvTexture> m_texture;

	//5. Descriptors
	std::unique_ptr<MvDescriptorPool> m_GlobalPool;

};

