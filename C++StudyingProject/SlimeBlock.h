#pragma once
#include "EnemyBase.h"

class SlimeBlock : public EnemyBase {
public:
    SlimeBlock(float startX, float startY);
    ~SlimeBlock() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    void Draw(float cameraX) override;
    void TakeDamage(int damage);
    int GetCurrentSprite() override;

private:
    // ブロックスライム固有の状態
    enum SlimeBlockState {
        BLOCK_RESTING,      // 休憩中
        BLOCK_ACTIVE,       // アクティブ
        BLOCK_JUMPING,      // ジャンプ中
        BLOCK_HARDENED,     // 硬化状態
        BLOCK_BOUNCING      // バウンス中
    };

    SlimeBlockState blockState;
    float restTimer;            // 休憩タイマー
    float jumpTimer;            // ジャンプタイマー
    float hardenTimer;          // 硬化タイマー
    float bounceTimer;          // バウンスタイマー
    float bobPhase;             // 上下揺れ位相
    bool isHardened;            // 硬化フラグ

    // ブロックスライムのパラメータ
    static constexpr float JUMP_POWER = -14.0f;        // ジャンプ力
    static constexpr float JUMP_INTERVAL = 3.0f;       // ジャンプ間隔
    static constexpr float REST_DURATION = 2.0f;       // 休憩時間
    static constexpr float HARDEN_DURATION = 1.5f;     // 硬化時間
    static constexpr float BOUNCE_FORCE = -15.0f;      // バウンス力
    static constexpr float BOB_SPEED = 0.06f;          // 揺れ速度
    static constexpr float BOB_AMPLITUDE = 2.0f;       // 揺れ振幅

    // ブロックスライム固有の行動
    void UpdateSlimeBlockMovement();
    void UpdateSlimeBlockAnimation();
    void StartJumping();
    void StartHardening();
    void HandlePlayerBounce(Player* player);

    // ブロックスライム固有のスプライト取得
    int GetSlimeBlockSprite();
};