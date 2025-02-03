#pragma once

#include "MvWindow.hpp"
#include "MvDevice.hpp"
#include "MvRenderer.hpp"
#include "MvDescriptors.hpp"
#include "MvTexture.hpp"
#include "MvCamera.hpp"
#include "MvWorld.hpp"
#include <memory>

#include "MvCubeMap.hpp"
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
	MvWorld &GetWorld() {return *m_World;};
	// std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> & GetChunks() {return m_chunks;};
	// void SetWorldBlockAt(glm::ivec3 vec, int blockType);
	// int GetWorldBlockAt(glm::ivec3 vec);


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
	std::unique_ptr<MvWorld> m_World{};
	// std::unordered_map<glm::vec3, std::shared_ptr<MvChunk>> m_chunks;

	//camera
	std::unique_ptr<MvCamera> m_Camera{};
	//Texture
	std::unique_ptr<MvTexture> m_texture;

	//SkyBox
	std::unique_ptr<MvCubeMap> m_CubeMap;
	std::unique_ptr<MvModel> m_SkyBox;

	//5. Descriptors
	std::unique_ptr<MvDescriptorPool> m_GlobalPool;

};

