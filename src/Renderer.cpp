#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include "ScreenManager.h"

void Chip8::Renderer::Init(int windowWidth, int windowHeight, float windowScale)
{
	m_WidthBase = windowWidth;
	m_HeightBase = windowHeight;
	m_WindowScale = windowScale;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error(std::string("SDL could not initialize! SDL_Error: ") + SDL_GetError());
	}

	m_Window = SDL_CreateWindow("Window", windowWidth * windowScale, windowHeight * windowScale, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		throw std::runtime_error(std::string("SDL could not create window! SDL_Error: ") + SDL_GetError());
	}

	m_Renderer = SDL_CreateRenderer(m_Window, NULL);
	if (!m_Renderer)
	{
		throw std::runtime_error(std::string("SDL could not create renderer! SDL_Error: ") + SDL_GetError());
	}

	m_RenderTexture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, windowWidth, windowHeight);

	//if (!SDL_CreateWindowAndRenderer("Interpreter Window", windowWidth * windowScale, windowHeight * windowScale, 0, &m_Window, &m_Renderer))
	//{
	//	throw std::runtime_error(std::string("SDL could not create window or renderer! SDL_Error: ") + SDL_GetError());
	//}

	m_BackgroundColor = SDL_Color();
	m_BackgroundColor.r = 0;
	m_BackgroundColor.g = 0;
	m_BackgroundColor.b = 0;

	m_Screen.resize(windowHeight);
	for (int row = 0; row < windowHeight; row++)
	{
		m_Screen[row].resize(windowWidth);
	}
	


	for (int row = 0; row < windowHeight; row++)
	{
		for (int column = 0; column < windowWidth; column++)
		{
			m_Screen[row][column] = ((column + row) % 2);
		}
	}
}

void Chip8::Renderer::Render() const
{
	SDL_SetRenderScale(m_Renderer, m_WindowScale, m_WindowScale);

	const auto& color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_Renderer);

	//Chip8::ScreenManager::GetInstance().Render();

	for (int row = 0; row < m_Screen.size(); row++)
	{
		for (int column = 0; column < m_Screen[row].size(); column++)
		{
			//Render pixel
			if (m_Screen[row][column])
			{
				SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 100);
				SDL_RenderPoint(m_Renderer, column, row);
			}
		}
	}

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
