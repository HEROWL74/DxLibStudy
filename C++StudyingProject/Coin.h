#pragma once
#include "DxLib.h"

class Player;  // �O���錾

class Coin {
public:
    Coin(int x, int y);
    ~Coin();

    // �v���C���[���߂Â����� collected=true ��
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
