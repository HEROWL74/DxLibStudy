#pragma once
#pragma once
#include "EnemyBase.h"

class Fly : public EnemyBase {
public:
    Fly(float startX, float startY);
    ~Fly() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // ハエ固有の状態
    enum FlyState {
        FLY_RESTING,        // 休憩中
        FLY_BUZZING,        // ブンブン飛び回る
        FLY_CHASING,        // プレイヤーを追跡
        FLY_FLEEING         // 逃げる
    };

    FlyState flyState;
    float buzzTimer;            // ブンブンタイマー
    float restTimer;            // 休憩タイマー
    float chaseTimer;           // 追跡タイマー
    float buzzPhaseX;           // X方向のブンブン位相
    float buzzPhaseY;           // Y方向のブンブン位相
    float originalX;            // 初期X座標
    float originalY;            // 初期Y座標
    float fleeTimer;            // 逃げタイマー

    // ハエのパラメータ
    static constexpr float BUZZ_AMPLITUDE_X = 30.0f;   // X方向ブンブン振幅
    static constexpr float BUZZ_AMPLITUDE_Y = 15.0f;   // Y方向ブンブン振幅
    static constexpr float BUZZ_SPEED_X = 0.08f;       // X方向ブンブン速度
    static constexpr float BUZZ_SPEED_Y = 0.12f;       // Y方向ブンブン速度
    static constexpr float CHASE_SPEED = 4.0f;         // 追跡速度
    static constexpr float FLEE_SPEED = 6.0f;          // 逃げ速度
    static constexpr float REST_DURATION = 1.0f;       // 休憩時間
    static constexpr float BUZZ_DURATION = 4.0f;       // ブンブン時間
    static constexpr float CHASE_DURATION = 3.0f;      // 追跡時間
    static constexpr float FLEE_DURATION = 2.0f;       // 逃げ時間

    // ハエ固有の行動
    void UpdateFlyMovement();
    void UpdateFlyAnimation();
    void StartBuzzing();
    void StartChasing();
    void StartFleeing();
    void HandlePlayerDetectionFly(Player* player);

    // ハエ固有のスプライト取得
    int GetFlySprite();
    void UpdateAnimation();
};