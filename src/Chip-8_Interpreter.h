// Chip-8_Interpreter.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "TypeDefinitions.h"
#include <vector>

namespace Chip8
{
	class Interpreter
	{
	public:
		explicit Interpreter();
		~Interpreter();
		void Run();

		Interpreter(const Interpreter& other) = delete;
		Interpreter(Interpreter&& other) = delete;
		Interpreter& operator=(const Interpreter& other) = delete;
		Interpreter& operator=(Interpreter&& other) = delete;
	
	private:
		//Index register
		unsigned short m_I;

		//Program counter
		unsigned short m_PC;

		//Main memory
		std::vector<byte> m_Memory;

		//Registers
		std::vector<byte> m_V;

	};

}