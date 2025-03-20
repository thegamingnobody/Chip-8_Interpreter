#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include "ScreenManager.h"
#include <cassert>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "TimeManager.h"
#include <filesystem>

void Chip8::Renderer::Init(int windowWidth, int windowHeight, float windowScale)
{
	m_WindowWidthBase= windowWidth;
	m_WindowHeightBase = windowHeight;
	m_WindowScale = windowScale;

	m_ViewportWidthBase = 64;
	m_ViewportHeightBase = 32;
	m_ViewportScale = 8.0f;

	m_ScreenCleared = false;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error(std::string("SDL could not initialize! SDL_Error: ") + SDL_GetError());
	}

	//m_Window = SDL_CreateWindow("Window", windowWidth * windowScale, windowHeight * windowScale, SDL_WINDOW_RESIZABLE);
	m_Window = SDL_CreateWindow("Window", m_WindowWidthBase * m_WindowScale, m_WindowHeightBase * m_WindowScale, SDL_WINDOW_RESIZABLE);
	if (!m_Window)
	{
		throw std::runtime_error(std::string("SDL could not create window! SDL_Error: ") + SDL_GetError());
	}

	m_Renderer = SDL_CreateRenderer(m_Window, NULL);
	if (!m_Renderer)
	{
		throw std::runtime_error(std::string("SDL could not create renderer! SDL_Error: ") + SDL_GetError());
	}

	//SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	m_RenderTexture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, m_ViewportWidthBase, m_ViewportHeightBase);

	m_BackgroundColor = SDL_Color();
	m_BackgroundColor.r = 0;
	m_BackgroundColor.g = 0;
	m_BackgroundColor.b = 0;

	m_Screen.resize(m_ViewportHeightBase);
	for (int row = 0; row < m_ViewportHeightBase; row++)
	{
		m_Screen[row].resize(m_ViewportWidthBase);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer);
	ImGui_ImplSDLRenderer3_Init(m_Renderer);

	ImGuiIO& io = ImGui::GetIO();
	m_Font = io.Fonts->AddFontFromFileTTF( "../../../ProggyClean.ttf", 24);

	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);

	for (int row = 0; row < m_Screen.size(); row++)
	{
		for (int col = 0; col < m_Screen[row].size(); col++)
		{
			SDL_RenderPoint(m_Renderer, col, row);
		}
	}
}

void Chip8::Renderer::Render(bool drawFlag) const
{
	//Todo: Implement selectable color palettes
	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_Renderer);

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	
	if (drawFlag)
	{
		SDL_SetRenderTarget(m_Renderer, m_RenderTexture);
		SDL_SetTextureScaleMode(m_RenderTexture, SDL_SCALEMODE_NEAREST);

		if (m_ScreenCleared)
		{
			SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);

			for (int row = 0; row < m_Screen.size(); row++)
			{
				for (int col = 0; col < m_Screen[row].size(); col++)
				{
					SDL_RenderPoint(m_Renderer, col, row);
				}
			}
		}
		else
		{
			for (Pixel pixel : m_ChangedPixels)
			{
				if (m_Screen[pixel.y][pixel.x])
				{
					SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
				}
				else
				{
					SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
				}
				SDL_RenderPoint(m_Renderer, pixel.x, pixel.y);
			}
		}

		SDL_SetRenderTarget(m_Renderer, NULL);
	}

	RenderImgui();

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);

	SDL_RenderPresent(m_Renderer);
}

void Chip8::Renderer::Update()
{
	if (m_ChangedPixels.size() > 0)
	{
		m_ChangedPixels.clear();
	}
	if (m_ScreenCleared)
	{
		for (int row = 0; row < m_ViewportHeightBase; row++)
		{
			std::fill(m_Screen[row].begin(), m_Screen[row].end(), false);
		}
		m_ScreenCleared = false;
	}
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
	m_ChangedPixels.emplace_back(x, y);
}

void Chip8::Renderer::TogglePixel(int x, int y)
{
	bool isPixelOn = IsPixelOn(x, y);
	SetPixel(x, y, !isPixelOn);
}

bool Chip8::Renderer::IsPixelOn(int x, int y) const
{
	return m_Screen[y][x];
}

void Chip8::Renderer::ClearScreen()
{
	m_ScreenCleared = true;
}

void Chip8::Renderer::RenderImgui() const
{
	ImGui::PushFont(m_Font);
	auto& timer = Chip8::TimeManager::GetInstance();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)m_RenderTexture, ImVec2((float)m_ViewportWidthBase * m_ViewportScale, (float)m_ViewportHeightBase * m_ViewportScale));
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 300.0f), ImGuiCond_Once);

	ImGui::Begin("Performance", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize);
	std::string text{ "Instructions/s: " + std::to_string(timer.GetInstructionsPerSecond()) };
	ImGui::Text(text.c_str());
	ImGui::End();

	ImGui::ShowDemoWindow();

	ImGui::PopFont();
}

