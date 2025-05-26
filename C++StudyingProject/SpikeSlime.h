#pragma once
#include "EnemyBase.h"
#include <vector>

class SpikeSlime : public EnemyBase {
public:
    SpikeSlime(int startX, int startY);
    ~SpikeSlime() override;

    void Update() override;
    void Draw()   override;

    int GetX() const override { return m_x; }
    int GetY() const override { return m_y; }
    int GetW() const override { return m_w; }
    int GetH() const override { return m_h; }

private:
    int m_x, m_y;
    int m_w, m_h;
    int m_vx;

    int m_frame;
    int m_timer;
    std::vector<int> m_graphs;
};
