#include "Chip-8_Interpreter.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>
#include "ScreenManager.h"

Chip8::Interpreter::Interpreter()
	: m_Memory()
	, m_V()
	, m_Keys()
	, m_Stack()
	, m_I()
	, m_PC()
	, m_DelayTimer()
	, m_SoundTimer()
	, m_SP()
	, m_DrawFlag(true)
{
	//Resize memory to 4KB and initialize registers
	m_Memory.resize(4096);
	m_V.resize(16);
	m_Keys.resize(16);

	//Initialize Singletons
	//Todo: Read in from config file?
	int windowWidth{ 64 };
	int windowHeight{ 32 };
	float windowScale{ 16.0f };

	ScreenManager::GetInstance().Init(windowWidth, windowHeight, windowScale);
	Renderer::GetInstance().Init(windowWidth, windowHeight, windowScale);
	InputManager::GetInstance().Init();
}

Chip8::Interpreter::~Interpreter()
{
	Renderer::GetInstance().Destroy();
	SDL_Quit();
}

void Chip8::Interpreter::LoadGame(const std::string& /*gamePath*/)
{
	//Todo: Load game into memory
}

void Chip8::Interpreter::EmulateCycle()
{
	//Todo: Fetch opcode

	//Todo: Decode opcode

	//Todo: Execute opcode

	//Todo: Update timers
}

void Chip8::Interpreter::UpdateRender()
{
	if (m_DrawFlag)
	{
		Chip8::Renderer::GetInstance().Render();
	}
}

bool Chip8::Interpreter::SetkeyStates()
{
	return Chip8::InputManager::GetInstance().ProcessInput();
}

//void Chip8::Interpreter::Run()
//{
//
//	bool continueRunning{ true };
//	int const targetFramerate{ 60 };
//	long long const msPerFrame = 1000 / targetFramerate;
//
//	auto lastTime = std::chrono::high_resolution_clock::now();
//	while (continueRunning)
//	{
//		auto const currentTime = std::chrono::high_resolution_clock::now();
//		float const deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
//		lastTime = currentTime;
//
//		auto const sleepTime = std::chrono::milliseconds(msPerFrame) - (std::chrono::high_resolution_clock::now() - currentTime);
//
//		std::this_thread::sleep_for(sleepTime);
//	}
//}