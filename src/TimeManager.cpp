#include "TimeManager.h"
#include <imgui.h>
#include "Chip-8_Interpreter.h"

void Chip8::TimeManager::Init()
{
	m_DeltaTime = 0.0f;

	m_TargetTimerUpdatesPerSecond = 61;
	m_SecondsPerTimerUpdate = 1.0 / m_TargetTimerUpdatesPerSecond;
	m_TimersTimer = 0.0f;

	m_TargetInstructionsPerSecond = 700;
	m_InstructionsPerFrame = (m_TargetInstructionsPerSecond / m_TargetTimerUpdatesPerSecond);


	m_LastTime = std::chrono::high_resolution_clock::now();
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_CyclesExecuted = 0;
	m_PerSecondStatsDelay = 0.0f;
	m_PerSecondStatsMaxDelay = 1.0f;
}

void Chip8::TimeManager::UpdateTime(bool isGamePaused)
{
	m_CurrentTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = std::chrono::duration<float>(m_CurrentTime - m_LastTime).count();
	m_LastTime = m_CurrentTime;

	if (isGamePaused) return;

	m_TimersTimer += m_DeltaTime;
	m_PerSecondStatsDelay += m_DeltaTime;

	if (m_PerSecondStatsDelay >= m_PerSecondStatsMaxDelay)
	{
		m_AverageCyclesPerSecond = (m_CyclesExecuted / m_PerSecondStatsDelay);
		m_AverageFramesPerSecond = (m_FrameUpdateCount / m_PerSecondStatsDelay);
		m_CyclesExecuted = 0;
		m_FrameUpdateCount = 0;
		m_PerSecondStatsDelay -= m_PerSecondStatsMaxDelay;
	}

}

bool Chip8::TimeManager::ShouldUpdateTimers()
{
	if (m_TimersTimer >= m_SecondsPerTimerUpdate)
	{
		m_TimersTimer -= m_SecondsPerTimerUpdate;
		Chip8::Interpreter::GetInstance().SetDrawFlag(true);
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
	text = ("Instruction count: " + std::to_string(m_CyclesExecuted));
	ImGui::Text(text.c_str());
	text = "Frames/s: " + std::to_string(m_AverageFramesPerSecond);
	ImGui::Text(text.c_str());
	text = ("Frame count: " + std::to_string(m_FrameUpdateCount));
	ImGui::Text(text.c_str());

	ImGui::End();
}
