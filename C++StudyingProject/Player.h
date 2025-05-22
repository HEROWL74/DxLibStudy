#pragma once
#include <vector>
#include <string>

class Player {
public:
    Player();
    ~Player();

    void Update();
    void Draw();

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

    const float gravity = 0.5f;
    const float moveSpeed = 4.0f;
    const float jumpPower = -10.0f;
    const int maxJumpTime = 15;

    // ‰æ‘œƒnƒ“ƒhƒ‹
    int m_idle;
    int m_jump;
    int m_duck;
    std::vector<int> m_walk;
    bool m_facingLeft;

    void LoadImages();
};
