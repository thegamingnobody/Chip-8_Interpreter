#include <Windows.h>
#include <commdlg.h>
#include "Chip-8_Interpreter.h"
#include <InputManager.h>
#include <Renderer.h>
#include <chrono>
#include <thread>
#include <iostream>
#include "TimeManager.h"
#include "Logger.h"
#include <filesystem>


#define GAME_INDEX 7

std::string OpenFileDialog();

int main()
{
	std::srand(time(NULL));
	//Chip8::Interpreter interpreter{};
	auto& timer = Chip8::TimeManager::GetInstance();
	auto& renderer = Chip8::Renderer::GetInstance();
	auto& interpreter = Chip8::Interpreter::GetInstance();
	auto& inputManager = Chip8::InputManager::GetInstance();

	float windowScale{ 2.0f };
	interpreter.Init();
	renderer.Init(windowScale);
	inputManager.Init();
	Chip8::Logger::GetInstance().Init(false);
	timer.Init();

	//changed the numbering of the test roms to match the index
	std::vector<std::string> gameNames{ "0-chip8-logo.ch8", "1-ibm-logo.ch8", "2-corax+.ch8", "3-flags.ch8", "4-quirks.ch8", "5-keypad.ch8", "6-beep.ch8", "Space Invaders [David Winter].ch8"};
	std::filesystem::path gamePath{ "../../../roms/" + gameNames[GAME_INDEX] };
	interpreter.LoadGame(std::filesystem::absolute(gamePath).string());

	bool continueRunning{ true };
	//Chip8::EmulatorStates emulatorState{ Chip8::EmulatorStates::Paused };
	Chip8::EmulatorStates emulatorState{ Chip8::EmulatorStates::Reset };
	while (continueRunning)
	{
		timer.UpdateTime((emulatorState == Chip8::EmulatorStates::Paused));

		continueRunning = interpreter.SetkeyStates();

		switch (emulatorState)
		{
		case Chip8::EmulatorStates::Running:
			for (int i = 0; i < timer.GetInstructionPerFrame(); i++)
			{
				bool waitForVblank = interpreter.EmulateCycle();
				//should this stay or go?
				//if (waitForVblank)
				//{
				//	//skips the rest of instructions for this frame
				//	break;
				//}
			}
			break;
		case Chip8::EmulatorStates::Paused:
			//shouldUpdateGame = false;
			break;
		case Chip8::EmulatorStates::Step:
			interpreter.EmulateCycle();
			emulatorState = Chip8::EmulatorStates::Paused;
			break;
		case Chip8::EmulatorStates::Reset:
			interpreter.Reset();
			emulatorState = Chip8::EmulatorStates::Running;
			continue;
		case Chip8::EmulatorStates::Loading_Game:
			std::string newGameName{ OpenFileDialog() };
			interpreter.LoadGame(newGameName);
			emulatorState = Chip8::EmulatorStates::Running;
			continue;
		}

		interpreter.UpdateTimers();

		//Imgui handling is currently done in the Renderer render function, not sure how to split this
		emulatorState = renderer.Render(emulatorState);
		renderer.Update();

		auto sleepTime{ timer.GetSleepTime() };
		std::this_thread::sleep_for(sleepTime);
	}

	interpreter.Destroy();

	return 0;
}

std::string OpenFileDialog()
{
	OPENFILENAME ofn;        // Structure for file dialog
	char filePath[MAX_PATH] = { 0 }; // Buffer to store the selected file path
	char absolutePath[MAX_PATH] = { 0 };
	std::string initialDir{ "../../../roms" };

	// Convert relative path to absolute path
	GetFullPathName(initialDir.c_str(), MAX_PATH, absolutePath, nullptr);

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = nullptr; // Parent window handle (nullptr for none)
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrInitialDir = absolutePath;

	// Filter for file types: Description | *.ext | Description | *.ext | NULL
	ofn.lpstrFilter = "Chip8\0*.ch8";
	ofn.nFilterIndex = 1; // Default filter index (1-based)
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

	if (GetOpenFileName(&ofn))
	{
		return std::string(filePath);
	}
	return "";
}
