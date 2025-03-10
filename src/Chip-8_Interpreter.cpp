#include "Chip-8_Interpreter.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>

Chip8::Interpreter::Interpreter()
	: m_Memory()
	, m_V()
	, m_Keys()
	, m_Stack()
	, m_I(0)
	, m_PC(0x0200)
	, m_DelayTimer()
	, m_SoundTimer()
	, m_SP()
	, m_DrawFlag(true)
{
	//Resize memory to 4KB and initialize registers
	m_Memory.resize(4096);
	m_V.resize(16);
	m_Keys.resize(16);

	//Todo: Clear display	
	//Todo: Clear stack
	//Todo: Clear registers V0-VF
	//Todo: Clear memory
	//Todo: Load fontset
	//Todo: Reset timers

	//Initialize Singletons
	//Todo: Read in from config file?
	int windowWidth{ 64 };
	int windowHeight{ 32 };
	float windowScale{ 16.0f };

	//ScreenManager::GetInstance().Init(windowWidth, windowHeight, windowScale);
	Renderer::GetInstance().Init(windowWidth, windowHeight, windowScale);
	InputManager::GetInstance().Init();
}

Chip8::Interpreter::~Interpreter()
{
	Renderer::GetInstance().Destroy();
	SDL_Quit();
}

void Chip8::Interpreter::LoadGame(const std::string& gameName)
{
	std::string relativePath{ "../../../roms/" + gameName };
	//Todo: Load game into memory
	std::ifstream input(relativePath, std::ios::binary | std::ios::ate);
	if (!input.is_open())
	{
		std::cerr << "Failed to open file: " << relativePath << "\n";
		return;
	}

	//Get file size
	std::streamsize fileSize = input.tellg();
	input.seekg(0, std::ios::beg);

	assert(fileSize < m_Memory.size());

	//Todo: improve with a std algorithm
	for (int i = 0; i < fileSize; ++i)
	{
		m_Memory[i + m_PC] = input.get();
	}

	std::cout << "Game loaded successfully\n";
}

void Chip8::Interpreter::EmulateCycle()
{
	////Todo: delete this temp
	//m_Memory[m_PC] = 0xA2;
	//m_Memory[m_PC + 1] = 0xF0;

	//Todo: Fetch opcode
	m_I = m_Memory[m_PC] << 8 | m_Memory[m_PC + 1];

	//Todo: Decode opcode
	//get the value of the first 4 bits of the opcode to determine the instruction type
	byte instructionType = (m_I & 0xF000) >> 8 >> 4;

	switch (instructionType)
	{
	case 0x0:
		std::cout << "instructionType: 0\n";
		break;
	case 0x1:
		std::cout << "instructionType: 1\n";
		break;
	case 0x6:
		std::cout << "instructionType: 6\n";
		break;
	case 0x7:
		std::cout << "instructionType: 7\n";
		break;
	case 0xA:
		std::cout << "instructionType: A\n";
		break;
	case 0xD:
		std::cout << "instructionType: D\n";
		break;
	default:
		std::cout << "instructionType: unknown\n";
		break;
	}

	//Todo: Execute opcode
	m_PC += 2;

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