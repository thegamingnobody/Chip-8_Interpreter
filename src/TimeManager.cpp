#include "TimeManager.h"
#include <imgui.h>
#include "Chip-8_Interpreter.h"
#include <iostream>

void Chip8::TimeManager::Init()
{
	m_DeltaTime = 0.0f;

	m_TargetTimerUpdatesPerSecond = 60;

	m_TargetIPF = 11;

	m_LastTime = std::chrono::high_resolution_clock::now();
	m_CurrentTime = std::chrono::high_resolution_clock::now();

	m_CyclesExecuted = 0;
	m_ImguiStatsUpdateDelay = 0.0f;
	m_ImguiStatsUpdateMaxDelay = 1.0f;
}

void Chip8::TimeManager::UpdateTime(bool isGamePaused)
{
	m_CurrentTime = std::chrono::high_resolution_clock::now();
	m_DeltaTime = std::chrono::duration<float>(m_CurrentTime - m_LastTime).count();
	m_LastTime = m_CurrentTime;

	if (isGamePaused) return;

	m_ImguiStatsUpdateDelay += m_DeltaTime;

	if (m_ImguiStatsUpdateDelay >= m_ImguiStatsUpdateMaxDelay)
	{
		m_AverageCyclesPerSecond = (m_CyclesExecuted / m_ImguiStatsUpdateDelay);
		m_AverageFramesPerSecond = (m_FrameUpdateCount / m_ImguiStatsUpdateDelay);
		m_CyclesExecuted = 0;
		m_FrameUpdateCount = 0;

		m_ImguiStatsUpdateDelay -= m_ImguiStatsUpdateMaxDelay;
	}

}

std::chrono::milliseconds Chip8::TimeManager::GetSleepTime()
{
	//Todo: replace magic number td::chrono::milliseconds(15) with variable
	m_SleepTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::milliseconds(16) - (std::chrono::high_resolution_clock::now() - m_CurrentTime));
	return m_SleepTime;
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
	text = ("Sleep Time: " + std::to_string(m_SleepTime.count()));
	ImGui::Text(text.c_str());

	ImGui::End();
}
