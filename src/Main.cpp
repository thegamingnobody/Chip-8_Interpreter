#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>
#include <chrono>
#include <thread>
#include <iostream>

int main()
{
	Chip8::Interpreter interpreter{};

	std::vector<std::string> gameNames{ "0-chip8-logo.ch8", "1-ibm-logo.ch8", "2-corax+.ch8", "3-flags.ch8", "4-quirks.ch8", "5-keypad.ch8", "test_opcode.ch8"};
	int const gameIndex{ 2 };

	interpreter.LoadGame(gameNames[gameIndex]);

	bool continueRunning{ true };

	int const targetInstructionsPerSecond{ 700 };
	long long const instructionMsPerFrame = 1000 / targetInstructionsPerSecond;

	int const targetTimerUpdatesPerSecond{ 60 };
	double const msPerTimerUpdate = 1 / targetTimerUpdatesPerSecond;
	float timersTimer{};
	bool updateTimers{ false };

	auto lastTime = std::chrono::high_resolution_clock::now();
	//int cyclesExecuted{};
	//float totalTime{};
	while (continueRunning)
	{
		auto const currentTime = std::chrono::high_resolution_clock::now();
		float const deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;
		timersTimer += deltaTime;

		if (timersTimer >= msPerTimerUpdate)
		{
			timersTimer -= msPerTimerUpdate;
			updateTimers = true;
		}
		else
		{
			updateTimers = false;
		}

		continueRunning = interpreter.SetkeyStates();
		interpreter.EmulateCycle(updateTimers);
		interpreter.UpdateRender();
		
		auto const sleepTime = std::chrono::milliseconds(instructionMsPerFrame) - (std::chrono::high_resolution_clock::now() - currentTime);
		
		std::this_thread::sleep_for(sleepTime);

		//cyclesExecuted++;
		//totalTime += deltaTime; 
		//std::cout << "average cycles per second: " << (cyclesExecuted / totalTime) << "\n";
	}

	return 0;
}