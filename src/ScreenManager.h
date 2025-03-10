#pragma once

#include "Singleton.h"
#include <vector>
#include "TypeDefinitions.h"

namespace Chip8
{
	class ScreenManager : public Singleton<ScreenManager>
	{
	public:
		void Init(int windowWidth, int windowHeight, float windowScale = 1.0f);
		void Render() const;

		//int GetWidthBase() const { return m_WidthBase; }
		//int GetHeightBase() const { return m_HeightBase; }
		//float GetWindowScale() const { return m_WindowScale; }

	private:

	};
}