// Chip-8_Interpreter.h : Include file for standard system include files,
// or project specific include files.

#pragma once

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

		const int m_WidthBase = 640;
		const int m_HeightBase = 480;
	};

}