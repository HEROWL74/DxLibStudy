#pragma once
#include "DxLib.h"

class Player;  // 前方宣言

class Coin {
public:
    Coin(int x, int y);
    ~Coin();

    // プレイヤーが近づいたら collected=true に
    void     Update(const Player& player);
    void     Draw() const;
    bool     isCollected() const { return m_collected; }
    int      GetX() const { return m_x; }
    int      GetY() const { return m_y; }

private:
    int  m_x, m_y;
    int  m_w, m_h;
    int  m_handle;
    bool m_collected;
};
