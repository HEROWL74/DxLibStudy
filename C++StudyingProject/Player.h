#pragma once
#include <vector>
#include <cmath>
#include "DxLib.h"

class Player {
public:
    Player();
    ~Player();

    // 毎フレーム更新・描画
    void Update();
    void Draw();
    void Draw(int cameraX);

     void SetCameraOffset(int cameraX) { m_cameraOffsetX = cameraX; }

    // 当たり判定用アクセサ
    int   GetX() const;
    int   GetY() const;
    int   GetW() const;
    int   GetH() const;
    float GetVY() const;

    // HP・コイン管理
    int   GetHealth()    const;
    int   GetMaxHealth() const;
    int   GetCoinCount() const;
    void  AddCoin(int amount);
    void  TakeDamage(int damage);

    // ノックバック・位置調整用セッター
    void  SetX(float x);
    void  SetY(float y);
    void  SetVY(float vy);
    void  SetVX(float vx);
    void  SetOnGround(bool onGround);

    void SetClimbing(bool climbing) { m_climbing = climbing; }

private:
    enum class State { Idle, Walk, Jump, Duck, Hit, Climb };
    State m_state;

    // 座標・速度
    float m_x, m_y;
    float m_vx, m_vy;

    int m_cameraOffsetX = 0;

    // ステートタイマー
    int m_animFrame;
    int m_animTimer;
    int m_jumpTime;
    int m_hitTimer;

    // フラグ
    bool m_onGround;
    bool m_isJumping;
    bool m_facingLeft;

    // HP・コイン
    int m_health;
    int m_maxHealth;
    int m_coins;

    //ーーはしごーー//
    bool m_climbing;
    float m_climbSpeed;
    int   m_climbFrame, m_climbTimer;
    int   m_climbA, m_climbB;
    std::vector<int> m_climb;

    // 描画用ハンドル
    int m_idle;
    int m_walkA;
    int m_walkB;
    int m_jump;
    int m_duck;
    int m_hit;
    std::vector<int> m_walk;

    // 当たり判定サイズ
    int m_width;
    int m_height;

    // 定数
    static constexpr float gravity = 0.5f;
    static constexpr float accel = 0.5f;
    static constexpr float maxSpeed = 6.0f;
    static constexpr float friction = 0.85f;
    static constexpr float jumpPower = -10.0f;
    static constexpr int   maxJumpTime = 15;
    static constexpr int   hitDuration = 30;

    //無敵時間
    int m_invincibleTimer;
    static constexpr int invicibleDuration = 60;
    static constexpr int blinkInterval = 6;   // 点滅間隔（フレーム数）
    bool  m_visible;                    // 点滅表示フラグ

    //被ダメ効果音ハンドル
    int m_hurtSound;

    // 画像ロード
    void LoadImages();
    //効果音ロード
    void LoadSounds();
};