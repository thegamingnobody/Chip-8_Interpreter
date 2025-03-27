#pragma once

#include "Singleton.h"
#include <vector>
#include "TypeDefinitions.h"
#include <string>

namespace Chip8
{
	class InputManager final : public Singleton<InputManager>
	{
	public:
		void Init();
		bool ProcessInput();
		
		bool IsKeyPressed(int key) const;
		bool IsAnyKeyPressed() const;

		bool IsKeyPressedThisFrame() const { return m_KeyPressedThisFrame; }

		void RenderImGui(std::string windowName);

		int GetNumberOfKeys() const { return m_Keymap.size(); }
	private:
		void SetKey(int key, bool newState);

		std::vector<bool> m_KeysState;
		std::vector<byte> m_Keymap;

		bool m_KeyPressedThisFrame{};
	};
}