#pragma once
#include "EnemyBase.h"

class Saw : public EnemyBase {
public:
    Saw(float startX, float startY);
    ~Saw() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // ノコギリ固有の状態
    enum SawState {
        SAW_RESTING,        // 休憩中
        SAW_SPINNING,       // 回転中
        SAW_MOVING,         // 移動中
        SAW_ATTACKING       // 攻撃中
    };

    SawState sawState;
    float restTimer;            // 休憩タイマー
    float spinTimer;            // 回転タイマー
    float moveTimer;            // 移動タイマー
    float attackTimer;          // 攻撃タイマー
    float rotationAngle;        // 回転角度
    float attackDirection;      // 攻撃方向

    // ノコギリのパラメータ
    static constexpr float SPIN_SPEED = 0.3f;          // 回転速度
    static constexpr float MOVE_SPEED = 3.0f;          // 移動速度
    static constexpr float ATTACK_SPEED = 6.0f;        // 攻撃速度
    static constexpr float REST_DURATION = 1.5f;       // 休憩時間
    static constexpr float SPIN_DURATION = 2.0f;       // 回転時間
    static constexpr float MOVE_DURATION = 3.0f;       // 移動時間
    static constexpr float ATTACK_DURATION = 1.0f;     // 攻撃時間
    static constexpr float ATTACK_DISTANCE = 100.0f;   // 攻撃距離

    // ノコギリ固有の行動
    void UpdateSawMovement();
    void UpdateSawAnimation();
    void StartSpinning();
    void StartMoving();
    void StartAttacking();
    void HandlePlayerDetectionSaw(Player* player);

    // ノコギリ固有のスプライト取得
    int GetSawSprite();
    void TakeDamage(int damage);
};