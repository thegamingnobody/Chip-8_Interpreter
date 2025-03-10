#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>

int main()
{
	Chip8::Interpreter interpreter{};

	std::string gameName{ "IBM_Logo.ch8" };
	interpreter.LoadGame(gameName);

	bool continueRunning{ true };
	while (continueRunning)
	{
		interpreter.EmulateCycle();
		interpreter.UpdateRender();
		continueRunning = interpreter.SetkeyStates();
	}

	return 0;
}