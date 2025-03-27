#pragma once
#include <vector>

using opcode = unsigned short;
using byte = unsigned char;

#define PROGRAM_COUNTER_START 0x0200
#define FONTSET_ADDRESS 0x50
#define CHARACTER_HEIGHT 5
#define MEMORY_SIZE 4096
#define NR_OF_REGISTERS 16
#define WINDOW_WIDTH_BASE 640
#define WINDOW_HEIGHT_BASE 480

namespace Chip8
{
	enum class EmulatorStates
	{
		Running,
		Paused,
		Step
	};
}