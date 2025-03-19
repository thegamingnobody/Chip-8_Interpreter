#include "InputManager.h"
#include <SDL3/SDL.h>
#include <Logger.h>
#include <SDL3/SDL_keyboard.h>
#include <backends/imgui_impl_sdl3.h>

void Chip8::InputManager::Init()
{
	m_KeysState.resize(16);
	m_KeyPressedThisFrame = false;
}

bool Chip8::InputManager::ProcessInput()
{
	m_KeyPressedThisFrame = false;
	std::fill(m_KeysState.begin(), m_KeysState.end(), false);

	SDL_Event e;
	while (SDL_PollEvent(&e)) 
	{
		if (e.type == SDL_EVENT_QUIT) 
		{
			return false;
		}
		ImGui_ImplSDL3_ProcessEvent(&e);
	}


	CheckKeys();

	return true;
}

bool Chip8::InputManager::IsKeyPressed(int key) const
{
	return m_KeysState[key];
}

void Chip8::InputManager::CheckKeys()
{
	SDL_PumpEvents();
	const bool* keyState = SDL_GetKeyboardState(nullptr);

	//Todo: remappable input
	//Todo: Azerty support
	//Todo: fix eww looking if else if ?

	if (keyState[SDL_SCANCODE_1])
	{
		SetKey(0x1);
	}
	else if (keyState[SDL_SCANCODE_2])
	{
		SetKey(0x2);
	}
	else if (keyState[SDL_SCANCODE_3])
	{
		SetKey(0x3);
	}
	else if (keyState[SDL_SCANCODE_4])
	{
		SetKey(0xC);
	}
	else if (keyState[SDL_SCANCODE_Q])
	{
		SetKey(0x4);
	}
	else if (keyState[SDL_SCANCODE_W])
	{
		SetKey(0x5);
	}
	else if (keyState[SDL_SCANCODE_E])
	{
		SetKey(0x6);
	}
	else if (keyState[SDL_SCANCODE_R])
	{
		SetKey(0xD);
	}
	else if (keyState[SDL_SCANCODE_A])
	{
		SetKey(0x7);
	}
	else if (keyState[SDL_SCANCODE_S])
	{
		SetKey(0x8);
	}
	else if (keyState[SDL_SCANCODE_D])
	{
		SetKey(0x9);
	}
	else if (keyState[SDL_SCANCODE_F])
	{
		SetKey(0xE);
	}
	else if (keyState[SDL_SCANCODE_Z])
	{
		SetKey(0xA);
	}
	else if (keyState[SDL_SCANCODE_X])
	{
		SetKey(0x0);
	}
	else if (keyState[SDL_SCANCODE_C])
	{
		SetKey(0xB);
	}
	else if (keyState[SDL_SCANCODE_V])
	{
		SetKey(0xF);
	}
}

void Chip8::InputManager::SetKey(int key)
{
	m_KeysState[key] = true;
	//Chip8::Logger::GetInstance().Log(std::to_string(key) + " pressed!");
	m_KeyPressedThisFrame = true;
}