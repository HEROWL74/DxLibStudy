#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"    // HUDシステムを追加
#include "CoinSystem.h"   // コインシステムを追加
#include "GoalSystem.h"   // ゴールシステムを追加
#include "StarSystem.h"   // **星システムを追加（新機能）**
#include "ResultUISystem.h" // **リザルトUIシステムを追加（新機能）**
#include "EnemyManager.h" // **敵管理システムを追加（新機能）**
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
    HUDSystem hudSystem;        // HUDシステムを追加
    CoinSystem coinSystem;      // コインシステムを追加
    GoalSystem goalSystem;      // ゴールシステムを追加
    StarSystem starSystem;      // **星システムを追加（新機能）**
    ResultUISystem resultUI;    // **リザルトUIシステムを追加（新機能）**
    EnemyManager enemyManager;  // **敵管理システムを追加（新機能）**

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterName;

    // カメラシステム（滑らかな追従用に簡単化）
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
    int playerStars;     // **プレイヤーの星数（新機能）**
    int currentStageIndex; // 現在のステージインデックス

    // **追加：フェード演出**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // **リザルト表示制御（新機能）**
    bool showingResult;
    bool goalReached;

    // **敵との相互作用管理（新機能）**
    bool playerInvulnerable;     // プレイヤーの無敵状態
    float invulnerabilityTimer;  // 無敵時間タイマー
    static constexpr float INVULNERABILITY_DURATION = 2.0f; // 無敵時間

    // ヘルパー関数
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateCameraSimple();  // シンプル版カメラ（オプション）
    void UpdateGameLogic();  // ゲームロジック更新を追加
    void DrawUI();
    void DrawSeamlessBackground(); // シームレス背景描画を追加

    // **追加：フェード関数**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **追加：HUD関連関数**
    void InitializeHUD();
    void UpdateHUD();

    // **追加：リザルト関連関数（新機能）**
    void UpdateResult();
    void ShowStageResult();
    void HandleResultButtons();

    // **追加：敵との相互作用関数（新機能）**
    void UpdatePlayerEnemyInteractions();
    void HandlePlayerDamage(int damage);
    void HandlePlayerEnemyCollision();
    void UpdatePlayerInvulnerability();

    bool CheckIfPlayerStompedEnemy();
    void HandleSuccessfulStomp();
    void ApplyStompBounce();

    // ユーティリティ
    float Lerp(float a, float b, float t);
    float SmoothLerp(float current, float target, float speed);  // 滑らかなイージング用
};