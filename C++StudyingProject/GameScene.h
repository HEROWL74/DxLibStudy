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

    // カメラシステム（滑らかな追従用に簡素化）
    float cameraX;              // カメラのX座標
    float previousPlayerX;      // 前フレームのプレイヤーX座標

    // **カメラの滑らか追従定数（シンプル化）**
    static constexpr float CAMERA_FOLLOW_SPEED = 0.05f;  // カメラ追従速度（遅めで滑らか）

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
    void UpdateCameraSimple();  // シンプル版カメラ（オプション）
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
    float SmoothLerp(float current, float target, float speed);  // 滑らかなイージング用
};