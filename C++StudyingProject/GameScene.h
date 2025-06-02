#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include <string>

class GameScene
{
public:
    GameScene();
    ~GameScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // テクスチャハンドル
    int backgroundHandle;
    int fontHandle;

    // ゲームオブジェクト
    StageManager stageManager;
    Player gamePlayer;  // 'player'から'gamePlayer'に変更

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterName;

    // カメラシステム
    float cameraX;              // カメラのX座標
    float targetCameraX;        // カメラの目標X座標

    // 物理定数
    static constexpr float CAMERA_LERP = 0.08f;     // カメラの追従速度

    // 状態管理
    bool exitRequested;

    // キー入力
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev; // ステージ切り替え用

    // ヘルパー関数
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void DrawUI();

    // ユーティリティ
    float Lerp(float a, float b, float t);
};