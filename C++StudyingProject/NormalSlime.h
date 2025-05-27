#pragma once
#include "EnemyBase.h"
#include <vector>

class NormalSlime : public EnemyBase {
public:
    // startX/Y: 初期座標, floorY: 床のY座標
    NormalSlime(int startX, int startY, float floorY);
    ~NormalSlime() override;

    void Update() override;
    void Draw() override;

    int GetX()    const override { return static_cast<int>(m_x); }
    int GetY()    const override { return static_cast<int>(m_y); }
    int GetW()    const override { return m_w; }
    int GetH()    const override { return m_h; }

private:
    float m_x, m_y;    // 座標
    float m_vx, m_vy;  // 速度 (水平・垂直)
    float m_floorY;    // 床のY座標
    int   m_w, m_h;    // 幅・高さ

    int m_frame;       // アニメーションフレーム
    int m_timer;       // フレームカウンタ
    std::vector<int> m_graphs;

    static constexpr float gravity = 0.5f;  // 重力加速度
};