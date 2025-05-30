#pragma once
#include "DxLib.h"
#include <map>
#include <string>

class SoundManager
{
private:
    // �T�E���h�n���h���̃}�b�v
    std::map<std::string, int> bgmHandles;
    std::map<std::string, int> sfxHandles;

    // ���ݍĐ�����BGM
    std::string currentBGM;
    int currentBGMHandle;

    // ���ʐݒ�
    float masterVolume;
    float bgmVolume;
    float sfxVolume;

    // �t�F�[�h�֘A
    bool isFading;
    float fadeTimer;
    float fadeDuration;
    std::string nextBGM;

public:
    SoundManager();
    ~SoundManager();

    void Initialize();
    void Update();
    void Finalize();

    // BGM�֘A
    void LoadBGM(const std::string& name, const std::string& filepath);
    void PlayBGM(const std::string& name, bool loop = true);
    void StopBGM();
    void FadeBGM(const std::string& name, float duration = 2.0f);

    // ���ʉ��֘A
    void LoadSFX(const std::string& name, const std::string& filepath);
    void PlaySFX(const std::string& name);

    // ���ʐݒ�
    void SetMasterVolume(float volume);
    void SetBGMVolume(float volume);
    void SetSFXVolume(float volume);

    // �Q�b�^�[
    float GetMasterVolume() const { return masterVolume; }
    float GetBGMVolume() const { return bgmVolume; }
    float GetSFXVolume() const { return sfxVolume; }

private:
    void UpdateFade();
    void ApplyVolumeSettings();
};