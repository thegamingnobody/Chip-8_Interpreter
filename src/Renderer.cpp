#include "Renderer.h"
#include <stdexcept>
#include <iostream>

void Chip8::Renderer::Init(int windowWidth, int windowHeight)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error(std::string("SDL could not initialize! SDL_Error: ") + SDL_GetError());
	}

	if (!SDL_CreateWindowAndRenderer("Interpreter Window", windowWidth, windowHeight, 0, &m_Window, &m_Renderer))
	{
		throw std::runtime_error(std::string("SDL could not create window or renderer! SDL_Error: ") + SDL_GetError());
	}

	m_BackgroundColor = SDL_Color();
	m_BackgroundColor.r = 0;
	m_BackgroundColor.g = 0;
	m_BackgroundColor.b = 0;
}

void Chip8::Renderer::Render() const
{
	const auto& color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_Renderer);

	SDL_RenderPresent(m_Renderer);
}

void Chip8::Renderer::Destroy()
{
	if (m_Renderer != nullptr)
	{
		SDL_DestroyRenderer(m_Renderer);
		m_Renderer = nullptr;
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}
}
