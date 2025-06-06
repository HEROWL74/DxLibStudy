#include "SoundManager.h"

void SoundManager::Initialize()
{
    // サウンドファイルパスを設定
    soundPaths[BGM_TITLE] = "Sounds/title_bgm.ogg";
    soundPaths[BGM_GAME] = "Sounds/game_bgm.ogg";
    soundPaths[SFX_SELECT] = "Sounds/sfx_select.ogg";
    soundPaths[SFX_BUMP] = "Sounds/sfx_bump.ogg";
    soundPaths[SFX_HURT] = "Sounds/sfx_hurt.ogg";
    soundPaths[SFX_JUMP] = "Sounds/sfx_jump.ogg";
    soundPaths[SFX_COIN] = "Sounds/sfx_coin.ogg";
    soundPaths[SFX_DISAPPEAR] = "Sounds/sfx_disappear.ogg";

    // BGMはストリーミング再生
    SetCreateSoundDataType(DX_SOUNDTYPE_STREAMSTYLE);
    LoadSound(BGM_TITLE, soundPaths[BGM_TITLE]);
    LoadSound(BGM_GAME, soundPaths[BGM_GAME]);

    // SEは展開メモリ再生
    SetCreateSoundDataType(DX_SOUNDDATATYPE_MEMPRESS);
    LoadSound(SFX_SELECT, soundPaths[SFX_SELECT]);
    LoadSound(SFX_BUMP, soundPaths[SFX_BUMP]);
    LoadSound(SFX_HURT, soundPaths[SFX_HURT]);
    LoadSound(SFX_JUMP, soundPaths[SFX_JUMP]);
    LoadSound(SFX_COIN, soundPaths[SFX_COIN]);
    LoadSound(SFX_DISAPPEAR, soundPaths[SFX_DISAPPEAR]);

    // 初期ボリュームを設定
    UpdateVolumes();

    OutputDebugStringA("SoundManager: Initialized successfully\n");
}

void SoundManager::Finalize()
{
    // BGM停止
    StopBGM();

    // 全SE停止
    StopAllSE();

    // サウンドハンドルを解放
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
        // ファイルが見つからない場合はデバッグ出力
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
    // **既に同じBGMが再生中の場合は何もしない**
    if (currentBGM != -1 && currentBGMType == bgm && CheckSoundMem(currentBGM) == 1) {
        return; // 既に再生中なので何もしない
    }

    // 現在のBGMを停止
    if (currentBGM != -1) {
        StopSoundMem(currentBGM);
    }


    // 新しいBGMを再生
    auto it = soundHandles.find(bgm);
    if (it != soundHandles.end() && it->second != -1) {
        int playType = loop ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_NORMAL;
        PlaySoundMem(it->second, playType);
        currentBGM = it->second;
        currentBGMType = bgm; // **現在のBGMタイプを記録**

        // BGMボリュームを設定
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

// **新規追加: BGM再生状態チェック関数の実装**
bool SoundManager::IsBGMPlaying() const
{
    if (currentBGM == -1) return false;

    // DxLibのCheckSoundMem関数で再生状態をチェック
    return CheckSoundMem(currentBGM) == 1;
}

bool SoundManager::IsBGMPlaying(SoundType bgm) const
{
    if (currentBGM == -1) return false;

    // 指定されたBGMタイプが現在再生中かチェック
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
        // SEが既に再生中の場合は停止してから再生
        if (CheckSoundMem(it->second)) {
            StopSoundMem(it->second);
        }

        PlaySoundMem(it->second, DX_PLAYTYPE_BACK);

        // SEボリュームを設定
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
        // BGM以外を停止
        if (pair.second != currentBGM && pair.second != -1) {
            StopSoundMem(pair.second);
        }
    }
}

void SoundManager::SetBGMVolume(float volume)
{
    bgmVolume = max(0.0f, min(1.0f, volume));

    // 現在再生中のBGMにボリュームを適用
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
    // 現在再生中のBGMにボリュームを適用
    if (currentBGM != -1) {
        ChangeVolumeSoundMem(CalculateVolume(bgmVolume), currentBGM);
    }

    // 全SEにボリュームを適用（BGM以外）
    for (const auto& pair : soundHandles) {
        if (pair.second != currentBGM && pair.second != -1) {
            ChangeVolumeSoundMem(CalculateVolume(seVolume), pair.second);
        }
    }
}

int SoundManager::CalculateVolume(float baseVolume) const
{
    // マスターボリュームと個別ボリュームを組み合わせて計算
    float finalVolume = baseVolume * masterVolume;
    return (int)(finalVolume * 255); // DxLib のボリューム範囲は 0-255
}