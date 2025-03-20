#pragma once

#include "Singleton.h"
#include <vector>
#include "TypeDefinitions.h"

namespace Chip8
{
	class InputManager final : public Singleton<InputManager>
	{
	public:
		void Init();
		bool ProcessInput();
		
		bool IsKeyPressed(int key) const;

		bool IsKeyPressedThisFrame() const { return m_KeyPressedThisFrame; }

	private:
		//void CheckKeys();
		void SetKey(int key, bool newState);

		std::vector<bool> m_KeysState;
		std::vector<byte> m_Keymap;

		bool m_KeyPressedThisFrame;
	};
}