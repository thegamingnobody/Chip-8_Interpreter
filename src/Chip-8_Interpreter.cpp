#include "Chip-8_Interpreter.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include "Logger.h"

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
	Logger::GetInstance().Init(true);
}

Chip8::Interpreter::~Interpreter()
{
	Renderer::GetInstance().Destroy();
	SDL_Quit();
}

void Chip8::Interpreter::LoadGame(const std::string& gameName)
{
	//Todo: improve to allow any path (windows messagebox thingy)
	std::string relativePath{ "../../../roms/" + gameName };
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

	Logger::GetInstance().Log("Game loaded successfully");
}

void Chip8::Interpreter::EmulateCycle()
{
	auto& logger = Logger::GetInstance();
	////Todo: delete this temp
	//m_Memory[m_PC] = 0xA2;
	//m_Memory[m_PC + 1] = 0xF0;

	//Todo: Fetch opcode
	m_I = m_Memory[m_PC] << 8 | m_Memory[m_PC + 1];

	//Todo: Decode opcode
	//get the value of the first 4 bits of the opcode to determine the instruction type
	byte instructionType = (m_I & 0xF000) >> 8 >> 4;

	//Todo: Execute opcode
	bool instructionExecuted{ true };
	
	switch (instructionType)
	{
	case 0x0:
		//0x0NNN: don't implement
		//0x00E0: Clears the screen		
		{
			byte instructionParam = (m_I & 0x00FF);
			if (instructionParam == 0xE0)
			{
				Renderer::GetInstance().ClearScreen();
				m_PC += 2;
			}
			else
			{
				instructionExecuted = false;
			}
			break;
		}
	case 0x1:
		//0x1NNN: Jump to address NNN
		m_PC = m_I & 0x0FFF;
		break;
	case 0x6:
		//0x6XNN: Set register vX to value NN
		{
			int registerIndex = (m_I & 0x0F00) >> 8;
			byte value = m_I & 0x00FF;
			m_V[registerIndex] = value;
		}
		break;
	case 0x7:
		//0x7XNN: Add value NN to register vX
		break;
	case 0xA:
		//0xANNN: Set index register to value NNN
		break;
	case 0xD:
		//0xDXYN: Draw/Render
		break;
	default:
		instructionExecuted = false;
		break;
	}

#ifdef MY_DEBUG
	if (instructionExecuted)
	{
		logger.SetHexMode();
		logger.Log("[EXEC] instruction executed:", m_I);
	}
	else
	{
		logger.SetHexMode();
		logger.Log("[ERROR] instruction failed:", m_I);
	}
#endif // MY_DEBUG

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