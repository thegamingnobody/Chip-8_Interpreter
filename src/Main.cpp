#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>

int main()
{
	Chip8::Interpreter interpreter{};

	std::vector<std::string> gameNames{ "IBM_Logo.ch8", "1-chip8-logo.ch8" };
	int const gameIndex{ 1 };

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