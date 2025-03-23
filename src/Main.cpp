#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>
#include <chrono>
#include <thread>
#include <iostream>
#include "TimeManager.h"

int main()
{
	Chip8::Interpreter interpreter{};

	auto& timer = Chip8::TimeManager::GetInstance();

	std::vector<std::string> gameNames{ "0-chip8-logo.ch8", "1-ibm-logo.ch8", "2-corax+.ch8", "3-flags.ch8", "4-quirks.ch8", "5-keypad.ch8", "test_opcode.ch8" };
	int const gameIndex{ 5 };

	interpreter.LoadGame(gameNames[gameIndex]);

	int instructionsPerCycle{ timer.GetInstructionPerFrame() };
	bool continueRunning{ true };
	Chip8::EmulatorStates emulatorState{ Chip8::EmulatorStates::Running };
	while (continueRunning)
	{
		timer.UpdateTime((emulatorState == Chip8::EmulatorStates::Paused));
		bool shouldUpdateGame{ timer.ShouldUpdateTimers() };

		continueRunning = interpreter.SetkeyStates();

		switch (emulatorState)
		{
		case Chip8::EmulatorStates::Running:
			for (int i = 0; i < instructionsPerCycle; i++)
			{
				interpreter.EmulateCycle(shouldUpdateGame);
			}
			break;
		case Chip8::EmulatorStates::Paused:
			//We want to handle/update imgui, but not the game itself
			shouldUpdateGame = false;
			break;
		}

		auto pcInfo = interpreter.CreateProgramCounterInfo();
		//Imgui handling is currently done in the Renderer render function, not sure how to split this
		//Todo: find way to improve this
		emulatorState = Chip8::Renderer::GetInstance().Render(shouldUpdateGame, pcInfo, emulatorState);
		Chip8::Renderer::GetInstance().Update();

		//Todo: Allow faster emulation
		auto sleepTime{ timer.GetSleepTime() };
		std::this_thread::sleep_for(sleepTime);
	}

	return 0;
}