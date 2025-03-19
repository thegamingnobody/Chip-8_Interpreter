#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>

int main()
{
	Chip8::Interpreter interpreter{};

	std::vector<std::string> gameNames{ "0-chip8-logo.ch8", "1-ibm-logo.ch8", "2-corax+.ch8", "test_opcode.ch8"};
	int const gameIndex{ 2 };

	interpreter.LoadGame(gameNames[gameIndex]);

	bool continueRunning{ true };
	while (continueRunning)
	{
		interpreter.EmulateCycle();
		interpreter.UpdateRender();
		continueRunning = interpreter.SetkeyStates();
	}

	return 0;
}