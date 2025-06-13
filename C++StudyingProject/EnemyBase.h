#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include <string>
#include <memory>

class EnemyBase {
public:
    // 敵の状態
    enum EnemyState {
        IDLE,           // 待機
        WALKING,        // 歩行
        ATTACKING,      // 攻撃
        DAMAGED,        // ダメージ
        DYING,          // 死亡アニメーション
        DEAD            // 死亡完了
    };

    // 敵のタイプ - ★修正: 新しい敵タイプを追加
    enum EnemyType {
        NORMAL_SLIME,
        SPIKE_SLIME,
        BEE,            // ★追加
        FLY,            // ★追加  
        LADYBUG,        // ★追加
        SAW,            // ★追加
        SLIME_BLOCK,    // ★追加
        // 将来的に他の敵も追加可能
        GOBLIN,
        SKELETON
    };

    // 敵のサイズ定数
    static const int ENEMY_WIDTH = 64;
    static const int ENEMY_HEIGHT = 64;

    EnemyBase(float startX, float startY, EnemyType type);
    virtual ~EnemyBase();

    // 純粋仮想関数（継承先で実装必須）
    virtual void Initialize() = 0;
    virtual void UpdateBehavior(Player* player, StageManager* stageManager) = 0;
    virtual void LoadSprites() = 0;

    // 共通処理
    virtual void Update(Player* player, StageManager* stageManager);
    virtual void Draw(float cameraX);
    virtual void DrawDebugInfo(float cameraX);

    // 当たり判定
    virtual bool CheckPlayerCollision(Player* player);
    virtual void OnPlayerCollision(Player* player);

    // ゲッター
    float GetX() const { return x; }
    float GetY() const { return y; }
    bool IsActive() const { return active; }
    bool IsDead() const { return currentState == DEAD; }
    EnemyState GetState() const { return currentState; }
    EnemyType GetType() const { return enemyType; }

    // セッター
    void SetPosition(float newX, float newY);
    void SetActive(bool isActive) { active = isActive; }
    void TakeDamage(int damage);

protected:
    // 基本パラメータ
    float x, y;                     // 位置
    float velocityX, velocityY;     // 速度
    bool facingRight;               // 向き
    EnemyState currentState;        // 現在の状態
    EnemyType enemyType;           // 敵のタイプ
    bool active;                   // アクティブフラグ
    bool onGround;                 // 地面接触フラグ

    // ステータス
    int health;                    // 体力
    int maxHealth;                 // 最大体力
    float moveSpeed;               // 移動速度
    float detectionRange;          // プレイヤー検出範囲
    float attackRange;             // 攻撃範囲
    int attackPower;               // 攻撃力

    // アニメーション
    float animationTimer;          // アニメーションタイマー
    bool animationFrame;           // アニメーションフレーム
    float stateTimer;              // 状態タイマー

    // 移動関連
    float patrolStartX;            // パトロール開始位置
    float patrolEndX;              // パトロール終了位置
    float patrolDistance;          // パトロール距離
    bool patrolDirection;          // パトロール方向（true=右、false=左）

    // スプライト - ★修正: jumpメンバーを追加
    struct EnemySprites {
        int idle;
        int walk_a;
        int walk_b;
        int attack;
        int damaged;
        int dead;
        int flat;      // 潰れた状態（スライム用）
        int jump;      // ★追加: ジャンプ/飛行状態用
    } sprites;

    // 物理定数
    static constexpr float GRAVITY = 0.8f;
    static constexpr float MAX_FALL_SPEED = 15.0f;
    static constexpr float COLLISION_WIDTH = 48.0f;
    static constexpr float COLLISION_HEIGHT = 56.0f;

    // 共通処理関数
    virtual void UpdatePhysics(StageManager* stageManager);
    virtual void UpdateAnimation();
    virtual void HandleCollisions(StageManager* stageManager);
    virtual void UpdatePatrol();
    virtual void HandlePlayerDetection(Player* player);

    // 当たり判定ヘルパー
    bool CheckGroundCollision(float checkX, float checkY, StageManager* stageManager);
    bool CheckWallCollision(float checkX, float checkY, StageManager* stageManager);
    float FindGroundY(float checkX, StageManager* stageManager);
    float GetDistanceToPlayer(Player* player);

    // スプライト関連
    virtual int GetCurrentSprite();
    void DrawEnemySprite(float cameraX, int spriteHandle);

    // ユーティリティ
    float Lerp(float a, float b, float t);
    bool IsPlayerInRange(Player* player, float range);
    void FacePlayer(Player* player);
};