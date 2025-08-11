#include <AudioManager.h>
#include <SDL.h>
#include <SDL_mixer.h>

void Chip8::AudioManager::Init()
{
    SDL_Init(SDL_INIT_AUDIO);
    Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 1024);
}

void Chip8::AudioManager::Destroy()
{
    if (m_SoundEffect)
    {
        Mix_FreeChunk(m_SoundEffect);
        m_SoundEffect = nullptr;
    }
    
    Mix_CloseAudio();
    SDL_Quit();
}

void Chip8::AudioManager::StartSound()
{
    if (!m_SoundEffect)
    {
        SDL_Log("Failed to load sound effect: %s", Mix_GetError());
        return;
    }
    
	Mix_PlayChannel(-1, m_SoundEffect, -1);
	m_IsSoundEnabled = true;
}

void Chip8::AudioManager::StopSound()
{
    if (m_SoundEffect)
    {
        Mix_HaltChannel(-1);
		m_IsSoundEnabled = false;
	}
}