#pragma once
#include "DxLib.h"
#include "Player.h"

class Coin {
public:
    Coin(int x, int y, int tx, int ty);
    ~Coin();

    // 毎フレーム更新。回収されたら isCollected() が true になる
    void Update(Player& player);
    void Draw() const;

    bool IsDone() const { return m_state == State::Collected; }

    bool isCollected() const { return m_collected; }

private:

    enum class State { Idle, MovingToHUD, Collected };
    float m_x, m_y;
    int m_w, m_h;
    int m_handle;

    // アニメーション開始座標
    float m_startX, m_startY;
    // HUDアイコン座標
    int   m_targetX, m_targetY;
    // アニメーション進行
    int   m_timer, m_duration;

    State m_state;

    bool m_collected;

    static constexpr int COLLIDE_MARGIN = 0;
};
