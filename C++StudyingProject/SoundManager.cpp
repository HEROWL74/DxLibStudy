#include "SoundManager.h"

SoundManager::SoundManager()
{
    currentBGMHandle = -1;
    masterVolume = 0.8f;
    bgmVolume = 0.7f;
    sfxVolume = 0.8f;
    isFading = false;
    fadeTimer = 0.0f;
    fadeDuration = 2.0f;
}

SoundManager::~SoundManager()
{
    Finalize();
}

void SoundManager::Initialize()
{
    // ���ۂ̃Q�[���ł͉����t�@�C����ǂݍ���
    // ����̓f���Ȃ̂ŋ�̎���

    // ��F
    // LoadBGM("title", "Sounds/BGM/title.ogg");
    // LoadBGM("game", "Sounds/BGM/game.ogg");
    // LoadSFX("button", "Sounds/SFX/button.wav");
    // LoadSFX("start", "Sounds/SFX/start.wav");

    ApplyVolumeSettings();
}

void SoundManager::Update()
{
    if (isFading)
    {
        UpdateFade();
    }
}

void SoundManager::LoadBGM(const std::string& name, const std::string& filepath)
{
    int handle = LoadSoundMem(filepath.c_str());
    if (handle != -1)
    {
        bgmHandles[name] = handle;
    }
}

void SoundManager::LoadSFX(const std::string& name, const std::string& filepath)
{
    int handle = LoadSoundMem(filepath.c_str());
    if (handle != -1)
    {
        sfxHandles[name] = handle;
    }
}

void SoundManager::PlayBGM(const std::string& name, bool loop)
{
    auto it = bgmHandles.find(name);
    if (it != bgmHandles.end())
    {
        // ���݂�BGM���~
        if (currentBGMHandle != -1)
        {
            StopSoundMem(currentBGMHandle);
        }

        currentBGM = name;
        currentBGMHandle = it->second;

        // ���[�v�ݒ�
        if (loop)
        {
            PlaySoundMem(currentBGMHandle, DX_PLAYTYPE_LOOP);
        }
        else
        {
            PlaySoundMem(currentBGMHandle, DX_PLAYTYPE_BACK);
        }
    }
    else
    {
        // �����t�@�C����������Ȃ��ꍇ�̃f�o�b�O�p
        // ���ۂ̃Q�[���ł͓K�؂ȃG���[�n���h�����O���s��
    }
}

void SoundManager::StopBGM()
{
    if (currentBGMHandle != -1)
    {
        StopSoundMem(currentBGMHandle);
        currentBGMHandle = -1;
        currentBGM = "";
    }
}

void SoundManager::FadeBGM(const std::string& name, float duration)
{
    if (name != currentBGM)
    {
        nextBGM = name;
        fadeDuration = duration;
        fadeTimer = 0.0f;
        isFading = true;
    }
}

void SoundManager::PlaySFX(const std::string& name)
{
    auto it = sfxHandles.find(name);
    if (it != sfxHandles.end())
    {
        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);
    }
}

void SoundManager::SetMasterVolume(float volume)
{
    masterVolume = volume;
    if (masterVolume < 0.0f) masterVolume = 0.0f;
    if (masterVolume > 1.0f) masterVolume = 1.0f;
    ApplyVolumeSettings();
}

void SoundManager::SetBGMVolume(float volume)
{
    bgmVolume = volume;
    if (bgmVolume < 0.0f) bgmVolume = 0.0f;
    if (bgmVolume > 1.0f) bgmVolume = 1.0f;
    ApplyVolumeSettings();
}

void SoundManager::SetSFXVolume(float volume)
{
    sfxVolume = volume;
    if (sfxVolume < 0.0f) sfxVolume = 0.0f;
    if (sfxVolume > 1.0f) sfxVolume = 1.0f;
    ApplyVolumeSettings();
}

void SoundManager::UpdateFade()
{
    fadeTimer += 1.0f / 60.0f;  // 60FPS�z��

    if (fadeTimer >= fadeDuration)
    {
        // �t�F�[�h����
        isFading = false;
        PlayBGM(nextBGM);
        nextBGM = "";
    }
    else
    {
        // �t�F�[�h���̉��ʒ���
        float fadeRatio = fadeTimer / fadeDuration;
        float currentVolume = (1.0f - fadeRatio) * bgmVolume * masterVolume;

        if (currentBGMHandle != -1)
        {
            ChangeVolumeSoundMem((int)(currentVolume * 255), currentBGMHandle);
        }
    }
}

void SoundManager::ApplyVolumeSettings()
{
    // BGM�̉��ʐݒ�
    if (currentBGMHandle != -1)
    {
        int volume = (int)(masterVolume * bgmVolume * 255);
        ChangeVolumeSoundMem(volume, currentBGMHandle);
    }

    // ���ʉ��̉��ʐݒ�i�S�̓I�ɓK�p�j
    // ���ۂ̃Q�[���ł͌ʂɊǗ����邱�Ƃ�����
}

void SoundManager::Finalize()
{
    // BGM�n���h���̉��
    for (auto& pair : bgmHandles)
    {
        if (pair.second != -1)
        {
            DeleteSoundMem(pair.second);
        }
    }
    bgmHandles.clear();

    // ���ʉ��n���h���̉��
    for (auto& pair : sfxHandles)
    {
        if (pair.second != -1)
        {
            DeleteSoundMem(pair.second);
        }
    }
    sfxHandles.clear();

    currentBGMHandle = -1;
    currentBGM = "";
}