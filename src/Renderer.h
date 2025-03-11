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

		void SetPixel(int x, int y, bool value);

		void ClearScreen();

	private:
		void DrawGrid();

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
