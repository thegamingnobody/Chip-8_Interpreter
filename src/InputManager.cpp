#include "InputManager.h"
#include <SDL3/SDL.h>
#include <Logger.h>
#include <SDL3/SDL_keyboard.h>
#include <backends/imgui_impl_sdl3.h>
#include <iostream>

void Chip8::InputManager::Init()
{
	m_KeysState.resize(16);
	m_KeyPressedThisFrame = false;

	m_Keymap = { SDLK_X, SDLK_1, SDLK_2, SDLK_3, SDLK_Q, SDLK_W, SDLK_E, SDLK_A, SDLK_S, SDLK_D, SDLK_Z, SDLK_C, SDLK_4, SDLK_R, SDLK_F, SDLK_V };
}

bool Chip8::InputManager::ProcessInput()
{
	m_KeyPressedThisFrame = false;
	//std::fill(m_KeysState.begin(), m_KeysState.end(), false);

	SDL_Event e;
	while (SDL_PollEvent(&e)) 
	{
		if (e.type == SDL_EVENT_QUIT)
		{
			return false;
		}
		else if (e.type == SDL_EVENT_KEY_DOWN)
		{
			for (int i = 0; i < m_KeysState.size(); i++)
			{
				if (e.key.key == m_Keymap[i])
				{
					SetKey(i, true);
					break;
				}
			}
		}
		else if (e.type == SDL_EVENT_KEY_UP)
		{
			for (int i = 0; i < m_KeysState.size(); i++)
			{
				if (e.key.key == m_Keymap[i])
				{
					SetKey(i, false);
					break;
				}
			}
		}
		ImGui_ImplSDL3_ProcessEvent(&e);
	}


	//CheckKeys();

	return true;
}

bool Chip8::InputManager::IsKeyPressed(int key) const
{
	return m_KeysState[key];
}

//void Chip8::InputManager::CheckKeys()
//{
//	SDL_PumpEvents();
//	const bool* keyState = SDL_GetKeyboardState(nullptr);
//
//	//Todo: remappable input
//	//Todo: Azerty support
//	//Todo: fix eww looking if else if ?
//
//	if (keyState[SDL_SCANCODE_1])
//	{
//		SetKey(0x1);
//	}
//	else if (keyState[SDL_SCANCODE_2])
//	{
//		SetKey(0x2);
//	}
//	else if (keyState[SDL_SCANCODE_3])
//	{
//		SetKey(0x3);
//	}
//	else if (keyState[SDL_SCANCODE_4])
//	{
//		SetKey(0xC);
//	}
//	else if (keyState[SDL_SCANCODE_Q])
//	{
//		SetKey(0x4);
//	}
//	else if (keyState[SDL_SCANCODE_W])
//	{
//		SetKey(0x5);
//	}
//	else if (keyState[SDL_SCANCODE_E])
//	{
//		SetKey(0x6);
//	}
//	else if (keyState[SDL_SCANCODE_R])
//	{
//		SetKey(0xD);
//	}
//	else if (keyState[SDL_SCANCODE_A])
//	{
//		SetKey(0x7);
//	}
//	else if (keyState[SDL_SCANCODE_S])
//	{
//		SetKey(0x8);
//	}
//	else if (keyState[SDL_SCANCODE_D])
//	{
//		SetKey(0x9);
//	}
//	else if (keyState[SDL_SCANCODE_F])
//	{
//		SetKey(0xE);
//	}
//	else if (keyState[SDL_SCANCODE_Z])
//	{
//		SetKey(0xA);
//	}
//	else if (keyState[SDL_SCANCODE_X])
//	{
//		SetKey(0x0);
//	}
//	else if (keyState[SDL_SCANCODE_C])
//	{
//		SetKey(0xB);
//	}
//	else if (keyState[SDL_SCANCODE_V])
//	{
//		SetKey(0xF);
//	}
//}

void Chip8::InputManager::RenderImGui(std::string windowName)
{
	ImGui::Begin(windowName.c_str());

	std::vector<byte> keysOrder = { 0x1, 0x2, 0x3, 0xC, 0x4, 0x5, 0x6, 0xD, 0x7, 0x8, 0x9, 0xE, 0xA, 0x0, 0xB, 0xF };

	ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("buttons pressed", 4, flags))
	{
		for (int row = 0; row < 4; row++)
		{
			ImGui::TableNextRow();
			for (int column = 0; column < 4; column++)
			{
				ImGui::TableSetColumnIndex(column);
				byte buttonID = keysOrder[column + (row * 4)];

				std::string text{ std::to_string(buttonID) + ": " + std::to_string(m_KeysState[buttonID]) };
				ImGui::Text(text.c_str());
			}
		}
		ImGui::EndTable();
	}
	ImGui::End();
}

void Chip8::InputManager::SetKey(int key, bool newState)
{
	m_KeysState[key] = newState;
	//Chip8::Logger::GetInstance().Log(std::to_string(key) + " is now " + std::to_string(newState) + "!");
	//std::cout << std::to_string(key) + " is now " + std::to_string(newState) + "!\n";
	m_KeyPressedThisFrame = true;
}