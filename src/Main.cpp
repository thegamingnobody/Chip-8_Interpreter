#include "Chip-8_Interpreter.h"

int main()
{
	Chip8::Interpreter interpreter{};
	interpreter.Run();
	return 0;
}