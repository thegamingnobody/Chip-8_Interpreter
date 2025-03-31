#pragma once

#include "Singleton.h"
#include <SDL3/SDL.h>
#include <vector>
#include <imgui.h>
#include "TypeDefinitions.h"

namespace Chip8
{
	class Renderer : public Singleton<Renderer>
	{
	public:
		void Init(float windowScale = 1.0f);
		Chip8::EmulatorStates Render(Chip8::EmulatorStates emulatorState) const;
		void Update();
		void Destroy();

		void SetBackgroundColor(const SDL_Color& newColor) { m_BackgroundColor = newColor; }
		const SDL_Color& GetBackgroundColor() const { return m_BackgroundColor; }

		float GetWindowScale() const { return m_WindowScale; }

		float GetViewportScale() const { return m_ViewportScale; }

		void SetPixel(int x, int y, bool value);
		void TogglePixel(int x, int y);
		bool IsPixelOn(int x, int y) const;

		void ClearScreen();

		EmulatorStates RenderImgui(Chip8::EmulatorStates emulatorState) const;

	private:
		void UpdateRenderTexture() const;

		float m_WindowScale;

		float m_ViewportScale;

		SDL_Window* m_Window{};
		SDL_Renderer* m_Renderer{};
		SDL_Texture* m_RenderTexture{};
		SDL_Color m_BackgroundColor{};

		//Screen
		std::vector<std::vector<bool>> m_Screen;
		//std::vector<Pixel> m_ChangedPixels;
		ImFont* m_Font = nullptr;
	};
}
