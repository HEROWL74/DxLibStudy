#pragma once
#include "EnemyBase.h"

class Bee : public EnemyBase {
public:
    Bee(float startX, float startY);
    ~Bee() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // ミツバチ固有の状態
    enum BeeState {
        BEE_RESTING,        // 休憩中
        BEE_HOVERING,       // ホバリング
        BEE_ATTACKING,      // 攻撃中（急降下）
        BEE_RETURNING       // 元の位置に戻る
    };

    BeeState beeState;
    float hoverTimer;           // ホバリングタイマー
    float restTimer;            // 休憩タイマー
    float attackTimer;          // 攻撃タイマー
    float hoverPhase;           // ホバリング位相
    float originalY;            // 初期Y座標
    float targetY;              // 目標Y座標
    float attackStartY;         // 攻撃開始Y座標

    // ミツバチのパラメータ
    static constexpr float HOVER_AMPLITUDE = 20.0f;    // ホバリング振幅
    static constexpr float HOVER_SPEED = 0.05f;        // ホバリング速度
    static constexpr float ATTACK_SPEED = 8.0f;        // 攻撃速度
    static constexpr float RETURN_SPEED = 3.0f;        // 復帰速度
    static constexpr float REST_DURATION = 2.0f;       // 休憩時間
    static constexpr float HOVER_DURATION = 3.0f;      // ホバリング時間
    static constexpr float ATTACK_RANGE_Y = 150.0f;    // 縦方向攻撃範囲

    // ミツバチ固有の行動
    void UpdateBeeMovement();
    void UpdateBeeAnimation();
    void StartHovering();
    void StartAttack();
    void StartReturning();
    void HandlePlayerDetectionBee(Player* player);

    // ミツバチ固有のスプライト取得
    int GetBeeSprite();
    void UpdateAnimation();
};