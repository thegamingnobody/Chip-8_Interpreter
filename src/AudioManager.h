#pragma once
#include "Singleton.h"  


namespace Chip8  
{  
	class AudioManager : public Singleton<AudioManager>
	{
	public:
		void Init();
		void Destroy();

		void StartSound();
		void StopSound();

		bool IsSoundEnabled() const { return m_IsSoundEnabled; }

	private:
		Mix_Chunk* m_SoundEffect{ nullptr };
		bool m_IsSoundEnabled{ false };
	};

}