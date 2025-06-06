#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"    
#include "CoinSystem.h"   
#include "GoalSystem.h"   
#include "StarSystem.h"   
#include "ResultUISystem.h" 
#include "EnemyManager.h" 
#include "DoorSystem.h"   // **新追加：ドアシステム**
#include "SoundManager.h"
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
    bool IsStageChangeRequested() const { return stageChangeRequested; }
    int GetRequestedStageIndex() const { return requestedStageIndex; }

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
    HUDSystem hudSystem;
    CoinSystem coinSystem;
    GoalSystem goalSystem;
    StarSystem starSystem;
    ResultUISystem resultUI;
    EnemyManager enemyManager;
    DoorSystem doorSystem;      // **新追加：ドアシステム**

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterName;

    // カメラシステム
    float cameraX;
    float previousPlayerX;

    // **カメラの滑らか追従定数**
    static constexpr float CAMERA_FOLLOW_SPEED = 0.05f;

    // 状態管理
    bool exitRequested;

    // キー入力
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev;

    // **ゲーム状態管理**
    int playerLife;
    int playerCoins;
    int playerStars;
    int currentStageIndex;

    // **フェード演出**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // **リザルト表示制御**
    bool showingResult;
    bool goalReached;

    // **新追加：ドア関連の状態管理**
    bool doorOpened;            // ドアが開いたか
    bool playerEnteringDoor;    // プレイヤーがドアに入っているか


    // **新追加：遅延自動歩行システム**
    bool pendingAutoWalk;       // 自動歩行が待機中か
    int autoWalkDelayFrames;    // 自動歩行開始までの遅延フレーム数
    // **敵との相互作用用管理**
    bool playerInvulnerable;
    float invulnerabilityTimer;
    static constexpr float INVULNERABILITY_DURATION = 2.0f;

    // **新追加：遅延自動歩行関数の宣言**
    void UpdateDelayedAutoWalk();

    // ヘルパー関数
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateCameraSimple();
    void UpdateGameLogic();
    void DrawUI();
    void DrawSeamlessBackground();

    // **フェード関数**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **HUD機能関数**
    void InitializeHUD();
    void UpdateHUD();

    // **リザルト機能関数**
    void UpdateResult();
    void ShowStageResult();
    void HandleResultButtons();

    // **敵との相互作用用関数**
    void UpdatePlayerEnemyInteractions();
    void HandlePlayerDamage(int damage);
    void HandlePlayerEnemyCollision();
    void UpdatePlayerInvulnerability();

    bool CheckIfPlayerStompedEnemy();
    void HandleSuccessfulStomp();
    void ApplyStompBounce();

    void UpdatePlayerAutoWalk();

    // **新追加：ドア関連の処理関数**
    void UpdateDoorInteraction();
    void HandleGoalReached();
    void HandlePlayerEnteredDoor();

    // ユーティリティ
    float Lerp(float a, float b, float t);
    float SmoothLerp(float current, float target, float speed);

    bool stageChangeRequested;
    int requestedStageIndex;

};