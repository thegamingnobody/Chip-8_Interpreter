#include "Chip-8_Interpreter.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>

Chip8::Interpreter::Interpreter()
{
	Renderer::GetInstance().Init(m_WidthBase, m_HeightBase);
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
