#pragma once
#include "EnemyBase.h"

class SpikeSlime : public EnemyBase {
public:
    SpikeSlime(float startX, float startY);
    ~SpikeSlime() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

    // **新機能: 外部からトゲの状態を確認**
    bool AreSpikesOut() const { return isSpikesOut; }

    // **新機能: SpikeSlimeの状態を取得**
    enum SpikeSlimeState {
        SPIKE_DORMANT,      // 休眠中
        SPIKE_ALERT,        // 警戒中
        SPIKE_CHARGING,     // チャージ中
        SPIKE_ATTACKING,    // 攻撃中
        SPIKE_COOLDOWN,     // クールダウン
        SPIKE_STUNNED       // スタン状態
    };

    SpikeSlimeState GetSpikeState() const { return spikeState; }

private:
    // トゲスライム固有の状態
    SpikeSlimeState spikeState;
    float chargeTimer;          // チャージタイマー
    float alertTimer;           // 警戒タイマー
    float cooldownTimer;        // クールダウンタイマー
    float stunTimer;            // スタンタイマー
    float pulsePhase;           // 脈動効果用
    bool isSpikesOut;           // トゲが出ているか

    // トゲスライム固有のパラメータ
    static constexpr float CHARGE_DURATION = 1.0f;     // チャージ時間
    static constexpr float ATTACK_DURATION = 0.5f;     // 攻撃時間
    static constexpr float COOLDOWN_DURATION = 2.0f;   // クールダウン時間
    static constexpr float STUN_DURATION = 2.0f;       // スタン時間
    static constexpr float ALERT_DURATION = 0.8f;      // 警戒時間
    static constexpr float PULSE_SPEED = 0.15f;        // 脈動速度
    static constexpr float CHARGE_SPEED = 4.0f;        // チャージ攻撃の速度
    static constexpr float SPIKE_DAMAGE = 15;          // トゲダメージ

    // トゲスライム固有の行動
    void UpdateSpikeSlimeMovement();
    void UpdateSpikeSlimeAnimation();
    void StartChargeAttack(Player* player);
    void ExecuteChargeAttack(Player* player);
    void EnterCooldownState();
    void CheckPlayerInAlertRange(Player* player);
    void HandleStunState();

    // 視覚効果
    void DrawSpikeEffect(float cameraX);

    // トゲスライム固有のスプライト取得
    int GetSpikeSlimeSprite();
};