#pragma once

#include "Singleton.h"
#include <vector>

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
		void CheckKeys();
		void SetKey(int key);

		std::vector<bool> m_KeysState;

		bool m_KeyPressedThisFrame;
	};
}