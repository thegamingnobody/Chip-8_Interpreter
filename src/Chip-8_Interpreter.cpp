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

	//Initialize Singletons
	//Todo: Read in from config file?
	int windowWidth{ 64 };
	int windowHeight{ 32 };
	float windowScale{ 16.0f };

	//ScreenManager::GetInstance().Init(windowWidth, windowHeight, windowScale);
	Renderer::GetInstance().Init(windowWidth, windowHeight, windowScale);
	InputManager::GetInstance().Init();
	Logger::GetInstance().Init(true);

	Reset();
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
	auto& renderer = Renderer::GetInstance();

	//Todo: Fetch opcode
	opcode instructionThisCycle = m_Memory[m_PC] << 8 | m_Memory[m_PC + 1];

	//Todo: Decode opcode
	//get the value of the first 4 bits of the opcode to determine the instruction type
	byte instructionType = (instructionThisCycle & 0xF000) >> 8 >> 4;

	//Todo: Execute opcode
	bool instructionExecuted{ true };
	
	switch (instructionType)
	{
	case 0x0:
		//0x0NNN: don't implement
		//0x00E0: Clears the screen		
		{
			byte instructionParam = (instructionThisCycle & 0x00FF);
			if (instructionParam == 0xE0)
			{
				Renderer::GetInstance().ClearScreen();
			}
			else
			{
				instructionExecuted = false;
			}
			break;
		}
	case 0x1:
		//0x1NNN: Jump to address NNN
		m_PC = instructionThisCycle & 0x0FFF;
		m_PC -= 2; //Subtract 2 because the program counter is incremented by 2 after each instruction
		break;
	case 0x6:
		//0x6XNN: Set register vX to value NN
		{
			int registerIndex = (instructionThisCycle & 0x0F00) >> 8;
			byte value = instructionThisCycle & 0x00FF;
			m_V[registerIndex] = value;
		}
		break;
	case 0x7:
		//0x7XNN: Add value NN to register vX
		{
			//Todo: Set Carry flag (how?) (or don't?)
			int registerIndex = (instructionThisCycle & 0x0F00) >> 8;
			byte value = instructionThisCycle & 0x00FF;
			m_V[registerIndex] += value;
		}
		break;
	case 0xA:
		//0xANNN: Set index register to value NNN
		{
			opcode value = instructionThisCycle & 0x0FFF;
			m_I = value;
		}
		break;
	case 0xD:
		//0xDXYN: Draw/Render
		{
			//Todo: make renderer 1 pixel per frame instead of all at once?
			int xIndex = (instructionThisCycle & 0x0F00) >> 8;
			int yIndex = (instructionThisCycle & 0x00F0) >> 4;
			int height = (instructionThisCycle & 0x000F);

			byte xCoordValue;
			byte yCoordValue = m_V[yIndex] % renderer.GetHeight();
			m_V[0xF] = 0;

			for (int row = 0; row < height; row++)
			{
				xCoordValue = m_V[xIndex] % renderer.GetWidth();
				if (yCoordValue > renderer.GetHeight())
				{
					continue;
				}

				byte spriteRow = m_Memory[m_I + row];

				auto bits = ByteToBits(spriteRow);

				for (int pixel = bits.size()-1; pixel >= 0 ; pixel--)
				{
					if (xCoordValue > renderer.GetWidth())
					{
						continue;
					}

					if (bits[pixel])
					{
						if (renderer.IsPixelOn(xCoordValue, yCoordValue))
						{
							m_V[0xF] = 1;
						}
						renderer.TogglePixel(xCoordValue, yCoordValue);
					}
					xCoordValue++;
				}

				yCoordValue++;
			}

			//Todo: Wrap X coordinate of starting position
		}
		break;
	default:
		instructionExecuted = false;
		break;
	}

	m_PC += 2;

#ifdef MY_DEBUG
	if (instructionExecuted)
	{
		logger.SetHexMode();
		logger.Log("[EXEC] instruction executed:", instructionThisCycle);
	}
	else
	{
		logger.SetHexMode();
		logger.Log("[ERROR] instruction failed:", instructionThisCycle);
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

void Chip8::Interpreter::Reset()
{
	m_PC = 0x0200;
	m_I = 0;

	//Todo: Clear display
	Renderer::GetInstance().ClearScreen();
	//Todo: Clear stack
	ClearRegisters();
	//Todo: Clear registers V0-VF
	ClearRegisters();
	//Todo: Clear memory
	ClearMemory();
	//Todo: Load fontset
	LoadFontset();
	//Todo: Reset timers
	ResetTimers();

}

void Chip8::Interpreter::ClearMemory()
{
	std::fill(m_Memory.begin(), m_Memory.end(), 0);
}
void Chip8::Interpreter::ClearStack()
{
	std::fill(m_Stack.begin(), m_Stack.end(), 0);
	m_SP = 0;
}
void Chip8::Interpreter::ClearRegisters()
{
	std::fill(m_V.begin(), m_V.end(), 0);
}
void Chip8::Interpreter::LoadFontset()
{
	//Todo: implement fonset loading
}
void Chip8::Interpreter::ResetTimers()
{
	m_DelayTimer = 0;
	m_SoundTimer = 0;
}

std::vector<bool> Chip8::Interpreter::ByteToBits(byte byteValue) const
{
	//Todo: optimize
	std::vector<bool> bits(8);

	for (int i = 0; i < 8; ++i)
	{
		bits[i] = (byteValue & (1 << i)) != 0;
	}

	return bits;
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