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
#include "QuirkManager.h"
#include <openssl/sha.h>
#include <SDL_mixer.h>
#include <AudioManager.h>


void Chip8::Interpreter::Init()
{
	//Resize memory to 4KB and initialize registers
	m_Memory.resize(MEMORY_SIZE);
	m_V.resize(NR_OF_REGISTERS);
}
void Chip8::Interpreter::Destroy()
{
	Renderer::GetInstance().Destroy();
	SDL_Quit();
}

void Chip8::Interpreter::LoadGame(const std::string& gamePath)
{
	Reset();

	std::ifstream input(gamePath, std::ios::binary | std::ios::ate);
	if (!input.is_open())
	{
		std::cerr << "Failed to open file: " << gamePath << "\n";
		return;
	}

	//Get file size
	std::streamsize fileSize = input.tellg();
	input.seekg(0, std::ios::beg);

	assert(fileSize < m_Memory.size());

	//Copy input ifstream into m_Memory
	//Todo: consider: improve with a std algorithm
    std::vector<unsigned char> buffer(fileSize);
	for (int i = 0; i < fileSize; ++i)
	{
		m_Memory[i + m_PC] = input.get();
		buffer[i] = m_Memory[i + m_PC];
	}

	Logger::GetInstance().Log("Game loaded successfully");	

	//*-----------------*
	//|	  Create hash   |
	//*-----------------*
	unsigned char hash[SHA_DIGEST_LENGTH]; // == 20
	input.read(reinterpret_cast<char*>(buffer.data()), fileSize);

	SHA1(buffer.data(), fileSize, hash);

	std::stringstream stream;
	for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
	{
		stream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(hash[i]);
	}

	Chip8::QuirkManager::GetInstance().LoadGameQuirks(stream.str());
}

bool Chip8::Interpreter::EmulateCycle()
{
	auto& logger = Logger::GetInstance();
	auto& renderer = Renderer::GetInstance();
	auto& timer = TimeManager::GetInstance();

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

	return m_DrawFlag;
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
		if (AudioManager::GetInstance().IsSoundEnabled() == false)
		{
			AudioManager::GetInstance().StartSound();
		}
	}
	else
	{
		if (AudioManager::GetInstance().IsSoundEnabled())
		{
			AudioManager::GetInstance().StopSound();
		}
	}
}

Chip8::EmulatorStates Chip8::Interpreter::RenderImgui(std::string windowName, EmulatorStates emulatorState) const
{
	Chip8::EmulatorStates returnState{ emulatorState };

	if (windowName == "Memory")
	{
		ImGui::Begin(windowName.c_str(), nullptr);
			if (ImGui::Button("Reset"))
			{
				returnState = Chip8::EmulatorStates::Reset;
				ImGui::End();
				return returnState;
			}

			if (emulatorState == Chip8::EmulatorStates::Running and ImGui::Button("Pause"))
			{
				returnState = Chip8::EmulatorStates::Paused;
			}
			else if (emulatorState == Chip8::EmulatorStates::Paused and ImGui::Button("Resume"))
			{
				returnState = Chip8::EmulatorStates::Running;
			}
			ImGui::SameLine();
			if (ImGui::Button("Step"))
			{
				returnState = Chip8::EmulatorStates::Step;
			}
			ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
			if (ImGui::BeginTable("Memory", 3, flags))
			{
				ImGui::TableSetupColumn("PC");
				ImGui::TableSetupColumn("Address");
				ImGui::TableSetupColumn("Value");
				ImGui::TableHeadersRow();

				for (int row = 0; row < 5; row++)
				{
					ImGui::TableNextRow();

					//White text is default
					ImVec4 textColor{ 1.0f, 1.0f, 1.0f, 1.0f };
					//if ((pcInfo.MemoryValuesAtPC[row] & 0xF000) == 0xD000)
					//{
					//	//Instruction is a draw, color is red
					//	textColor = ImVec4(0.85f, 0.3f, 0.3f, 1.0f);
					//}

					int relativeIndex{ (2 * (row - 2)) };
					opcode memoryValue = m_Memory[m_PC + relativeIndex] << 8 | m_Memory[m_PC + relativeIndex + 1];
					for (int column = 0; column < 3; column++)
					{
						ImGui::TableSetColumnIndex(column);
						switch (column)
						{
						case 0:
							if (row == 2)
							{
								ImGui::Text("=>");
							}
							else
							{
								ImGui::Text("  ");
							}
							break;
						case 1:
						{
							std::stringstream stream;
							stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << m_PC + relativeIndex;
							ImGui::TextColored(textColor, stream.str().c_str());
						}
						break;
						case 2:
						{
							std::stringstream stream;
							stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << memoryValue;
							ImGui::TextColored(textColor, stream.str().c_str());
						}
						break;
						}
					}
				}
				ImGui::EndTable();

				if (ImGui::BeginTable("Registers", 4, flags))
				{
					//print register values
					for (int i = 0; i < 4; i++)
					{
						ImGui::TableNextRow();

						for (int j = 0; j < 4; j++)
						{
							ImGui::TableSetColumnIndex(j);
							std::stringstream stream;
							stream << "v" << std::uppercase << std::hex << (i + j) << ":" << static_cast<int>(m_V[i + j]);
							ImGui::Text(stream.str().c_str());
						}
					}
				}
				ImGui::EndTable();

				std::stringstream stream;
				stream << "delay timer: " << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(m_DelayTimer);
				ImGui::Text(stream.str().c_str());
				stream = std::stringstream();
				stream << "sound timer: " << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(m_SoundTimer);
				ImGui::Text(stream.str().c_str());

			}
		ImGui::End();
	}
	else if (windowName == "Game Info")
	{
		ImGui::Begin(windowName.c_str(), nullptr);			
			auto gameInfo = Chip8::QuirkManager::GetInstance().GetGameInfo();
			auto& gameQuirks = Chip8::QuirkManager::GetInstance().GetQuirks();

			ImGui::Text("Title: %s", gameInfo.title.c_str());
			ImGui::Text("Author: ");

			for (int author = 0; author < gameInfo.authors.size(); author++)
			{
				ImGui::SameLine();
				ImGui::Text("%s", gameInfo.authors[author].c_str());
				if (author != gameInfo.authors.size() - 1)
				{
					ImGui::Text(", ");
				}
			}

			ImGui::TextWrapped("Description: %s", gameInfo.description.c_str());
			//ImGui::Text("Description: %s", gameInfo.description.c_str());
			
			ImGui::Checkbox("Shift Quirk", &gameQuirks.shiftQuirk);
			ImGui::Checkbox("Load Store Quirk Increment", &gameQuirks.loadStoreQuirkIncrement);
			ImGui::Checkbox("Load Store Quirk Unchanged", &gameQuirks.loadStoreQuirkUnchanged);
			ImGui::Checkbox("Wrap Quirk", &gameQuirks.wrapQuirk);
			ImGui::Checkbox("Jump Quirk", &gameQuirks.jumpQuirk);
			ImGui::Checkbox("Vblank Quirk", &gameQuirks.vblankQuirk);
			ImGui::Checkbox("Vf Reset Quirk", &gameQuirks.vFResetQuirk);

			if (ImGui::Button("Load Game"))
			{
				returnState = Chip8::EmulatorStates::Loading_Game;
			}
		ImGui::End();
	}
	
	return returnState;
}

void Chip8::Interpreter::Reset()
{
	LoadFontset();
	ClearRegisters();
	ResetTimers();
	Renderer::GetInstance().ClearScreen();
	//ClearMemory();

	m_PC = PROGRAM_COUNTER_START;
	m_I = 0;
}
void Chip8::Interpreter::ClearMemory()
{
	std::fill(m_Memory.begin(), m_Memory.end(), 0);
}
void Chip8::Interpreter::ClearStack()
{
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
	opcode instructionParam = (baseInstruction & 0x0FFF);

	switch (instructionParam)
	{
	case 0x00E0:
		Renderer::GetInstance().ClearScreen();
		m_DrawFlag = true;
		break;
	case 0x00EE:
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
	int registerIndex = (baseInstruction & 0x0F00) >> 8;
	byte value = baseInstruction & 0x00FF;
	m_V[registerIndex] += value;

	return true;
}
bool Chip8::Interpreter::Instruction_8XYN(opcode baseInstruction)
{
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

	bool vFResetQuirk = Chip8::QuirkManager::GetInstance().GetVfResetQuirk();

	switch (subInstruction)
	{
	case 0x0:
		m_V[registerXIndex] = YValue;
		break;
	case 0x1:
		m_V[registerXIndex] = (XValue | YValue);
		if (vFResetQuirk)
		{
			m_V[0x0F] = 0;
		}
		break;
	case 0x2:
		m_V[registerXIndex] = (XValue & YValue);
		if (vFResetQuirk)
		{
			m_V[0x0F] = 0;
		}
		break;
	case 0x3:
		m_V[registerXIndex] = (XValue ^ YValue);
		if (vFResetQuirk)
		{
			m_V[0x0F] = 0;
		}
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
			bool shiftQuirk = Chip8::QuirkManager::GetInstance().GetShiftQuirk();

			if (!shiftQuirk)
			{
				m_V[registerXIndex] = YValue;
				XValue = m_V[registerXIndex];
			}
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
			bool shiftQuirk = Chip8::QuirkManager::GetInstance().GetShiftQuirk();

			if (!shiftQuirk)
			{
				m_V[registerXIndex] = YValue;
				XValue = m_V[registerXIndex];
			}
			byte lostBit = (XValue & 0x80);

			XValue = XValue << 1;
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
	bool jumpQuirk = Chip8::QuirkManager::GetInstance().GetJumpQuirk();
	if (jumpQuirk)
	{
		xIndex = (baseInstruction & 0x0F00) >> 8;
	}

	byte registerValue = m_V[xIndex];
	m_PC = jumpValue + registerValue;

	// 1) originally
	//0xBNNN: jump to address (NNN + value in v0)
	// 2) later on
	//0xBXNN: jump to address (XNN + value in vX)
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
	bool wrapQuirk = Chip8::QuirkManager::GetInstance().GetWrapQuirk();

	int xIndex = (baseInstruction & 0x0F00) >> 8;
	int yIndex = (baseInstruction & 0x00F0) >> 4;
	int height = (baseInstruction & 0x000F);

	byte baseCoordX = m_V[xIndex] % VIEWPORT_WIDTH_BASE;
	byte baseCoordY = m_V[yIndex] % VIEWPORT_HEIGHT_BASE;
	m_V[0xF] = 0;

	for (int row = 0; row < height; row++)
	{
		auto pixelY = (baseCoordY + row);

		if (wrapQuirk)
		{
			pixelY = pixelY % VIEWPORT_HEIGHT_BASE;
		}

		byte spriteRow = m_Memory[m_I + row];

		for (int pixel = 0; pixel < 8; pixel++)
		{
			auto pixelX = (baseCoordX + pixel);

			if (wrapQuirk)
			{
				pixelX = pixelX % VIEWPORT_WIDTH_BASE;
			}
			else if (pixelY >= VIEWPORT_HEIGHT_BASE or pixelX >= VIEWPORT_WIDTH_BASE)
				continue;

			if (spriteRow & (0x80 >> pixel))
			{
				if (renderer.IsPixelOn(pixelX, pixelY))
				{
					m_V[0xF] = 1;
				}
				renderer.TogglePixel(pixelX, pixelY);
			}
		}
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
	auto& inputManager = InputManager::GetInstance();

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
		//0xFX0A: stops instruction execution untill a key is pressed. Decrement PC unless a key is pressed. delay and sound timers should still decrease. When key is pressed, its value is placed in vX and executions continues
		//			on OG machine, key is registered upon release of key
		
		//Check if already in wating mode
		if (not m_WaitForInput)
		{
			m_PC -= 2;
			m_WaitForInput = true;
			return true;
		}
		
		//Check if any key is being pressed
		if (inputManager.isKeyReleasedThisFrame())
		{
			m_WaitForInput = false;
			for (int key = 0; key < inputManager.GetNumberOfKeys(); key++)
			{
				//set value of pressed key in vX
				//Todo: Set value of released key in vX
				if (inputManager.IsKeyReleased(key))
				{
					m_V[registerXIndex] = static_cast<byte>(key & 0xFF);
					m_PC += 2;
					return true;
				}
			}
		}
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
		{
			//0xFX55: store values of registers v0 to vX (inclusive) sequeltially starting at address in index register,	example: I will be v0, I + 1 will be v1, I + 2 will be v2 etc.
			bool loadStoreQuirkIncrement = Chip8::QuirkManager::GetInstance().GetLoadStoreQuirkIncrement();
			bool loadStoreQuirkUnchanged = Chip8::QuirkManager::GetInstance().GetLoadStoreQuirkUnchanged();

			for (int i = 0; i <= registerXIndex; i++)
			{
				if (loadStoreQuirkUnchanged)
				{
					m_Memory[m_I + i] = m_V[i];
				}
				else
				{
					m_Memory[m_I] = m_V[i];
					m_I++;
				}


			}
			if (!loadStoreQuirkUnchanged and loadStoreQuirkIncrement)
			{
				m_I--;
			}

		}
		break;
	case 0x65:
		{
			//0xFX65: reverse of 0xFX55, stores values from I to I + X in v0 t vX. congif: does I increment or does it use a temp value
			bool loadStoreQuirkIncrement = Chip8::QuirkManager::GetInstance().GetLoadStoreQuirkIncrement();
			bool loadStoreQuirkUnchanged = Chip8::QuirkManager::GetInstance().GetLoadStoreQuirkUnchanged();
		
			for (int i = 0; i <= registerXIndex; i++)
			{
				if (loadStoreQuirkUnchanged)
				{
					m_V[i] = m_Memory[m_I + i];
				}
				else
				{
					m_V[i] = m_Memory[m_I];
					m_I++;
				}
			}
			if (!loadStoreQuirkUnchanged and loadStoreQuirkIncrement)
			{
				m_I--;
			}
		}
		break;
	default:
		return false;
	}

	return true;
}