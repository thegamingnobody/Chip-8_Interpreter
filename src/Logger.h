#pragma once
#include "Singleton.h"
#include <string>
#include "TypeDefinitions.h"

namespace Chip8
{
	class Logger : public Singleton<Logger>
	{
	public:
		void Init(bool isEnabled);
		void Log(const std::string& message) const;
		void Log(const std::string& message, opcode address) const;

		void Enable() { m_IsEnabled = true; }
		void Disable() { m_IsEnabled = false; }

		void SetHexMode() { m_HexMode = true; }
		void SetDecMode() { m_HexMode = false; }
	private:
		bool m_IsEnabled = false;
		bool m_HexMode = true;
	};
}
