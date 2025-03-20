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

	bool continueRunning{ true };
	int instructionsPerCycle{ timer.GetInstructionPerFrame() };
	while (continueRunning)
	{
		timer.UpdateTime();

		bool shouldUpdateGame{ timer.ShouldUpdateTimers() };


		continueRunning = interpreter.SetkeyStates();

		for (int i = 0; i < instructionsPerCycle; i++)
		{
			interpreter.EmulateCycle(shouldUpdateGame);
		}
		interpreter.UpdateRender(shouldUpdateGame);

		//Todo: Allow faster emulation
		//std::this_thread::sleep_for(std::chrono::milliseconds(500));

		auto sleepTime{ timer.GetSleepTime() };
		//std::cout << sleepTime.count() << " Microseconds\n";
		std::this_thread::sleep_for(sleepTime);
	}

	return 0;
}