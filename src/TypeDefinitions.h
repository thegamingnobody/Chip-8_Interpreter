#pragma once
#include <vector>

using opcode = unsigned short;
using byte = unsigned char;

namespace Chip8
{
	//Todo: consider: find better solution
	struct ProgramCounterInfo
	{
		opcode CurrentProgramCounter;
		std::vector<opcode> MemoryValuesAtPC;
		int const MemoryValuesSize{ 5 };
	};

	enum class EmulatorStates
	{
		Running,
		Paused
	};
}