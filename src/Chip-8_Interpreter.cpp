#include "Chip-8_Interpreter.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>
#include "ScreenManager.h"

Chip8::Interpreter::Interpreter()
	: m_I()
	, m_PC()
{
	//Resize memory to 4KB and initialize registers
	m_Memory.resize(4096);
	m_V.resize(16);

	//Initialize Singletons
	//Todo: Read in from config file?
	int windowWidth{ 64 };
	int windowHeight{ 32 };
	float windowScale{ 8.0f };

	ScreenManager::GetInstance().Init(windowWidth, windowHeight, windowScale);
	Renderer::GetInstance().Init(windowWidth, windowHeight, windowScale);
	InputManager::GetInstance().Init();
}

Chip8::Interpreter::~Interpreter()
{
	Renderer::GetInstance().Destroy();
	SDL_Quit();
}

void Chip8::Interpreter::Run()
{
	auto& renderer{ Renderer::GetInstance() };
	auto& inputManager{ InputManager::GetInstance() };

	bool continueRunning{ true };
	int const targetFramerate{ 60 };
	long long const msPerFrame = 1000 / targetFramerate;

	auto lastTime = std::chrono::high_resolution_clock::now();
	while (continueRunning)
	{
		auto const currentTime = std::chrono::high_resolution_clock::now();
		float const deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		continueRunning = inputManager.ProcessInput();

		renderer.Render();

		auto const sleepTime = std::chrono::milliseconds(msPerFrame) - (std::chrono::high_resolution_clock::now() - currentTime);

		std::this_thread::sleep_for(sleepTime);
	}
}
