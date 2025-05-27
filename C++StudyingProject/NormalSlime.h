#pragma once
#include "EnemyBase.h"
#include <vector>

class NormalSlime : public EnemyBase {
public:
    // startX/Y: �������W, floorY: ����Y���W
    NormalSlime(int startX, int startY, float floorY);
    ~NormalSlime() override;

    void Update() override;
    void Draw() override;

    int GetX()    const override { return static_cast<int>(m_x); }
    int GetY()    const override { return static_cast<int>(m_y); }
    int GetW()    const override { return m_w; }
    int GetH()    const override { return m_h; }

private:
    float m_x, m_y;    // ���W
    float m_vx, m_vy;  // ���x (�����E����)
    float m_floorY;    // ����Y���W
    int   m_w, m_h;    // ���E����

    int m_frame;       // �A�j���[�V�����t���[��
    int m_timer;       // �t���[���J�E���^
    std::vector<int> m_graphs;

    static constexpr float gravity = 0.5f;  // �d�͉����x
};