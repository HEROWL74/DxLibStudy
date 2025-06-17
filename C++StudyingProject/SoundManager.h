#pragma once
#include "DxLib.h"
#include <map>
#include <string>

class SoundManager {
public:
    enum SoundType {
        BGM_TITLE,
        BGM_GAME,
        SFX_SELECT,
        SFX_BUMP,
        SFX_HURT,
        SFX_JUMP,
        SFX_COIN,
        SFX_DISAPPEAR,
        SFX_BREAK_BLOCK,
        SFX_HIT
    };

    static SoundManager& GetInstance() {
        static SoundManager instance;
        return instance;
    }

    void Initialize();
    void Finalize();

    // BGM制御
    void PlayBGM(SoundType bgm, bool loop = true);
    void StopBGM();
    void PauseBGM();
    void ResumeBGM();

    // **新規追加: BGM状態チェック関数**
    bool IsBGMPlaying() const;
    bool IsBGMPlaying(SoundType bgm) const;

    // SE制御
    void PlaySE(SoundType se);
    void StopSE(SoundType se);
    void StopAllSE();

    // ボリューム制御
    void SetBGMVolume(float volume); // 0.0f～1.0f
    void SetSEVolume(float volume);  // 0.0f～1.0f
    float GetBGMVolume() const { return bgmVolume; }
    float GetSEVolume() const { return seVolume; }

    // マスターボリューム制御
    void SetMasterVolume(float volume); // 0.0f～1.0f
    float GetMasterVolume() const { return masterVolume; }

private:
    SoundManager() : currentBGM(-1), currentBGMType(BGM_TITLE), bgmVolume(0.7f), seVolume(0.8f), masterVolume(0.5f) {}
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::map<SoundType, int> soundHandles;
    std::map<SoundType, std::string> soundPaths;

    int currentBGM;
    SoundType currentBGMType; // **新規追加: 現在のBGMの種類を記録**
    float bgmVolume;
    float seVolume;
    float masterVolume;

    void LoadSound(SoundType type, const std::string& filePath);
    void UpdateVolumes();
    int CalculateVolume(float baseVolume) const;
};