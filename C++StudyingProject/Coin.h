#pragma once
#include "DxLib.h"
#include "Player.h"

class Coin {
public:
    Coin(int x, int y);
    ~Coin();

    // 毎フレーム更新。回収されたら isCollected() が true になる
    void Update(Player& player);
    void Draw() const;

    bool isCollected() const { return m_collected; }

private:
    int m_x, m_y;
    int m_w, m_h;
    int m_handle;

    bool m_collected;
};
