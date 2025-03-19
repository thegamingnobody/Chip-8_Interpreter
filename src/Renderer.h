#pragma once

#include "Singleton.h"
#include <SDL3/SDL.h>
#include <vector>

namespace Chip8
{
	class Renderer : public Singleton<Renderer>
	{
	public:
		void Init(int windowWidth, int windowHeight, float windowScale = 1.0f);
		void Render() const;
		void Destroy();

		void SetBackgroundColor(const SDL_Color& newColor) { m_BackgroundColor = newColor; }
		const SDL_Color& GetBackgroundColor() const { return m_BackgroundColor; }

		int GetWidth() const { return m_WidthBase; }
		int GetHeight() const { return m_HeightBase; }
		float GetScale() const { return m_WindowScale; }

		void SetPixel(int x, int y, bool value);
		void TogglePixel(int x, int y);
		bool IsPixelOn(int x, int y) const;

		void ClearScreen();

		void RenderImgui() const;

	private:
		int m_WidthBase;
		int m_HeightBase;
		float m_WindowScale;

		SDL_Window* m_Window{};
		SDL_Renderer* m_Renderer{};
		SDL_Texture* m_RenderTexture{};
		SDL_Color m_BackgroundColor{};

		//Screen
		std::vector<std::vector<bool>> m_Screen;
	};
}
