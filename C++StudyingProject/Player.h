#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include <string>

class Player {
public:
    // プレイヤー状態
    enum State {
        IDLE,
        WALKING,
        JUMPING,
        FALLING,
        DUCKING
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

private:
    // キャラクタースプライト構造体
    struct CharacterSprites {
        int front, idle, walk_a, walk_b, jump, duck, hit, climb_a, climb_b;
    };

    // 物理定数（ふわっとしたジャンプに調整）
    static constexpr float GRAVITY = 0.6f;             // 重力を弱く
    static constexpr float JUMP_POWER = -16.0f;        // ジャンプ力を少し弱く
    static constexpr float MOVE_SPEED = 8.0f;
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float MAX_FALL_SPEED = 12.0f;     // 最大落下速度を抑制

    // **追加：イージング関数の定数**
    static constexpr float FRICTION = 0.85f;           // 摩擦係数（横移動の減速）
    static constexpr float ACCELERATION = 1.2f;        // 加速度
    static constexpr float MAX_HORIZONTAL_SPEED = 8.0f; // 最大横移動速度

    // ジャンプの空気抵抗（ふわっと感を演出）
    static constexpr float AIR_RESISTANCE = 0.98f;     // 空中での速度減衰

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

    // ===== 基本的なヘルパー関数 =====
    void UpdatePhysics(StageManager* stageManager);
    void UpdateAnimation();
    void HandleCollisions(StageManager* stageManager);
    int GetCurrentSprite();
    void LoadCharacterSprites(int characterIndex);
    std::string GetCharacterColorName(int index);

    // ===== 詳細な衝突判定システム =====

    // メイン衝突判定関数
    bool CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager);
    void HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager);
    void HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager);

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
};