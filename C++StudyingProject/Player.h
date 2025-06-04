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

    // プレイヤーサイズ定数（128x128に対応）
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

private:
    // キャラクタースプライト構造体
    struct CharacterSprites {
        int front, idle, walk_a, walk_b, jump, duck, hit, climb_a, climb_b;
    };

    // 物理定数（128x128プレイヤー用に最適化）
    static constexpr float GRAVITY = 0.8f;             // 重力を少し強めに調整
    static constexpr float JUMP_POWER = -20.0f;        // ジャンプ力を128x128プレイヤーに合わせて調整
    static constexpr float MOVE_SPEED = 8.0f;
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float MAX_FALL_SPEED = 16.0f;     // 最大落下速度

    // 改良：イージング関数の定数
    static constexpr float FRICTION = 0.85f;           // 摩擦係数（横移動の減速）
    static constexpr float ACCELERATION = 1.2f;        // 加速度
    static constexpr float MAX_HORIZONTAL_SPEED = 8.0f; // 最大横移動速度

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


   // 基本的な地面検出（高速）
    float FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // 高精度な地面検出（詳細）
    float FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // 適応的地面検出（複雑な地形対応）
    float FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // 最適な地面検出（状況に応じて最適な方法を選択）
    float FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // 影描画の定数
    static constexpr float MAX_SHADOW_DISTANCE = 200.0f; // 影が表示される最大距離
    static constexpr float BASE_SHADOW_SIZE_X = 40.0f;   // 基本影サイズ（幅）
    static constexpr float BASE_SHADOW_SIZE_Y = 16.0f;   // 基本影サイズ（高さ）
    static constexpr int BASE_SHADOW_ALPHA = 150;        // 基本影の透明度
};