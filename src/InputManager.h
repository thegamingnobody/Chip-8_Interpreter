#pragma once

#include "Singleton.h"

namespace Chip8
{
	class InputManager final : public Singleton<InputManager>
	{
	public:
		void Init();
		bool ProcessInput();
		
	};
}