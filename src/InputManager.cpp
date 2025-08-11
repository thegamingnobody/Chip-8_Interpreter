#include "InputManager.h"
#include <SDL2/SDL.h>
#include <Logger.h>
#include <SDL2/SDL_keyboard.h>
#include <backends/imgui_impl_sdl2.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include "Chip-8_Interpreter.h"

void Chip8::InputManager::Init()
{
	m_KeysStateCurrent.resize(16);
	m_KeyPressedThisFrame = false;

	m_Keymap = { SDLK_x, SDLK_1, SDLK_2, SDLK_3, SDLK_q, SDLK_w, SDLK_e, SDLK_a, SDLK_s, SDLK_d, SDLK_z, SDLK_c, SDLK_4, SDLK_r, SDLK_f, SDLK_v };
}

bool Chip8::InputManager::ProcessInput()
{
	m_KeyPressedThisFrame = false;
	m_KeyReleasedThisFrame = false;

	m_KeysStateLastFrame = m_KeysStateCurrent;

	SDL_Event e;
	while (SDL_PollEvent(&e)) 
	{
		if (e.type == SDL_QUIT)
		{
			return false;
		}
		else if (e.type == SDL_KEYDOWN)
		{
			for (int i = 0; i < m_KeysStateCurrent.size(); i++)
			{
				if (e.key.keysym.scancode == m_Keymap[i])
				{
					m_KeyPressedThisFrame = true;
					SetKey(i, true);
					break;
				}
			}
		}
		else if (e.type == SDL_KEYUP)
		{
			for (int i = 0; i < m_KeysStateCurrent.size(); i++)
			{
				if (e.key.keysym.scancode == m_Keymap[i])
				{
					m_KeyReleasedThisFrame = true;
					SetKey(i, false);
					break;
				}
			}
		}
		ImGui_ImplSDL2_ProcessEvent(&e);
	}

	return true;
}

bool Chip8::InputManager::IsKeyPressed(int key) const
{
	return m_KeysStateCurrent[key];
}

bool Chip8::InputManager::IsKeyReleased(int key) const
{
	return m_KeysStateLastFrame[key];
}

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

					std::stringstream stream;
					stream << std::uppercase << std::hex << static_cast<int>(buttonID) << ": " << m_KeysStateCurrent[buttonID];
					ImGui::Text(stream.str().c_str());
				}
			}
			ImGui::EndTable();
		}

		if (Chip8::Interpreter::GetInstance().IsWaitingForInput())
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.3f, 0.3f, 1.0f));
			ImGui::Button("Waiting for Input...");
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.85f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.85f, 0.3f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.85f, 0.3f, 1.0f));
			ImGui::Button("Running");
		}
		ImGui::PopStyleColor(3);
	ImGui::End();

}

void Chip8::InputManager::SetKey(int key, bool newState)
{
	m_KeysStateCurrent[key] = newState;
	m_KeyPressedThisFrame = true;
}