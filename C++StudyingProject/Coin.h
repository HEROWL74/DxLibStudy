#pragma once
#include "DxLib.h"
#include "Player.h"

class Coin {
public:
    Coin(int x, int y, int tx, int ty);
    ~Coin();

    // ���t���[���X�V�B������ꂽ�� isCollected() �� true �ɂȂ�
    void Update(Player& player);
    void Draw() const;

    bool IsDone() const { return m_state == State::Collected; }

    bool isCollected() const { return m_collected; }

private:

    enum class State { Idle, MovingToHUD, Collected };
    float m_x, m_y;
    int m_w, m_h;
    int m_handle;

    // �A�j���[�V�����J�n���W
    float m_startX, m_startY;
    // HUD�A�C�R�����W
    int   m_targetX, m_targetY;
    // �A�j���[�V�����i�s
    int   m_timer, m_duration;

    State m_state;

    bool m_collected;

    static constexpr int COLLIDE_MARGIN = 0;
};
