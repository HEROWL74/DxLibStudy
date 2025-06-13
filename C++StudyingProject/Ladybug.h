#pragma once
#include "EnemyBase.h"

class Ladybug : public EnemyBase {
public:
    Ladybug(float startX, float startY);
    ~Ladybug() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // テントウムシ固有の状態
    enum LadybugState {
        LADYBUG_RESTING,    // 休憩中
        LADYBUG_WALKING,    // 歩行中
        LADYBUG_FLYING,     // 飛行中
        LADYBUG_LANDING     // 着地中
    };

    LadybugState ladybugState;
    float restTimer;            // 休憩タイマー
    float walkTimer;            // 歩行タイマー
    float flyTimer;             // 飛行タイマー
    float flyPhase;             // 飛行位相
    float targetY;              // 目標Y座標
    float originalY;            // 地上Y座標

    // テントウムシのパラメータ
    static constexpr float FLY_AMPLITUDE = 25.0f;      // 飛行振幅
    static constexpr float FLY_SPEED_PHASE = 0.1f;     // 飛行位相速度
    static constexpr float FLY_SPEED = 3.0f;           // 飛行速度
    static constexpr float WALK_SPEED = 1.5f;          // 歩行速度
    static constexpr float REST_DURATION = 2.0f;       // 休憩時間
    static constexpr float WALK_DURATION = 3.0f;       // 歩行時間
    static constexpr float FLY_DURATION = 4.0f;        // 飛行時間
    static constexpr float FLY_HEIGHT = 60.0f;         // 飛行高度

    // テントウムシ固有の行動
    void UpdateLadybugMovement();
    void UpdateLadybugAnimation();
    void StartWalking();
    void StartFlying();
    void StartLanding();
    void HandleGroundCollisionLadybug(StageManager* stageManager);

    // テントウムシ固有のスプライト取得
    int GetLadybugSprite();
};