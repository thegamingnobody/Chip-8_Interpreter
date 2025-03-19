#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include "ScreenManager.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
//#include <backends/imgui_impl_opengl3.h>

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

	m_BackgroundColor = SDL_Color();
	m_BackgroundColor.r = 0;
	m_BackgroundColor.g = 0;
	m_BackgroundColor.b = 0;

	m_Screen.resize(windowHeight);
	for (int row = 0; row < windowHeight; row++)
	{
		m_Screen[row].resize(windowWidth);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer);
	ImGui_ImplSDLRenderer3_Init(m_Renderer);
}

void Chip8::Renderer::Render() const
{
	//Todo: Implement selectable color palettes
	//SDL_SetRenderScale(m_Renderer, m_WindowScale, m_WindowScale);

	const auto& color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_Renderer);

	//ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);

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


	RenderImgui();

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);

	SDL_RenderPresent(m_Renderer);
}

void Chip8::Renderer::Destroy()
{
	//ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
	ImGui::DestroyContext();

	if (m_Renderer != nullptr)
	{
		SDL_DestroyRenderer(m_Renderer);
		m_Renderer = nullptr;
		SDL_DestroyWindow(m_Window);
		m_Window = nullptr;
	}
}

void Chip8::Renderer::SetPixel(int x, int y, bool value)
{
	m_Screen[y][x] = value;
}

void Chip8::Renderer::TogglePixel(int x, int y)
{
	bool isPixelOn = IsPixelOn(x, y);
	SetPixel(x, y, !isPixelOn);
}

bool Chip8::Renderer::IsPixelOn(int x, int y) const
{
	assert(x >= 0 && x < m_WidthBase);
	assert(y >= 0 && y < m_HeightBase);

	return m_Screen[y][x];
}

void Chip8::Renderer::ClearScreen()
{
	for (int row = 0; row < m_HeightBase; row++)
	{
		std::fill(m_Screen[row].begin(), m_Screen[row].end(), false);
	}
}

void Chip8::Renderer::RenderImgui() const
{
	ImGui::Begin("Test", nullptr, ImGuiWindowFlags_NoCollapse);

	ImGui::ShowDemoWindow();

	ImGui::Text("Calculating...");

	ImGui::End();
}
