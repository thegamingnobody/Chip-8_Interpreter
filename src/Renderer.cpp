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
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);

	for (int row = 0; row < m_Screen.size(); row++)
	{
		for (int col = 0; col < m_Screen[row].size(); col++)
		{
			SDL_RenderPoint(m_Renderer, col, row);
		}
	}

	//Enable to reset layout
	ImGui::LoadIniSettingsFromMemory("");
}

Chip8::EmulatorStates Chip8::Renderer::Render(bool drawFlag, Chip8::ProgramCounterInfo pcInfo, Chip8::EmulatorStates emulatorState) const
{
	//Todo: Implement selectable color palettes
	SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
	SDL_RenderClear(m_Renderer);

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();
	
	if (drawFlag)
	{
		UpdateRenderTexture();
	}

	auto newRunningState = RenderImgui(pcInfo, emulatorState);

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), m_Renderer);

	SDL_RenderPresent(m_Renderer);

	return newRunningState;
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

Chip8::EmulatorStates Chip8::Renderer::RenderImgui(Chip8::ProgramCounterInfo pcInfo, Chip8::EmulatorStates emulatorState) const
{
	ImGui::PushFont(m_Font);
	auto& timer = Chip8::TimeManager::GetInstance();
	auto& input = Chip8::InputManager::GetInstance();
	Chip8::EmulatorStates returnState{ emulatorState };

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) 
	{
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
		ImGui::Begin("##DockspaceWindow", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoUndocking | ImGuiDockNodeFlags_NoCloseButton;
		ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
		ImGui::DockSpace(dockspace_id, ImVec2(m_WindowWidthBase * m_WindowScale, m_WindowHeightBase * m_WindowScale), dockspace_flags);
		ImGui::End();

		static auto first_time = true;
		if (first_time)
		{
			first_time = false;
			// Clear out existing layout
			ImGui::DockBuilderRemoveNode(dockspace_id);
			// Add empty node
			ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
			// Main node should cover entire window
			ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetWindowSize());
			// get id of main dock space area
			ImGuiID dockspace_main_id = dockspace_id;
			ImGuiID input_id = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Right, 1.0f, nullptr, &dockspace_main_id);
			ImGuiID performance_id = ImGui::DockBuilderSplitNode(dockspace_main_id, ImGuiDir_Down, 0.25f, nullptr, &dockspace_main_id);
			ImGuiID memory_id = ImGui::DockBuilderSplitNode(input_id, ImGuiDir_Down, 0.5f, nullptr, &input_id);

			ImGui::DockBuilderDockWindow("Viewport", dockspace_main_id);
			ImGui::DockBuilderDockWindow("Performance", performance_id);
			ImGui::DockBuilderDockWindow("Input", input_id);
			ImGui::DockBuilderDockWindow("Memory", memory_id);
			ImGui::DockBuilderFinish(dockspace_id);
		}
	}

	ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	ImGui::Image((ImTextureID)(intptr_t)m_RenderTexture, ImVec2((float)m_ViewportWidthBase * m_ViewportScale, (float)m_ViewportHeightBase * m_ViewportScale));
	ImGui::End();

	timer.RenderImGui("Performance");
	input.RenderImGui("Input");

	ImGui::Begin("Memory", nullptr);
		ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
		if (emulatorState == Chip8::EmulatorStates::Running and ImGui::Button("Pause"))
		{
			returnState = Chip8::EmulatorStates::Paused;
		}
		else if(emulatorState == Chip8::EmulatorStates::Paused and ImGui::Button("Resume"))
		{
			returnState = Chip8::EmulatorStates::Running;
		}
		ImGui::SameLine();
		if (ImGui::Button("Step"))
		{
			returnState = Chip8::EmulatorStates::Step;
		}
		if (ImGui::BeginTable("Memory", 3, flags))
		{
			ImGui::TableSetupColumn("PC");
			ImGui::TableSetupColumn("Address");
			ImGui::TableSetupColumn("Value");
			ImGui::TableHeadersRow();

			for (int row = 0; row < 5; row++)
			{
				ImGui::TableNextRow();
				for (int column = 0; column < 3; column++)
				{
					ImGui::TableSetColumnIndex(column);
					switch (column)
					{
					case 0:
						if (row == 2)
						{
							ImGui::Text("=>");
						}
						else
						{
							ImGui::Text("  ");
						}
						break;
					case 1:
						{
							std::stringstream stream;
							stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << pcInfo.CurrentProgramCounter + (2 * (row - 2));
							ImGui::Text(stream.str().c_str());
						}
						break;
					case 2:
						{
							std::stringstream stream;
							stream << "0x" << std::uppercase << std::hex << std::setfill('0') << std::setw(4) << pcInfo.MemoryValuesAtPC[row];
							ImGui::Text(stream.str().c_str());
						}
						break;
					}
				}
			}
			ImGui::EndTable();
		}
	ImGui::End();

	ImGui::PopFont();

	return returnState;
}

void Chip8::Renderer::UpdateRenderTexture() const
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


	SDL_SetRenderTarget(m_Renderer, NULL);
}

