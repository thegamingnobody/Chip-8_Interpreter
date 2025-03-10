#include "InputManager.h"
#include <SDL3/SDL.h>

void Chip8::InputManager::Init()
{
}

bool Chip8::InputManager::ProcessInput()
{
	//Todo: set keystates => move m_Keys to here?
	SDL_Event e;
	while (SDL_PollEvent(&e)) 
	{
		if (e.type == SDL_EVENT_QUIT) 
		{
			return false;
		}
	}

	return true;
}
