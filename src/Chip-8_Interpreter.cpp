#include "Chip-8_Interpreter.h"
#include "TypeDefinitions.h"
#include "Renderer.h"
#include "InputManager.h"
#include <chrono>
#include <thread>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <cassert>
#include "Logger.h"
#include "TimeManager.h"

#define FONTSET_ADDRESS 0x50
#define CHARACTER_HEIGHT 5

Chip8::Interpreter::Interpreter()
	: m_Memory()
	, m_V()
	, m_Stack()
	, m_I(0)
	, m_PC(0x0200)
	, m_DelayTimer()
	, m_SoundTimer()
	, m_SP()
	, m_DrawFlag(true)
	, m_WaitForInput(false)
{
	//Resize memory to 4KB and initialize registers
	m_Memory.resize(4096);
	m_V.resize(16);


	//Initialize Singletons
	int const windowWidth{ 640 };
	int const windowHeight{ 480 };
	//Todo: consider: Read in from config file?
	float windowScale{ 2.0f };

	Renderer::GetInstance().Init(windowWidth, windowHeight, windowScale);
	InputManager::GetInstance().Init();

	Logger::GetInstance().Init(false);
	TimeManager::GetInstance().Init();

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
	//Todo: consider: improve with a std algorithm
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
	auto& timer = TimeManager::GetInstance();
	auto& inputManager = InputManager::GetInstance();

	if (inputManager.IsAnyKeyPressed())
	{
		m_WaitForInput = false;
	}

	//Fetch opcode
	opcode instructionThisCycle = m_Memory[m_PC] << 8 | m_Memory[m_PC + 1];
	if (not m_WaitForInput)
	{
		timer.IncrementCycleCounter();
		m_PC += 2;
	}

	//Decode opcode
	//get the value of the first 4 bits of the opcode to determine the instruction type
	byte instructionType = (instructionThisCycle & 0xF000) >> 8 >> 4;

	//Execute opcode
	bool instructionExecuted{ true };
	
	//Todo: consider: Can this be improved/simplified?
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
		//0x2NNN: Subroutine call
		instructionExecuted = Instruction_2NNN(instructionThisCycle);
		break;
	case 0x3:
		//0x3XNN: skips
		instructionExecuted = Instruction_3XNN(instructionThisCycle);
		break;
	case 0x4:
		//0x4XNN: skips
		instructionExecuted = Instruction_4XNN(instructionThisCycle);
		break;
	case 0x5:
		//0x5XY0: skips
		instructionExecuted = Instruction_5XY0(instructionThisCycle);
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
		//0x8XYN: [VARIOUS] various different instructions based on the value of N
		instructionExecuted = Instruction_8XYN(instructionThisCycle);
		break;
	case 0x9:
		//0x9XY0: skips
		instructionExecuted = Instruction_9XY0(instructionThisCycle);
		break;
	case 0xA:
		//0xANNN: Set index register to value NNN
		instructionExecuted = Instruction_ANNN(instructionThisCycle);
		break;
	case 0xB:
		//0xBNNN: [VARIOUS] jump to address
		instructionExecuted = Instruction_BNNN(instructionThisCycle);
		break;
	case 0xC:
		//0xCXNN: store RNG number in vX
		instructionExecuted = Instruction_CXNN(instructionThisCycle);
		break;
	case 0xD:
		//0xDXYN: Draw/Render
		instructionExecuted = Instruction_DXYN(instructionThisCycle);
		break;
	case 0xE:
		//0xEXNN: [VARIOUS] skips
		instructionExecuted = Instruction_EXNN(instructionThisCycle);
		break;
	case 0xF:
		//0xFXNN: [VARIOUS]	various instructions
		instructionExecuted = Instruction_FXNN(instructionThisCycle);
		break;
	default:
		instructionExecuted = false;
		break;
	}

//Todo: look up how debug/release mode werkt in CMake 
//Todo: make imgui window with failed instructions?
#ifdef DEBUG
	if (instructionExecuted and not(m_WaitForInput))
	{
		logger.SetHexMode();
		logger.Log("[EXEC] instruction executed:", instructionThisCycle);
	}
	else if(not(m_WaitForInput))
	{
		logger.SetHexMode();
		logger.Log("[ERROR] instruction failed:", instructionThisCycle);
	}
#endif // DEBUG
}

bool Chip8::Interpreter::SetkeyStates()
{
	return Chip8::InputManager::GetInstance().ProcessInput();
}

void Chip8::Interpreter::UpdateTimers()
{
	if (m_DelayTimer > 0)
	{
		m_DelayTimer--;
	}
	if (m_SoundTimer > 0)
	{
		m_SoundTimer--;
	}
}

void Chip8::Interpreter::Reset()
{
	Renderer::GetInstance().ClearScreen();
	ClearRegisters();
	ClearRegisters();
	ClearMemory();
	LoadFontset();
	ResetTimers();

	m_PC = 0x0200;
	m_I = 0;
}

void Chip8::Interpreter::ClearMemory()
{
	std::fill(m_Memory.begin(), m_Memory.end(), 0);
}
void Chip8::Interpreter::ClearStack()
{
	//std::fill(m_Stack.begin(), m_Stack.end(), 0);
	m_Stack = std::stack<opcode>();
	m_SP = 0;
}
void Chip8::Interpreter::ClearRegisters()
{
	std::fill(m_V.begin(), m_V.end(), 0);
}
void Chip8::Interpreter::LoadFontset()
{
	m_PC = FONTSET_ADDRESS;

	std::vector<byte> font{ 0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
							0x20, 0x60, 0x20, 0x20, 0x70, // 1
							0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
							0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
							0x90, 0x90, 0xF0, 0x10, 0x10, // 4
							0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
							0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
							0xF0, 0x10, 0x20, 0x40, 0x40, // 7
							0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
							0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
							0xF0, 0x90, 0xF0, 0x90, 0x90, // A
							0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
							0xF0, 0x80, 0x80, 0x80, 0xF0, // C
							0xE0, 0x90, 0x90, 0x90, 0xE0, // D
							0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
							0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	//Todo: consider: improve with a std algorithm
	for (int i = 0; i < font.size(); ++i)
	{
		m_Memory[i + m_PC] = font[i];
	}
}
void Chip8::Interpreter::ResetTimers()
{
	m_DelayTimer = 0;
	m_SoundTimer = 0;
}

bool Chip8::Interpreter::Instruction_0NNN(opcode baseInstruction)
{
	//0x00EE: Return from subroutine
	byte instructionParam = (baseInstruction & 0x00FF);

	switch (instructionParam)
	{
	case 0xE0:
		Renderer::GetInstance().ClearScreen();
		break;
	case 0xEE:
		m_PC = m_Stack.top();
		m_Stack.pop();
		break;
	default:
		return false;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_1NNN(opcode baseInstruction)
{
	m_PC = baseInstruction & 0x0FFF;
	return true;
}
bool Chip8::Interpreter::Instruction_2NNN(opcode baseInstruction)
{
	//0x2NNN: Subroutine call. Jump to address NNN
	opcode value = baseInstruction & 0xFFF;

	m_Stack.push(m_PC);

	m_PC = value;

	return true;
}
bool Chip8::Interpreter::Instruction_3XNN(opcode baseInstruction)
{
	//0x3XNN: skip the next instruction if vX is equal to NN
	byte registerIndex = (baseInstruction & 0x0F00) >> 8;
	byte value = (baseInstruction & 0x00FF);

	assert(registerIndex <= 0xF);

	byte registerValue = m_V[registerIndex];

	if (registerValue == value)
	{
		m_PC += 2;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_4XNN(opcode baseInstruction)
{
	//0x4XNN: skip the next instruction if vX is NOT equal to NN
	byte registerIndex = (baseInstruction & 0x0F00) >> 8;
	byte value = (baseInstruction & 0x00FF);

	assert(registerIndex <= 0xF);

	byte registerValue = m_V[registerIndex];

	if (registerValue != value)
	{
		m_PC += 2;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_5XY0(opcode baseInstruction)
{
	//0x5XY0: skip the next instruction if vX is equal to vY
	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte registerYIndex = (baseInstruction & 0x00F0) >> 4;
	byte subInstruction = (baseInstruction & 0x000F);

	assert(registerXIndex <= 0xF);
	assert(registerYIndex <= 0xF);

	if (subInstruction != 0x0)
	{
		return false;
	}

	byte XValue = m_V[registerXIndex];
	byte YValue = m_V[registerYIndex];

	if (XValue == YValue)
	{
		m_PC += 2;
	}

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
bool Chip8::Interpreter::Instruction_8XYN(opcode baseInstruction)
{
	//Todo: CONFIG
	//	 0x8XYN: various different instructions based on the value of N
	//			- 0x8XY0: vX is set to value of vY
	//			- 0x8XY1: vX is set to the result of a binary OR  between vX and vY, vY is not affected
	//			- 0x8XY2: vX is set to the result of a binary AND between vX and vY, vY is not affected
	//			- 0x8XY3: vX is set to the result of a binary XOR between vX and vY, vY is not affected
	//			- 0x8XY4: vX is set to vX + vY, vY is not affected. set vF to 1 if addition overflows, otherwise set vF to 0
	//			- 0x8XY5: vX is set to vX - vY, vY is not affected. if vX is larger, set vF to 1, otherwise set vF to 0
	//			- 0x8XY7: vX is set to vY - vX, vY is not affected. if vY is larger, set vF to 1, otherwise set vF to 0
	//			- 0x8XY6: 1) (configurable) vX is set to vY 
	//					  2) shift vX one bit to the right
	//					  3) set vF to to the value of the bit that was shifted out
	//			- 0x8XYE: 1) (configurable) vX is set to vY 
	//					  2) shift vY one bit to the left
	//					  3) set vF to to the value of the bit that was shifted out
	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte registerYIndex = (baseInstruction & 0x00F0) >> 4;
	byte subInstruction = (baseInstruction & 0x000F);

	assert(registerXIndex <= 0xF);
	assert(registerYIndex <= 0xF);

	byte XValue = m_V[registerXIndex];
	byte YValue = m_V[registerYIndex];

	switch (subInstruction)
	{
	case 0x0:
		m_V[registerXIndex] = YValue;
		break;
	case 0x1:
		m_V[registerXIndex] = (XValue | YValue);
		break;
	case 0x2:
		m_V[registerXIndex] = (XValue & YValue);
		break;
	case 0x3:
		m_V[registerXIndex] = (XValue ^ YValue);
		break;
	case 0x4:
		{	
			opcode result = (XValue + YValue);
			m_V[registerXIndex] = (result & 0x00FF);
			if ((result & 0xFF00))
			{
				m_V[0xF] = 0x01;
			}
			else
			{
				m_V[0xF] = 0x00;
			}
		}
		break;
	case 0x5:
		{	
			opcode result = (XValue - YValue);
			m_V[registerXIndex] = (result & 0x00FF);
			if (result & 0xFF00)
			{
				m_V[0xF] = 0x00;
			}
			else
			{
				m_V[0xF] = 0x01;
			}
		}
		break;
	case 0x6:
		{
			//m_V[registerXIndex] = YValue;
			byte lostBit = (XValue & 0x01);

			XValue = XValue >> 1;
			m_V[registerXIndex] = XValue;

			m_V[0xF] = lostBit;
		}
		break;
	case 0x7:
		{	
			opcode result = (YValue - XValue);
			m_V[registerXIndex] = (result & 0x00FF);
			if (result & 0xFF00)
			{
				m_V[0xF] = 0x00;
			}
			else
			{
				m_V[0xF] = 0x01;
			}
		}
		break;
	case 0xE:
		{
			byte lostBit = (YValue & 0x80);
			m_V[registerXIndex] = YValue;

			XValue = YValue << 1;
			m_V[registerXIndex] = XValue;

			m_V[0xF] = lostBit >> 7;
		}
		break;
	default:
		return false;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_9XY0(opcode baseInstruction)
{
	//0x9XY0: skip the next instruction if vX is NOT equal to vY
	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte registerYIndex = (baseInstruction & 0x00F0) >> 4;
	byte subInstruction = (baseInstruction & 0x000F);

	assert(registerXIndex <= 0xF);
	assert(registerYIndex <= 0xF);

	if (subInstruction != 0x0)
	{
		return false;
	}

	byte XValue = m_V[registerXIndex];
	byte YValue = m_V[registerYIndex];

	if (XValue != YValue)
	{
		m_PC += 2;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_ANNN(opcode baseInstruction)
{
	opcode value = baseInstruction & 0x0FFF;
	m_I = value;

	return true;
}
bool Chip8::Interpreter::Instruction_BNNN(opcode baseInstruction)
{
	opcode jumpValue = (baseInstruction & 0x0FFF);
	byte xIndex = 0;
	//byte xIndex = (baseInstruction & 0x0F00) >> 8;

	byte registerValue = m_V[xIndex];
	m_PC = jumpValue + registerValue;
	//Todo: CONFIG
	// 1) originally
	//Todo: 	0xBNNN: jump to address (NNN + value in v0)
	// 2) later on
	//Todo:		0xBXNN: jump to address (XNN + value in vX)
	return true;
}
bool Chip8::Interpreter::Instruction_CXNN(opcode baseInstruction)
{
	//0xCXNN: 1) generate random number
	//				2) binary AND random number with NN
	//				3) store result of binary AND in vX
	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte mask = (baseInstruction & 0x00FF);
	byte randomNumber{ static_cast<byte>(std::rand() & 0xFF) };

	byte result = randomNumber & mask;
	m_V[registerXIndex] = result;
	return true;
}
bool Chip8::Interpreter::Instruction_DXYN(opcode baseInstruction)
{
	auto& renderer = Renderer::GetInstance();

	int xIndex = (baseInstruction & 0x0F00) >> 8;
	int yIndex = (baseInstruction & 0x00F0) >> 4;
	int height = (baseInstruction & 0x000F);

	byte xCoordValue;
	byte yCoordValue = m_V[yIndex] % renderer.GetViewportHeight();
	m_V[0xF] = 0;

	for (int row = 0; row < height; row++)
	{
		if (yCoordValue >= renderer.GetViewportHeight()) continue;

		xCoordValue = m_V[xIndex] % renderer.GetViewportWidth();

		byte spriteRow = m_Memory[m_I + row];
		int size = sizeof(spriteRow) * 8;

		for (int pixel = 0; pixel < size; pixel++)
		{
			if (xCoordValue >= renderer.GetViewportWidth()) continue;

			if (spriteRow & (0x80 >> pixel))
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

	m_DrawFlag = true;

	return true;
}
bool Chip8::Interpreter::Instruction_EXNN(opcode baseInstruction)
{
	//0xEX9E: skip next instruction if key corresponding to value in vX is pressed
	//0xEXA1: skip next instruction if key corresponding to value in vX is NOT pressed
	//valid key values: 0 - F
	auto& inputManager = InputManager::GetInstance();

	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte subInstruction = (baseInstruction & 0x00FF);
	
	assert(registerXIndex <= 0xF);

	byte XValue = m_V[registerXIndex];

	switch (subInstruction)
	{
	case 0xA1:
		if (not inputManager.IsKeyPressed(XValue))
		{
			m_PC += 2;
		}
		break;
	case 0x9E:
		if (inputManager.IsKeyPressed(XValue))
		{
			m_PC += 2;
		}
		break;
	default:
		break;
	}

	return true;
}
bool Chip8::Interpreter::Instruction_FXNN(opcode baseInstruction)
{
	auto& logger = Logger::GetInstance();

	byte registerXIndex = (baseInstruction & 0x0F00) >> 8;
	byte subInstruction = (baseInstruction & 0x00FF);

	assert(registerXIndex <= 0xF);

	byte XValue = m_V[registerXIndex];

	switch (subInstruction)
	{
	case 0x07:
		//0xFX07: vX is set to current value of delay timer
		m_V[registerXIndex] = m_DelayTimer;
		break;
	case 0x15:
		//0xFX15: delay timer is set to vX
		m_DelayTimer = XValue;
		break;
	case 0x18:
		//0xFX18: sound timer is set to vX
		m_SoundTimer = XValue;
		break;
	case 0x1E:
		//0xFX1E: index register is set to (index register + vX) => I += vX
		m_I += XValue;
		break;
	case 0x0A:
		////Todo: 0xFX0A: stops instruction execution untill a key is pressed. Decrement PC unless a key is pressed. delay and sound timers should still decrease. When key is pressed, its value is placed in vX and executions continues
		////				on OG machine, key is registered upon release of key
		//if (not m_WaitForInput)
		//{
		//	m_WaitForInput = true;
		//	logger.Log("Waiting for input...\n");
		//}
		//
		m_WaitForInput = true;
		return true;
		break;
	case 0x29:
		//0xFX29: set index register to address of character 
		m_I = FONTSET_ADDRESS + (CHARACTER_HEIGHT * XValue);
		break;
	case 0x33:
		//0xFX33: stores 3 digit decimal version of value in vX at address in index register.							example: if vX is 156, I will be 1, I + 1 will be 5 and I + 2 will be 6
		{
			byte hundreds = XValue / 100;
			byte tens = (XValue - hundreds * 100) / 10;
			byte singleDigit = (XValue - hundreds * 100 - tens * 10);

			m_Memory[m_I + 0] = hundreds;
			m_Memory[m_I + 1] = tens;
			m_Memory[m_I + 2] = singleDigit;
		}
		break;
	case 0x55:
		//Todo: CONFIG
		//0xFX55: store values of registers v0 to vX (inclusive) sequeltially starting at address in index register,	example: I will be v0, I + 1 will be v1, I + 2 will be v2 etc.
		for (int i = 0; i <= registerXIndex; i++)
		{
			//Option 1
			m_Memory[m_I + i] = m_V[i];

			////Option 2
			//m_Memory[m_I] = m_V[i];
			//m_I++;
		}
		break;
	case 0x65:
		//Todo: CONFIG
		//0xFX65: reverse of 0xFX55, stores values from I to I + X in v0 t vX. congif: does I increment or does it use a temp value
		for (int i = 0; i <= registerXIndex; i++)
		{
			//Option 1
			m_V[i] = m_Memory[m_I + i];
		
			////Option 2
			//m_V[i] = m_Memory[m_I];
			//m_I++;
		}
		break;
	default:
		return false;
	}

	return true;
}

Chip8::ProgramCounterInfo Chip8::Interpreter::CreateProgramCounterInfo()
{
	Chip8::ProgramCounterInfo result;

	result.CurrentProgramCounter = m_PC;

	for (int i = 0; i < result.MemoryValuesSize; i++)
	{
		opcode instruction = m_Memory[(m_PC + ((i - 2) * 2))] << 8 | m_Memory[(m_PC + ((i - 2) * 2)) + 1];
		result.MemoryValuesAtPC.emplace_back(instruction);
	}

	result.RegisterValues = m_V;

	return result;
}