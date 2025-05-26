#pragma once
#include "DxLib.h"
#include "Player.h"

class Coin {
public:
    Coin(int x, int y);
    ~Coin();

    // ���t���[���X�V�B������ꂽ�� isCollected() �� true �ɂȂ�
    void Update(Player& player);
    void Draw() const;

    bool isCollected() const { return m_collected; }

private:
    int m_x, m_y;
    int m_w, m_h;
    int m_handle;

    bool m_collected;
};
