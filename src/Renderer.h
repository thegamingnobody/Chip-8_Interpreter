#pragma once

#include "Singleton.h"
#include <SDL3/SDL.h>

namespace Chip8
{
	class Renderer : public Singleton<Renderer>
	{
	public:
		void Init(int windowWidth, int windowHeight);
		void Render() const;
		void Destroy();

		void SetBackgroundColor(const SDL_Color& newColor) { m_BackgroundColor = newColor; }
		const SDL_Color& GetBackgroundColor() const { return m_BackgroundColor; }

	private:
		SDL_Window* m_Window{};
		SDL_Renderer* m_Renderer{};
		SDL_Color m_BackgroundColor{};
	};
}
