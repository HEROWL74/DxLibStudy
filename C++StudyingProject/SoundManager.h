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

    // BGM����
    void PlayBGM(SoundType bgm, bool loop = true);
    void StopBGM();
    void PauseBGM();
    void ResumeBGM();

    // **�V�K�ǉ�: BGM��ԃ`�F�b�N�֐�**
    bool IsBGMPlaying() const;
    bool IsBGMPlaying(SoundType bgm) const;

    // SE����
    void PlaySE(SoundType se);
    void StopSE(SoundType se);
    void StopAllSE();

    // �{�����[������
    void SetBGMVolume(float volume); // 0.0f�`1.0f
    void SetSEVolume(float volume);  // 0.0f�`1.0f
    float GetBGMVolume() const { return bgmVolume; }
    float GetSEVolume() const { return seVolume; }

    // �}�X�^�[�{�����[������
    void SetMasterVolume(float volume); // 0.0f�`1.0f
    float GetMasterVolume() const { return masterVolume; }

private:
    SoundManager() : currentBGM(-1), currentBGMType(BGM_TITLE), bgmVolume(0.7f), seVolume(0.8f), masterVolume(0.5f) {}
    ~SoundManager() = default;
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    std::map<SoundType, int> soundHandles;
    std::map<SoundType, std::string> soundPaths;

    int currentBGM;
    SoundType currentBGMType; // **�V�K�ǉ�: ���݂�BGM�̎�ނ��L�^**
    float bgmVolume;
    float seVolume;
    float masterVolume;

    void LoadSound(SoundType type, const std::string& filePath);
    void UpdateVolumes();
    int CalculateVolume(float baseVolume) const;
};