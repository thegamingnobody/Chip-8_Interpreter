#pragma once
#include "Singleton.h"
#include <string>

namespace Chip8
{
	class Logger : public Singleton<Logger>
	{
	public:
		void Init(bool isEnabled);
		void Log(const std::string& message) const;

		void Enable() { m_IsEnabled = true; }
		void Disable() { m_IsEnabled = false; }
	private:
		bool m_IsEnabled;
	};
}
