#include "Renderer.h"
#include <stdexcept>
#include <iostream>
#include "ScreenManager.h"
#include <cassert>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>
//#define _CRT_SECURE_NO_WARNINGS
//#define STB_IMAGE_IMPLEMENTATION
//#include "stb_image.h"
//
//bool LoadTextureFromMemory(const void* data, size_t data_size, SDL_Renderer* renderer, SDL_Texture** out_texture, int* out_width, int* out_height)
//{
//	int image_width = 0;
//	int image_height = 0;
//	int channels = 4;
//	unsigned char* image_data = stbi_load_from_memory((const unsigned char*)data, (int)data_size, &image_width, &image_height, NULL, 4);
//	if (image_data == nullptr)
//	{
//		fprintf(stderr, "Failed to load image: %s\n", stbi_failure_reason());
//		return false;
//	}
//
//	SDL_Surface* surface = SDL_CreateSurfaceFrom(image_width, image_height, SDL_PIXELFORMAT_RGBA32, (void*)image_data, channels * image_width);
//	if (surface == nullptr)
//	{
//		fprintf(stderr, "Failed to create SDL surface: %s\n", SDL_GetError());
//		return false;
//	}
//
//	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
//	if (texture == nullptr)
//		fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
//
//	*out_texture = texture;
//	*out_width = image_width;
//	*out_height = image_height;
//
//	SDL_DestroySurface(surface);
//	stbi_image_free(image_data);
//
//	return true;
//}
//
//// Open and read a file, then forward to LoadTextureFromMemory()
//bool LoadTextureFromFile(const char* file_name, SDL_Renderer* renderer, SDL_Texture** out_texture, int* out_width, int* out_height)
//{
//	FILE* f = fopen(file_name, "rb");
//	if (f == NULL)
//		return false;
//	fseek(f, 0, SEEK_END);
//	size_t file_size = (size_t)ftell(f);
//	if (file_size == -1)
//		return false;
//	fseek(f, 0, SEEK_SET);
//	void* file_data = IM_ALLOC(file_size);
//	fread(file_data, 1, file_size, f);
//	fclose(f);
//	bool ret = LoadTextureFromMemory(file_data, file_size, renderer, out_texture, out_width, out_height);
//	IM_FREE(file_data);
//	return ret;
//}

void Chip8::Renderer::Init(int windowWidth, int windowHeight, float windowScale)
{
	m_WidthBase = windowWidth;
	m_HeightBase = windowHeight;
	m_WindowScale = windowScale;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		throw std::runtime_error(std::string("SDL could not initialize! SDL_Error: ") + SDL_GetError());
	}

	//m_Window = SDL_CreateWindow("Window", windowWidth * windowScale, windowHeight * windowScale, SDL_WINDOW_RESIZABLE);
	m_Window = SDL_CreateWindow("Window", 640, 480, SDL_WINDOW_RESIZABLE);
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
	SDL_SetRenderTarget(m_Renderer, m_RenderTexture);

	SDL_SetTextureScaleMode(m_RenderTexture, SDL_SCALEMODE_NEAREST);

	//Todo: make changed pixel buffer
	for (int row = 0; row < m_Screen.size(); row++)
	{
		for (int column = 0; column < m_Screen[row].size(); column++)
		{
			if (m_Screen[row][column])
			{
				SDL_SetRenderDrawColor(m_Renderer, 255, 255, 255, 100);
			}
			else
			{
				SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 100);
			}
			SDL_RenderPoint(m_Renderer, column, row);
		}
	}

	SDL_SetRenderTarget(m_Renderer, NULL);

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Once);

	RenderImgui();

	const auto& color = GetBackgroundColor();
	SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
	SDL_RenderClear(m_Renderer);

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
	ImGui::Begin("Viewport");
	ImGui::Image((ImTextureID)(intptr_t)m_RenderTexture, ImVec2((float)m_WidthBase * m_WindowScale, (float)m_HeightBase * m_WindowScale));
	ImGui::End();
}

