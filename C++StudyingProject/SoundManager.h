#pragma once
#include "DxLib.h"
#include <map>
#include <string>

class SoundManager
{
private:
    // サウンドハンドルのマップ
    std::map<std::string, int> bgmHandles;
    std::map<std::string, int> sfxHandles;

    // 現在再生中のBGM
    std::string currentBGM;
    int currentBGMHandle;

    // 音量設定
    float masterVolume;
    float bgmVolume;
    float sfxVolume;

    // フェード関連
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

    // BGM関連
    void LoadBGM(const std::string& name, const std::string& filepath);
    void PlayBGM(const std::string& name, bool loop = true);
    void StopBGM();
    void FadeBGM(const std::string& name, float duration = 2.0f);

    // 効果音関連
    void LoadSFX(const std::string& name, const std::string& filepath);
    void PlaySFX(const std::string& name);

    // 音量設定
    void SetMasterVolume(float volume);
    void SetBGMVolume(float volume);
    void SetSFXVolume(float volume);

    // ゲッター
    float GetMasterVolume() const { return masterVolume; }
    float GetBGMVolume() const { return bgmVolume; }
    float GetSFXVolume() const { return sfxVolume; }

private:
    void UpdateFade();
    void ApplyVolumeSettings();
};