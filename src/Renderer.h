#pragma once

#include "Singleton.h"
#include <SDL3/SDL.h>
#include <vector>
#include <imgui.h>

namespace Chip8
{
	struct Pixel
	{
		int x;
		int y;
		Pixel(int xpos = 0, int ypos = 0) : x(xpos), y(ypos) {}
	};

	class Renderer : public Singleton<Renderer>
	{
	public:
		void Init(int windowWidth, int windowHeight, float windowScale = 1.0f);
		void Render(bool drawFlag) const;
		void Update();
		void Destroy();

		void SetBackgroundColor(const SDL_Color& newColor) { m_BackgroundColor = newColor; }
		const SDL_Color& GetBackgroundColor() const { return m_BackgroundColor; }

		int GetWindowWidth() const { return m_WindowWidthBase; }
		int GetWindowHeight() const { return m_WindowHeightBase; }
		float GetWindowScale() const { return m_WindowScale; }

		int GetViewportWidth() const { return m_ViewportWidthBase; }
		int GetViewportHeight() const { return m_ViewportHeightBase; }
		float GetViewportScale() const { return m_ViewportScale; }

		void SetPixel(int x, int y, bool value);
		void TogglePixel(int x, int y);
		bool IsPixelOn(int x, int y) const;

		void ClearScreen();

		void RenderImgui() const;

	private:
		int m_WindowWidthBase;
		int m_WindowHeightBase;
		float m_WindowScale;

		int m_ViewportWidthBase;
		int m_ViewportHeightBase;
		float m_ViewportScale;

		SDL_Window* m_Window{};
		SDL_Renderer* m_Renderer{};
		SDL_Texture* m_RenderTexture{};
		SDL_Color m_BackgroundColor{};

		//Screen
		std::vector<std::vector<bool>> m_Screen;
		std::vector<Pixel> m_ChangedPixels;
		bool m_ScreenCleared;
		ImFont* m_Font = nullptr;
	};
}
