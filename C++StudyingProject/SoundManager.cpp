#include "SoundManager.h"

void SoundManager::Initialize()
{
    // �T�E���h�t�@�C���p�X��ݒ�
    soundPaths[BGM_TITLE] = "Sounds/title_bgm.ogg";
    soundPaths[BGM_GAME] = "Sounds/game_bgm.ogg";
    soundPaths[SFX_SELECT] = "Sounds/sfx_select.ogg";
    soundPaths[SFX_BUMP] = "Sounds/sfx_bump.ogg";
    soundPaths[SFX_HURT] = "Sounds/sfx_hurt.ogg";
    soundPaths[SFX_JUMP] = "Sounds/sfx_jump.ogg";
    soundPaths[SFX_COIN] = "Sounds/sfx_coin.ogg";
    soundPaths[SFX_DISAPPEAR] = "Sounds/sfx_disappear.ogg";

    // BGM�̓X�g���[�~���O�Đ�
    SetCreateSoundDataType(DX_SOUNDTYPE_STREAMSTYLE);
    LoadSound(BGM_TITLE, soundPaths[BGM_TITLE]);
    LoadSound(BGM_GAME, soundPaths[BGM_GAME]);

    // SE�͓W�J�������Đ�
    SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMPRESS);
    LoadSound(SFX_SELECT, soundPaths[SFX_SELECT]);
    LoadSound(SFX_BUMP, soundPaths[SFX_BUMP]);
    LoadSound(SFX_HURT, soundPaths[SFX_HURT]);
    LoadSound(SFX_JUMP, soundPaths[SFX_JUMP]);
    LoadSound(SFX_COIN, soundPaths[SFX_COIN]);
    LoadSound(SFX_DISAPPEAR, soundPaths[SFX_DISAPPEAR]);

    // �����{�����[����ݒ�
    UpdateVolumes();

    OutputDebugStringA("SoundManager: Initialized successfully\n");
}

void SoundManager::Finalize()
{
    // BGM��~
    StopBGM();

    // �SSE��~
    StopAllSE();

    // �T�E���h�n���h�������
    for (auto& pair : soundHandles) {
        if (pair.second != -1) {
            DeleteSoundMem(pair.second);
            pair.second = -1;
        }
    }

    soundHandles.clear();
    currentBGM = -1;

    OutputDebugStringA("SoundManager: Finalized\n");
}

void SoundManager::LoadSound(SoundType type, const std::string& filePath)
{
    int handle = LoadSoundMem(filePath.c_str());

    if (handle == -1) {
        // �t�@�C����������Ȃ��ꍇ�̓f�o�b�O�o��
        std::string debugMsg = "SoundManager: Failed to load " + filePath + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
    else {
        soundHandles[type] = handle;

        std::string debugMsg = "SoundManager: Loaded " + filePath + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
}

void SoundManager::PlayBGM(SoundType bgm, bool loop)
{
    // **���ɓ���BGM���Đ����̏ꍇ�͉������Ȃ�**
    if (currentBGM != -1 && currentBGMType == bgm && CheckSoundMem(currentBGM) == 1) {
        return; // ���ɍĐ����Ȃ̂ŉ������Ȃ�
    }

    // ���݂�BGM���~
    if (currentBGM != -1) {
        StopSoundMem(currentBGM);
    }


    // �V����BGM���Đ�
    auto it = soundHandles.find(bgm);
    if (it != soundHandles.end() && it->second != -1) {
        int playType = loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_NORMAL;
        PlaySoundMem(it->second, playType);
        currentBGM = it->second;
        currentBGMType = bgm; // **���݂�BGM�^�C�v���L�^**

        // BGM�{�����[����ݒ�
        ChangeVolumeSoundMem(CalculateVolume(bgmVolume), it->second);

        std::string debugMsg = "SoundManager: Started BGM type " + std::to_string(bgm) + "\n";
        OutputDebugStringA(debugMsg.c_str());
    }
}

void SoundManager::StopBGM()
{
    if (currentBGM != -1) {
        StopSoundMem(currentBGM);
        currentBGM = -1;
        OutputDebugStringA("SoundManager: BGM stopped\n");
    }
}

// **�V�K�ǉ�: BGM�Đ���ԃ`�F�b�N�֐��̎���**
bool SoundManager::IsBGMPlaying() const
{
    if (currentBGM == -1) return false;

    // DxLib��CheckSoundMem�֐��ōĐ���Ԃ��`�F�b�N
    return CheckSoundMem(currentBGM) == 1;
}

bool SoundManager::IsBGMPlaying(SoundType bgm) const
{
    if (currentBGM == -1) return false;

    // �w�肳�ꂽBGM�^�C�v�����ݍĐ������`�F�b�N
    if (currentBGMType != bgm) return false;

    return CheckSoundMem(currentBGM) == 1;
}

void SoundManager::PauseBGM()
{
    if (currentBGM != -1) {
        StopSoundMem(currentBGM);
    }
    
}

void SoundManager::ResumeBGM()
{
    if (currentBGM != -1) {
        PlaySoundMem(currentBGM, DX_PLAYTYPE_LOOP);
    }
}

void SoundManager::PlaySE(SoundType se)
{
    auto it = soundHandles.find(se);
    if (it != soundHandles.end() && it->second != -1) {
        // SE�����ɍĐ����̏ꍇ�͒�~���Ă���Đ�
        if (CheckSoundMem(it->second)) {
            StopSoundMem(it->second);
        }

        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);

        // SE�{�����[����ݒ�
        ChangeVolumeSoundMem(CalculateVolume(seVolume), it->second);
    }
}

void SoundManager::StopSE(SoundType se)
{
    auto it = soundHandles.find(se);
    if (it != soundHandles.end() && it->second != -1) {
        StopSoundMem(it->second);
    }
}

void SoundManager::StopAllSE()
{
    for (const auto& pair : soundHandles) {
        // BGM�ȊO���~
        if (pair.second != currentBGM && pair.second != -1) {
            StopSoundMem(pair.second);
        }
    }
}

void SoundManager::SetBGMVolume(float volume)
{
    bgmVolume = max(0.0f, min(1.0f, volume));

    // ���ݍĐ�����BGM�Ƀ{�����[����K�p
    if (currentBGM != -1) {
        ChangeVolumeSoundMem(CalculateVolume(bgmVolume), currentBGM);
    }
}

void SoundManager::SetSEVolume(float volume)
{
    seVolume = max(0.0f, min(1.0f, volume));
    UpdateVolumes();
}

void SoundManager::SetMasterVolume(float volume)
{
    masterVolume = max(0.0f, min(1.0f, volume));
    UpdateVolumes();
}

void SoundManager::UpdateVolumes()
{
    // ���ݍĐ�����BGM�Ƀ{�����[����K�p
    if (currentBGM != -1) {
        ChangeVolumeSoundMem(CalculateVolume(bgmVolume), currentBGM);
    }

    // �SSE�Ƀ{�����[����K�p�iBGM�ȊO�j
    for (const auto& pair : soundHandles) {
        if (pair.second != currentBGM && pair.second != -1) {
            ChangeVolumeSoundMem(CalculateVolume(seVolume), pair.second);
        }
    }
}

int SoundManager::CalculateVolume(float baseVolume) const
{
    // �}�X�^�[�{�����[���ƌʃ{�����[����g�ݍ��킹�Čv�Z
    float finalVolume = baseVolume * masterVolume;
    return (int)(finalVolume * 255); // DxLib �̃{�����[���͈͂� 0-255
}