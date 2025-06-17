#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include <string>

class BlockSystem;

class Player {
public:
    // プレイヤー状態
    enum State {
        IDLE,
        WALKING,
        JUMPING,
        FALLING,
        DUCKING,
        SLIDING,
        HIT
    };

    // プレイヤーサイズ定数
    static const int PLAYER_WIDTH = 128;
    static const int PLAYER_HEIGHT = 128;

    Player();
    ~Player();

    void Initialize(int characterIndex);
    void Update(StageManager* stageManager);
    void Draw(float cameraX);
    void DrawShadow(float cameraX, StageManager* stageManager);
    void DrawShadowDebugInfo(float cameraX, int shadowX, int shadowY, float distanceToGround, float normalizedDistance);
    void DrawDebugInfo(float cameraX);

    // ゲッター
    float GetX() const { return x; }
    float GetY() const { return y; }
    bool IsFacingRight() const { return facingRight; }
    State GetState() const { return currentState; }
    float GetVelocityX() const { return velocityX; }
    float GetVelocityY() const { return velocityY; }
    bool IsOnGround() const { return onGround; }

    // セッター
    void SetPosition(float newX, float newY);
    void ResetPosition();

    // 新規追加: ブロックシステム拡張用の関数
    void SetOnGround(bool ground) { onGround = ground; }
    void SetState(State newState) { currentState = newState; }

    // 踏みつけ効果
    void ApplyStompBounce(float bounceVelocity = -8.0f);

    // ダメージシステム
    void TakeDamage(int damage, float knockbackDirection = 0.0f);
    void ApplyGravityOnly(StageManager* stageManager);
    void HandleGroundCollisionOnly(StageManager* stageManager);
    void UpdateAnimationOnly();
    bool IsInvulnerable() const { return invulnerabilityTimer > 0.0f; }
    float GetInvulnerabilityTimer() const { return invulnerabilityTimer; }

    // デバッグ用
    bool IsStomping() const { return wasStomping; }

    // 自動歩行用
    void SetFacingRight(bool facing) { facingRight = facing; }
    void SetAutoWalking(bool autoWalk) { isAutoWalking = autoWalk; }
    bool IsAutoWalking() const { return isAutoWalking; }
    void UpdateAutoWalkPhysics(StageManager* stageManager);

    // スライディング関連
    bool IsSliding() const { return currentState == SLIDING; }
    float GetSlidingProgress() const { return slidingTimer / SLIDING_DURATION; }

    // 速度設定（ブロック衝突後用）
    void SetVelocityX(float velX) { velocityX = velX; }
    void SetVelocityY(float velY) { velocityY = velY; }

    // GameSceneから呼び出される複合衝突処理
    void HandleCollisionsWithBlocks(StageManager* stageManager, class BlockSystem* blockSystem);
    void UpdatePhysics(StageManager* stageManager);
    void UpdateAnimation();
    void HandleCollisions(StageManager* stageManager);

private:
    // キャラクタースプライト構造体
    struct CharacterSprites {
        int front, idle, walk_a, walk_b, jump, duck, hit, climb_a, climb_b;
    };

    // 改良された物理定数 - 適切な速度バランスとふわっとジャンプ
        static constexpr float GRAVITY = 0.35f;             // ふわっとした重力
    static constexpr float JUMP_POWER = -10.0f;         // 強化されたジャンプ力
    static constexpr float MAX_FALL_SPEED = 9.0f;       // 落下速度上限を下げる（ストンッと防止）

    // 水平移動速度を調整（遅くする）
    static constexpr float MOVE_SPEED = 3.0f;           // 4.0f → 3.0f に減速
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float FRICTION = 0.88f;            // 0.85f → 0.88f に増加（止まりやすく）
    static constexpr float ACCELERATION = 0.6f;         // 0.8f → 0.6f に減少（加速を緩やかに）
    static constexpr float MAX_HORIZONTAL_SPEED = 8.0f; // 10.0f → 8.0f に減速

    // ジャンプ関連の詳細調整（ふわっと効果強化）
    static constexpr float JUMP_RELEASE_MULTIPLIER = 0.3f;  // 0.5f → 0.3f（ボタン離し時の減速を強化）
    static constexpr float APEX_GRAVITY_REDUCTION = 0.4f;   // 0.6f → 0.4f（頂点付近での重力大幅軽減）
    static constexpr float APEX_THRESHOLD = 3.0f;           // 2.0f → 3.0f（頂点判定の範囲を拡大）

    // 段階的加速度システムの定数（既存）
    static constexpr float SLIDING_DURATION = 1.2f;
    static constexpr float SLIDING_INITIAL_SPEED = 12.0f;
    static constexpr float SLIDING_DECELERATION = 0.92f;
    static constexpr float SLIDING_MIN_SPEED = 1.0f;
    static constexpr float SLIDING_HEIGHT_REDUCTION = 0.6f;

    // ダメージシステム定数（既存）
    static constexpr float HIT_DURATION = 0.8f;
    static constexpr float INVULNERABILITY_DURATION = 2.0f;
    static constexpr float KNOCKBACK_FORCE = 6.0f;
    static constexpr float KNOCKBACK_VERTICAL = -4.0f;

    // プレイヤー状態
    float x, y;
    float velocityX, velocityY;
    bool facingRight;
    State currentState;
    bool onGround;

    // アニメーション
    float animationTimer;
    bool walkAnimFrame;
    float bobPhase;

    // キャラクター情報
    int characterIndex;
    CharacterSprites sprites;
    std::string characterColorName;

    // 踏みつけ状態管理用
    bool wasStomping;
    float stompCooldown;
    static constexpr float STOMP_COOLDOWN_DURATION = 0.2f;

    // ダメージシステム変数
    float hitTimer;
    float invulnerabilityTimer;
    float knockbackDecay;

    // スライディングシステム変数
    float slidingTimer;
    float slidingSpeed;
    bool wasRunningWhenSlideStarted;

    // 自動歩行フラグ
    bool isAutoWalking = false;

    // ===== 基本的なヘルパー関数 =====
    void HandleDownwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager);
    bool IsOnStageGround(float playerX, float playerY, float width, float height, StageManager* stageManager);
    int GetCurrentSprite();
    void LoadCharacterSprites(int characterIndex);
    std::string GetCharacterColorName(int index);

    // スライディングシステム関数
    void StartSliding();
    void UpdateSliding();
    void EndSliding();
    float GetSlidingCollisionHeight() const;
    void DrawSlidingEffect(float cameraX);

    // ダメージシステム関数
    void UpdateDamageState();
    void UpdateInvulnerability();

    // ===== 詳細な衝突判定システム =====

    void HandleUpwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager);

    // メイン衝突判定関数
    bool CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager);
    void HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager);
    void HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager);

    void SetLandingState();

    // 詳細な衝突判定
    bool CheckPointCollision(float centerX, float centerY, float width, float height, StageManager* stageManager);
    bool IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager);

    // 位置調整機能
    float AdjustXPosition(float currentX, bool movingRight, float width, StageManager* stageManager);
    float FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager);
    float FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager);

    // ===== 上位互換性のための旧関数 =====
    bool CheckCollision(float newX, float newY, StageManager* stageManager);
    float GetGroundY(float checkX, StageManager* stageManager);
    bool CheckCollisionRect(float x, float y, float width, float height, StageManager* stageManager);
    int FindGroundTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);
    int FindCeilingTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);

    // 影描画用の地面検索関数
    float FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // 影描画の定数
    static constexpr float MAX_SHADOW_DISTANCE = 200.0f;
    static constexpr float BASE_SHADOW_SIZE_X = 40.0f;
    static constexpr float BASE_SHADOW_SIZE_Y = 16.0f;
    static constexpr int BASE_SHADOW_ALPHA = 150;

};