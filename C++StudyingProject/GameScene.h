#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"  // HUDシステムを追加
#include "CoinSystem.h" // コインシステムを追加
#include "GoalSystem.h" // ゴールシステムを追加
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
    Player gamePlayer;
    HUDSystem hudSystem;  // HUDシステムを追加
    CoinSystem coinSystem; // コインシステムを追加
    GoalSystem goalSystem; // ゴールシステムを追加

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterName;

    // カメラシステム
    float cameraX;              // カメラのX座標
    float targetCameraX;        // カメラの目標X座標

    // **追加：カメラのイージング関連**
    float cameraVelocityX;      // カメラの速度
    float previousPlayerX;      // 前フレームのプレイヤーX座標

    // 物理定数
    static constexpr float CAMERA_LERP = 0.08f;     // カメラの追従速度

    // **追加：カメラのイージング定数**
    static constexpr float CAMERA_ACCELERATION = 0.5f;  // カメラの加速度
    static constexpr float CAMERA_FRICTION = 0.92f;     // カメラの摩擦
    static constexpr float CAMERA_MAX_SPEED = 12.0f;    // カメラの最大速度

    // 状態管理
    bool exitRequested;

    // キー入力
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev; // ステージ切り替え用

    // **追加：ゲーム状態管理**
    int playerLife;      // プレイヤーのライフ
    int playerCoins;     // プレイヤーのコイン数
    int currentStageIndex; // 現在のステージインデックス

    // **追加：フェード演出**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // ヘルパー関数
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateGameLogic();  // ゲームロジック更新を追加
    void DrawUI();
    void DrawSeamlessBackground(); // シームレス背景描画を追加

    // **追加：フェード関連の関数**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **追加：HUD関連の関数**
    void InitializeHUD();
    void UpdateHUD();

    // ユーティリティ
    float Lerp(float a, float b, float t);
};