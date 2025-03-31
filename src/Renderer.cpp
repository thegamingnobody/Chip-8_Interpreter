#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include "ScreenManager.h"
#include <cassert>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
#include "TimeManager.h"
#include "InputManager.h"
#include <imgui_internal.h>
#include <Chip-8_Interpreter.h>

void Chip8::Renderer::Init(float windowScale)
{
	m_WindowScale = windowScale;

	m_ViewportScale = 8.0f;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error(std::string("SDL could not initialize! SDL_Error: ") + SDL_GetError());
	}

	m_Window = SDL_CreateWindow("Window", WINDOW_WIDTH_BASE * m_WindowScale, WINDOW_HEIGHT_BASE * m_WindowScale, NULL);
	if (!m_Window)
	{
		throw std::runtime_error(std::string("SDL could not create window! SDL_Error: ") + SDL_GetError());
	}

	m_Renderer = SDL_CreateRenderer(m_Window, NULL);
	if (!m_Renderer)
	{
		throw std::runtime_error(std::string("SDL could not create renderer! SDL_Error: ") + SDL_GetError());
	}

	m_RenderTexture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, VIEWPORT_WIDTH_BASE, VIEWPORT_HEIGHT_BASE);

	m_BackgroundColor = SDL_Color();
	m_BackgroundColor.r = 0;
	m_BackgroundColor.g = 0;
	m_BackgroundColor.b = 0;

	m_Screen.resize(VIEWPORT_HEIGHT_BASE);
	for (int row = 0; row < VIEWPORT_HEIGHT_BASE; row++)
	{
		m_Screen[row].resize(VIEWPORT_WIDTH_BASE);
	}

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLRenderer(m_Window, m_Renderer);
	ImGui_ImplSDLRenderer3_Init(m_Renderer);

	ImGuiIO& io = ImGui::GetIO();
	m_Font = io.Fonts->AddFontFromFileTTF( "../../../ProggyClean.ttf", 24);
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	SDL_SetRenderTarget(m_Renderer, m_RenderTexture);
	SDL_SetRenderDrawColor(m_Renderer, 10, 10, 10, 255);

	for (int row = 0; row < m_Screen.size(); row++)
	{
		for (int col = 0; col < m_Screen[row].size(); col++)
		{
			SDL_RenderPoint(m_Renderer, col, row);
		}
	}
	SDL_SetRenderTarget(m_Renderer, NULL);

	//Enable to reset layout
	ImGui::LoadIniSettingsFromMemory("");
}

Chip8::EmulatorStates Chip8::Renderer::Render(Chip8::EmulatorStates emulatorState) const
{
	//Todo: Implement selectable color palettes
	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_Renderer);

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	
	auto& interpreter = Chip8::Interpreter::GetInstance();
	//if (interpreter.GetDrawFlag())
	{
		UpdateRenderTexture();
	}

	auto newRunningState = RenderImgui(emulatorState);

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);

	SDL_RenderPresent(m_Renderer);

	return newRunningState;
}

void Chip8::Renderer::Update()
{
	auto& interpreter = Chip8::Interpreter::GetInstance();
	if (interpreter.GetDrawFlag())
	{
		interpreter.SetDrawFlag(false);
		Chip8::TimeManager::GetInstance().IncrementFrameCounter();
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
	for (int row = 0; row < VIEWPORT_HEIGHT_BASE; row++)
	{
		std::fill(m_Screen[row].begin(), m_Screen[row].end(), false);
	}
	Chip8::Interpreter::GetInstance().SetDrawFlag(true);
}

Chip8::EmulatorStates Chip8::Renderer::RenderImgui(Chip8::EmulatorStates emulatorState) const
{
	ImGui::PushFont(m_Font);
	auto& timer = Chip8::TimeManager::GetInstance();
	auto& input = Chip8::InputManager::GetInstance();
	auto& interpreter = Chip8::Interpreter::GetInstance();
	Chip8::EmulatorStates returnState{ emulatorState };

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::Begin("##DockspaceWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoCloseButton;
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(WINDOW_WIDTH_BASE * m_WindowScale, WINDOW_HEIGHT_BASE * m_WindowScale), dockspace_flags);
		ImGui::End();

		static auto first_time = true;
		if (first_time)
		{
			first_time = false;
			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspace_id);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags);
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
			// get id of main dock space area
			ImGuiID dockspace_main_id = dockspace_id;
			ImGuiID input_id = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Right, 0.25f, nullptr, &dockspace_main_id);
			ImGuiID performance_id = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Down, 0.5f, nullptr, &dockspace_main_id);
			ImGuiID memory_id = ImGui::DockBuilderSplitNode(input_id, ImGuiDir_Down, 0.80f, nullptr, &input_id);
			ImGuiID game_info = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Right, 0.35f, nullptr, &dockspace_main_id);

			ImGui::DockBuilderDockWindow("Viewport", dockspace_main_id);
			ImGui::DockBuilderDockWindow("Performance", performance_id);
			ImGui::DockBuilderDockWindow("Input", input_id);
			ImGui::DockBuilderDockWindow("Memory", memory_id);
			ImGui::DockBuilderDockWindow("Game Info", game_info);
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)m_RenderTexture, ImVec2((float)VIEWPORT_WIDTH_BASE * m_ViewportScale, (float)VIEWPORT_HEIGHT_BASE * m_ViewportScale));
	ImGui::End();

	timer.RenderImGui("Performance");
	input.RenderImGui("Input");
	returnState = interpreter.RenderImgui("Memory", returnState);
	returnState = interpreter.RenderImgui("Game Info", returnState);

	//ImGui::ShowDemoWindow();

	ImGui::PopFont();

	return returnState;
}

void Chip8::Renderer::UpdateRenderTexture() const
{
	SDL_SetRenderTarget(m_Renderer, m_RenderTexture);
	SDL_SetTextureScaleMode(m_RenderTexture, SDL_SCALEMODE_NEAREST);

	//for (Pixel pixel : m_ChangedPixels)
	for (int row = 0; row < VIEWPORT_HEIGHT_BASE; row++)
	{
		for (int col = 0; col < VIEWPORT_WIDTH_BASE; col++)
		{
			if (m_Screen[row][col])
			{
				//Set color on
				SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 255);
			}
			else
			{
				//Set color off
				SDL_SetRenderDrawColor(m_Renderer, 10, 10, 10, 255);
			}
			SDL_RenderPoint(m_Renderer, col, row);
		}
	}

	SDL_SetRenderTarget(m_Renderer, NULL);
}

