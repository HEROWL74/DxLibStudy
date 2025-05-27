#pragma once
#include "DxLib.h"

class CoinEffect {
public:
    CoinEffect(int startX, int startY, int targetX, int targetY);
    ~CoinEffect();

    void Update();
    void Draw() const;
    bool IsFinished() const { return m_timer >= m_duration; }

private:
    float m_x, m_y;
    float m_startX, m_startY;
    float m_targetX, m_targetY;
    int   m_handle;
    int   m_timer;
    int   m_duration;  // アニメ全体フレーム数
};
