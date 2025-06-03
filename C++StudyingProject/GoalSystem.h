#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <string>

class GoalSystem {
public:
    // 旗の色
    enum FlagColor {
        FLAG_BLUE,
        FLAG_YELLOW,
        FLAG_GREEN,
        FLAG_RED
    };

    // ゴールの状態
    enum GoalState {
        GOAL_ACTIVE,    // アクティブ状態
        GOAL_TOUCHED,   // プレイヤーが触れた
        GOAL_COMPLETED  // ゴール完了
    };

    GoalSystem();
    ~GoalSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // ゴール管理
    void SetGoal(float x, float y, FlagColor color);
    void ClearGoal();
    bool IsGoalTouched() const { return goalState == GOAL_TOUCHED || goalState == GOAL_COMPLETED; }
    bool IsGoalCompleted() const { return goalState == GOAL_COMPLETED; }
    void ResetGoal() { goalState = GOAL_ACTIVE; goalTouchTimer = 0.0f; }

    // 現在のステージにゴールを自動配置
    void PlaceGoalForStage(int stageIndex, StageManager* stageManager);

private:
    // テクスチャハンドル
    struct FlagTextures {
        int blue_a, blue_b;
        int yellow_a, yellow_b;
        int green_a, green_b;
        int red_a, red_b;
    } flagTextures;

    // ゴール情報
    float goalX, goalY;         // ゴール位置
    FlagColor currentColor;     // 現在の旗の色
    GoalState goalState;        // ゴールの状態
    bool goalExists;            // ゴールが存在するか

    // アニメーション
    float animationTimer;       // アニメーションタイマー
    bool currentFrame;          // 現在のフレーム（false=a, true=b）
    float goalTouchTimer;       // ゴールタッチ後のタイマー

    // エフェクト
    float glowIntensity;        // グロー効果の強度
    float bobPhase;             // 上下浮遊効果

    // 定数
    static constexpr float FLAG_WIDTH = 64.0f;          // 旗の幅
    static constexpr float FLAG_HEIGHT = 96.0f;         // 旗の高さ
    static constexpr float GOAL_DETECTION_WIDTH = 80.0f; // ゴール判定幅
    static constexpr float GOAL_DETECTION_HEIGHT = 120.0f; // ゴール判定高さ
    static constexpr float ANIMATION_SPEED = 0.02f;     // アニメーション速度（ゆっくり）
    static constexpr float BOB_SPEED = 0.03f;           // 浮遊速度
    static constexpr float BOB_AMPLITUDE = 4.0f;        // 浮遊振幅
    static constexpr float GOAL_TOUCH_DURATION = 2.0f;  // ゴールタッチ演出時間

    // ヘルパー関数
    void LoadTextures();
    int GetCurrentFlagTexture();
    void UpdateAnimation();
    void UpdateEffects();
    bool CheckPlayerCollision(Player* player);
    void DrawGoalFlag(float cameraX);
    void DrawGoalEffects(float cameraX);
    float GetDistance(float x1, float y1, float x2, float y2);

    // ブロックの上に配置するためのヘルパー
    float FindGroundLevel(float x, StageManager* stageManager);
};