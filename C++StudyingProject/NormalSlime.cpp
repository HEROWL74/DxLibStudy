#include "NormalSlime.h"

NormalSlime::NormalSlime(int startX, int startY)
    : m_x(startX)
    , m_y(startY)
    , m_w(64)
    , m_h(64)
    , m_vx(2)
    , m_frame(0)
    , m_timer(0)
{
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_normal_walk_a.png"));
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_normal_walk_b.png"));
}

NormalSlime::~NormalSlime() {
    for (int h : m_graphs) {
        DeleteGraph(h);
    }
}

void NormalSlime::Update() {
    // 単純に左右往復
    m_x += m_vx;
    if (m_x < 0 || m_x > 1920 - m_w) {
        m_vx = -m_vx;
        m_x += m_vx;
    }

    // 歩行アニメ
    if (++m_timer > 16) {
        m_timer = 0;
        m_frame = (m_frame + 1) % (int)m_graphs.size();
    }
}

void NormalSlime::Draw() {
    DrawGraph(m_x, m_y, m_graphs[m_frame], TRUE);
}
