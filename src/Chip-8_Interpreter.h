// Chip-8_Interpreter.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "TypeDefinitions.h"
#include <vector>
#include <string>

namespace Chip8
{
	class Interpreter
	{
	public:
		explicit Interpreter();
		~Interpreter();
		//void Run();

		Interpreter(const Interpreter& other) = delete;
		Interpreter(Interpreter&& other) = delete;
		Interpreter& operator=(const Interpreter& other) = delete;
		Interpreter& operator=(Interpreter&& other) = delete;
		
		void LoadGame(const std::string& gameName);

		void EmulateCycle();

		void UpdateRender();

		bool SetkeyStates();

		void Reset();
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
		bool Instruction_6XNN(opcode baseInstruction);
		bool Instruction_7XNN(opcode baseInstruction);
		bool Instruction_ANNN(opcode baseInstruction);
		bool Instruction_DXYN(opcode baseInstruction);


		std::vector<bool> ByteToBits(byte byteValue) const;

		//Main memory
		std::vector<byte> m_Memory;

		//Registers
		std::vector<byte> m_V;
		
		//Keys
		//todo: is bool correct type?
		std::vector<bool> m_Keys;

		//Stack 
		std::vector<byte> m_Stack;

		//Index register
		opcode m_I;

		//Program counter
		opcode m_PC;

		//Timers & Stack Pointer
		byte m_DelayTimer;
		byte m_SoundTimer;
		byte m_SP;

		bool m_DrawFlag;
	};

}