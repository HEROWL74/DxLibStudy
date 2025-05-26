#pragma once
#include <vector>
#include <string>

class Player {
public:
    Player();
    ~Player();

    void Update();
    void Draw();
    //座標&大きさを取得する変数
    int  GetX() const;
    int  GetY() const;
    int  GetW() const;
    int  GetH() const;

    //垂直の速度取得と位置、速度、着地フラグ設定
    float GetVY() const;
    void SetY(float y);
    void SetVY(float vy);
    void SetOnGround(bool onGround);

private:
    enum class State {
        Idle,
        Walk,
        Jump,
        Duck
    };

    State m_state;
    int m_x, m_y;
    float m_vx, m_vy;
    bool m_onGround;
    bool m_isJumping;
    int m_jumpTime;
    int m_animFrame;
    int m_animTimer;

    // ── 追加 ──
       // スプライトの実際の幅・高さ
    int m_width;
    int m_height;



    const float gravity = 0.5f;
    const float moveSpeed = 4.0f;
    const float jumpPower = -10.0f;
    const int maxJumpTime = 15;

    const float maxSpeed = 6.0f;
    const float accel = 0.5f;
    const float friction = 0.85f;


    // 画像ハンドル
    int m_idle;
    int m_jump;
    int m_duck;
    std::vector<int> m_walk;
    bool m_facingLeft;



    void LoadImages();
};
