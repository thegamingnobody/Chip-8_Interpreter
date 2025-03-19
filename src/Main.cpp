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
	int const gameIndex{ 5 };

	interpreter.LoadGame(gameNames[gameIndex]);

	bool continueRunning{ true };

	int const targetInstructionsPerSecond{ 700 };
	long long const msPerFrame = 1000 / targetInstructionsPerSecond;

	auto lastTime = std::chrono::high_resolution_clock::now();
	//int cyclesExecuted{};
	//float totalTime{};
	while (continueRunning)
	{
		auto const currentTime = std::chrono::high_resolution_clock::now();
		float const deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
		lastTime = currentTime;

		continueRunning = interpreter.SetkeyStates();
		interpreter.EmulateCycle();
		interpreter.UpdateRender();
		
		auto const sleepTime = std::chrono::milliseconds(msPerFrame) - (std::chrono::high_resolution_clock::now() - currentTime);
		
		std::this_thread::sleep_for(sleepTime);

		//cyclesExecuted++;
		//totalTime += deltaTime; 
		//std::cout << "average cycles per second: " << (cyclesExecuted / totalTime) << "\n";
	}

	return 0;
}