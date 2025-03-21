#include "TimeManager.h"
#include <imgui.h>

void Chip8::TimeManager::Init()
{
	m_DeltaTime = 0.0f;

	m_TargetInstructionsPerSecond = 700;
	m_InstructionsPerFrame = m_TargetInstructionsPerSecond / 60;
	m_InstructionMsPerFrame = 1.0f / m_TargetInstructionsPerSecond / m_InstructionsPerFrame;

	m_TargetTimerUpdatesPerSecond = 60;
	m_SecondsPerTimerUpdate = 1.0 / m_TargetTimerUpdatesPerSecond;
	m_TimersTimer = 0.0f;

	m_LastTime = std::chrono::high_resolution_clock::now();
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_CyclesExecuted = 0;
	m_PerSecondStatsDelay = 0.0f;
	m_PerSecondStatsMaxDelay = 1.0f;
}

void Chip8::TimeManager::UpdateTime()
{
	m_CurrentTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = std::chrono::duration<float>(m_CurrentTime - m_LastTime).count();
	m_LastTime = m_CurrentTime;
	m_TimersTimer += m_DeltaTime;
	m_PerSecondStatsDelay += m_DeltaTime;

	if (m_PerSecondStatsDelay >= m_PerSecondStatsMaxDelay)
	{
		m_AverageCyclesPerSecond = (m_CyclesExecuted / m_PerSecondStatsDelay);
		m_CyclesExecuted = 0;
		m_PerSecondStatsDelay -= m_PerSecondStatsMaxDelay;
	}

}

bool Chip8::TimeManager::ShouldUpdateTimers()
{
	if (m_TimersTimer >= m_SecondsPerTimerUpdate)
	{
		m_TimersTimer -= m_SecondsPerTimerUpdate;
		return true;
	}
	else
	{
		return false;
	}
}

std::chrono::milliseconds Chip8::TimeManager::GetSleepTime()
{
	//Todo: replace magic number td::chrono::milliseconds(15) with variable
	auto sleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::milliseconds(15) - (std::chrono::high_resolution_clock::now() - m_CurrentTime));
	return sleepTime;
}

void Chip8::TimeManager::RenderImGui(std::string windowName)
{
	ImGui::Begin(windowName.c_str(), nullptr);

	std::string text{ "Instructions/s: " + std::to_string(GetInstructionsPerSecond()) };
	ImGui::Text(text.c_str());

	ImGui::End();
}
