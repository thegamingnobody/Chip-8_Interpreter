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
	int const windowWidth{ 64 };
	int const windowHeight{ 32 };
	//Todo: Read in from config file?
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

	//Copy input ifstream into m_Memory
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

	//Fetch opcode
	opcode instructionThisCycle = m_Memory[m_PC] << 8 | m_Memory[m_PC + 1];
	m_PC += 2;

	//Decode opcode
	//get the value of the first 4 bits of the opcode to determine the instruction type
	byte instructionType = (instructionThisCycle & 0xF000) >> 8 >> 4;

	//Todo: Execute opcode
	bool instructionExecuted{ true };
	
	//Todo: Can this be improved/simplified?
	//=> funtion pointer vector?
	switch (instructionType)
	{
	case 0x0:
		//0x0NNN: don't implement
		//0x00E0: Clears the screen		
		//0x00EE: Returns from a subroutine
		instructionExecuted = Instruction_0NNN(instructionThisCycle);
		break;
	case 0x1:
		//0x1NNN: Jump to address NNN
		instructionExecuted = Instruction_1NNN(instructionThisCycle);
		break;
	case 0x2:
		//0x2NNN: Subroutine call. Jump to address NNN
		//Todo: Implement 0x2NNN
		break;
	case 0x3:
		//0x3XNN: skip the next instruction if vX is equal to NN
		//Todo: Implement 0x3XNN
		break;
	case 0x4:
		//0x4XNN: skip the next instruction if vX is NOT equal to NN
		//Todo: Implement 0x4XNN
		break;
	case 0x5:
		//0x5XY0: skip the next instruction if vX is equal to vY
		//Todo: Implement 0x5XY0
		break;
	case 0x6:
		//0x6XNN: Set register vX to value NN
		instructionExecuted = Instruction_6XNN(instructionThisCycle);
		break;
	case 0x7:
		//0x7XNN: Add value NN to register vX
		instructionExecuted = Instruction_7XNN(instructionThisCycle);
		break;
	case 0x8:
		//Todo: CONFIG
		//Todo: 0x8XYN: various different instructions based on the value of N
		//Todo:		- 0x8XY0: vX is set to value of vY
		//Todo:		- 0x8XY1: vX is set to the result of a binary OR  between vX and vY, vY is not affected
		//Todo:		- 0x8XY2: vX is set to the result of a binary AND between vX and vY, vY is not affected
		//Todo:		- 0x8XY3: vX is set to the result of a binary XOR between vX and vY, vY is not affected
		//Todo:		- 0x8XY4: vX is set to vX + vY, vY is not affected. set vF to 1 if addition overflows, otherwise set vF to 0
		//Todo:		- 0x8XY5: vX is set to vX - vY, vY is not affected. if vX is larger, set vF to 1, otherwise set vF to 0
		//Todo:		- 0x8XY7: vX is set to vY - vX, vY is not affected. if vY is larger, set vF to 1, otherwise set vF to 0
		//Todo:		- 0x8XY6: 1) (configurable) vX is set to vY 
		//					  2) shift vX one bit to the right
		//					  3) set vF to to the value of the bit that was shifted out
		//Todo:		- 0x8XYE: 1) (configurable) vX is set to vY 
		//					  2) shift vX one bit to the left
		//					  3) set vF to to the value of the bit that was shifted out
		break;
	case 0x9:
		//0x9XY0: skip the next instruction if vX is NOT equal to vY
		//Todo: Implement 0x9XY0
		break;
	case 0xA:
		//0xANNN: Set index register to value NNN
		instructionExecuted = Instruction_ANNN(instructionThisCycle);
		break;
	case 0xB:
		//Todo: CONFIG
		// 1) originally
		//Todo: 	0xBNNN: jump to address (NNN + value in v0)
		// 2) later on
		//Todo:		0xBXNN: jump to address (XNN + value in vX)
		break;
	case 0xC:
		//Todo: 0xCXNN: 1) generate random number
		//				2) binary AND random number with NN
		//				3) store result of binary AND in vX
		break;
	case 0xD:
		//0xDXYN: Draw/Render
		instructionExecuted = Instruction_DXYN(instructionThisCycle);
		break;
	case 0xE:
		//Todo: 0xEX9E: skip next instruction if key corresponding to value in vX is pressed
		//Todo: 0xEXA1: skip next instruction if key corresponding to value in vX is NOT pressed
		//valid key values: 0 - F
		break;
	case 0xF:
		//Todo: 0xFX07: vX is set to current value of delay timer
		//Todo: 0xFX15: delay timer is set to vX
		//Todo: 0xFX18: sound timer is set to vX
		//Todo: 0xFX1E: index register is set to (index register + vX) => I += vX
		//Todo: 0xFX0A: stops instruction execution untill a key is pressed. Decrement PC unless a key is pressed. delay and sound timers should still decrease. When key is pressed, its value is placed in vX and executions continues
		//				on OG machine, key is registered upon release of key
		//Todo: 0xFX29: set index register to address of character 
		//Todo: 0xFX33: stores 3 digit decimal version of value in vX at address in index register.							example: if vX is 156, I will be 1, I + 1 will be 5 and I + 2 will be 6
		//Todo: 0xFX55: store values of registers v0 to vX (inclusive) sequeltially starting at address in index register,	example: I will be v0, I + 1 will be v1, I + 2 will be v2 etc.
		//Todo: CONFIG
		//Todo: 0xFX65: reverse of 0xFX55, stores values from I to I + X in v0 t vX. congif: does I increment or does it use a temp value
		//Todo: 0xFX
	default:
		instructionExecuted = false;
		break;
	}

//Todo: look up how debug/release mode werkt in CMake 
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
	//Todo: handle input
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

bool Chip8::Interpreter::Instruction_0NNN(opcode baseInstruction)
{
	//Todo: Implement 0x00EE: Return from subroutine
	byte instructionParam = (baseInstruction & 0x00FF);
	if (instructionParam == 0xE0)
	{
		Renderer::GetInstance().ClearScreen();
	}
	else
	{
		return false;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_1NNN(opcode baseInstruction)
{
	m_PC = baseInstruction & 0x0FFF;
	m_PC -= 2; //Subtract 2 because the program counter is incremented by 2 after each instruction
	return true;
}
bool Chip8::Interpreter::Instruction_6XNN(opcode baseInstruction)
{
	int registerIndex = (baseInstruction & 0x0F00) >> 8;
	byte value = baseInstruction & 0x00FF;
	m_V[registerIndex] = value;

	return true;
}
bool Chip8::Interpreter::Instruction_7XNN(opcode baseInstruction)
{
	//Todo: Set Carry flag (how?) (or don't?)
	int registerIndex = (baseInstruction & 0x0F00) >> 8;
	byte value = baseInstruction & 0x00FF;
	m_V[registerIndex] += value;

	return true;
}
bool Chip8::Interpreter::Instruction_ANNN(opcode baseInstruction)
{
	opcode value = baseInstruction & 0x0FFF;
	m_I = value;

	return true;
}
bool Chip8::Interpreter::Instruction_DXYN(opcode baseInstruction)
{
	auto& renderer = Renderer::GetInstance();

	//Todo: make renderer 1 pixel per frame instead of all at once?
	int xIndex = (baseInstruction & 0x0F00) >> 8;
	int yIndex = (baseInstruction & 0x00F0) >> 4;
	int height = (baseInstruction & 0x000F);

	byte xCoordValue;
	byte yCoordValue = m_V[yIndex] % renderer.GetHeight();
	m_V[0xF] = 0;

	for (int row = 0; row < height; row++)
	{
		xCoordValue = m_V[xIndex] % renderer.GetWidth();

		if (yCoordValue > renderer.GetHeight()) continue;

		byte spriteRow = m_Memory[m_I + row];

		auto bits = ByteToBits(spriteRow);

		for (int pixel = bits.size() - 1; pixel >= 0; pixel--)
		{
			if (xCoordValue > renderer.GetWidth()) continue;

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

	return true;
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