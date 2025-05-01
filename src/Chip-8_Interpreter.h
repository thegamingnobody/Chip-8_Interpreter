#pragma once

#include "Singleton.h"
#include "TypeDefinitions.h"
#include <vector>
#include <string>
#include <stack>

namespace Chip8
{
	class Interpreter : public Singleton<Interpreter>
	{
	public:
		void Init();
		void Destroy();

		void LoadGame(const std::string& gameName);

		bool EmulateCycle();

		bool SetkeyStates();

		void UpdateTimers();

		void Reset();

		Chip8::EmulatorStates RenderImgui(std::string windowName, EmulatorStates emulatorState) const;

		bool IsWaitingForInput() const { return m_WaitForInput; }
		bool GetDrawFlag() const { return m_DrawFlag; }

		void SetDrawFlag(bool newFlag) { m_DrawFlag = newFlag; }
	private:
		//Reset functions
		void ClearMemory();
		void ClearStack();
		void ClearRegisters();
		void LoadFontset();
		void ResetTimers();

		//Instruction functions
		bool Instruction_0NNN(opcode baseInstruction);
		bool Instruction_1NNN(opcode baseInstruction);
		bool Instruction_2NNN(opcode baseInstruction);
		bool Instruction_3XNN(opcode baseInstruction);
		bool Instruction_4XNN(opcode baseInstruction);
		bool Instruction_5XY0(opcode baseInstruction);
		bool Instruction_6XNN(opcode baseInstruction);
		bool Instruction_7XNN(opcode baseInstruction);
		bool Instruction_8XYN(opcode baseInstruction);
		bool Instruction_9XY0(opcode baseInstruction);
		bool Instruction_ANNN(opcode baseInstruction);
		bool Instruction_BNNN(opcode baseInstruction);
		bool Instruction_CXNN(opcode baseInstruction);
		bool Instruction_DXYN(opcode baseInstruction);
		bool Instruction_EXNN(opcode baseInstruction);
		bool Instruction_FXNN(opcode baseInstruction);

		//Main memory
		std::vector<byte> m_Memory{};

		//Registers
		std::vector<byte> m_V{};

		//Stack 
		std::stack<opcode> m_Stack{};

		//Index register
		opcode m_I{};

		//Program counter
		opcode m_PC{ PROGRAM_COUNTER_START };

		//Timers & Stack Pointer
		byte m_DelayTimer{};
		byte m_SoundTimer{};
		byte m_SP{};

		bool m_DrawFlag{ true };
		bool m_WaitForInput{ false };
	};

}