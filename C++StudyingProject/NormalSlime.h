#pragma once
#include "EnemyBase.h"

class NormalSlime : public EnemyBase {
public:
    NormalSlime(float startX, float startY);
    ~NormalSlime() override = default;

    // 純粋仮想関数の実装
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // オーバーライド
    void OnPlayerCollision(Player* player) override;
    void Draw(float cameraX) override; // **新追加：潰れアニメーション用**
    int GetCurrentSprite() override;

private:
    // ノーマルスライム固有の状態
    enum SlimeState {
        SLIME_RESTING,      // 休憩中
        SLIME_ACTIVE,       // アクティブ
        SLIME_JUMPING,      // ジャンプ中
        SLIME_FLATTENED     // 潰れた状態
    };

    SlimeState slimeState;
    float jumpTimer;            // ジャンプタイマー
    float restTimer;            // 休憩タイマー
    float flattenedTimer;       // 潰れ状態タイマー
    float bobPhase;             // 上下揺れ用

    // **新追加：潰れアニメーション用変数**
    float flattenScale;         // 潰れた時のスケール
    float flattenProgress;      // 潰れアニメーションの進行度

    // スライム固有のパラメータ
    static constexpr float JUMP_POWER = -12.0f;
    static constexpr float JUMP_INTERVAL = 2.0f;    // ジャンプ間隔
    static constexpr float REST_DURATION = 1.5f;    // 休憩期間
    static constexpr float FLATTENED_DISPLAY_DURATION = 1.5f; // **潰れ表示期間（延長）**
    static constexpr float BOB_SPEED = 0.08f;        // 揺れ速度
    static constexpr float BOB_AMPLITUDE = 3.0f;    // 揺れ幅

    // スライム固有の行動
    void UpdateSlimeMovement();
    void UpdateSlimeAnimation();
    void StartJump();
    void HandleJumpMovement(Player* player);
    void CheckFlattenedRecovery();

    // **新追加：潰れアニメーション更新**
    void UpdateFlattenedAnimation();

    // スライム固有のスプライト取得
    int GetSlimeSprite();
};