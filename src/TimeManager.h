#pragma once

#include "Singleton.h"
#include <chrono>

namespace Chip8
{
	class TimeManager: public Singleton<TimeManager>
	{
	public:
		void Init();
		void UpdateTime(bool isGamePaused);

		bool ShouldUpdateTimers();

		std::chrono::milliseconds GetSleepTime();

		float GetInstructionsPerSecond() const { return m_AverageCyclesPerSecond; }

		void IncrementCycleCounter() { m_CyclesExecuted++; }
		int GetInstructionPerFrame() const { return m_InstructionsPerFrame; }

		void IncrementFrameCounter() { m_FrameUpdateCount++; }

		void RenderImGui(std::string windowName);
	private:
		float m_DeltaTime;

		int m_TargetInstructionsPerSecond;
		int m_InstructionsPerFrame;

		int m_TargetTimerUpdatesPerSecond;
		double m_SecondsPerTimerUpdate;
		float m_TimersTimer;

		std::chrono::high_resolution_clock::time_point m_LastTime;
		std::chrono::high_resolution_clock::time_point m_CurrentTime;
		int m_CyclesExecuted;
		int m_FrameUpdateCount;
		float m_PerSecondStatsDelay;
		float m_PerSecondStatsMaxDelay;
		double m_AverageCyclesPerSecond;
		double m_AverageFramesPerSecond;
	};
}
