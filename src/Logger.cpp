#include "Logger.h"
#include <iostream>

void Chip8::Logger::Init(bool isEnabled)
{
	m_IsEnabled = isEnabled;
}

void Chip8::Logger::Log(const std::string& message) const
{
	if (m_IsEnabled)
	{
		std::cout << message << "\n";
	}
}

void Chip8::Logger::Log(const std::string& message, opcode address) const
{
	if (m_IsEnabled)
	{
		if (m_HexMode)
		{
			std::cout << std::hex;
		}
		else
		{
			std::cout << std::dec;
		}

		std::cout << message << " " << address << "\n";
	}
}
