#include "SpikeSlime.h"

SpikeSlime::SpikeSlime(int startX, int startY)
    : m_x(startX)
    , m_y(startY)
    , m_w(64)
    , m_h(64)
    , m_vx(1)
    , m_frame(0)
    , m_timer(0)
{
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_spike_rest.png"));    // ‘Ò‹@
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_spike_walk_a.png"));
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_spike_walk_b.png"));
}

SpikeSlime::~SpikeSlime() {
    for (int h : m_graphs) {
        DeleteGraph(h);
    }
}

void SpikeSlime::Update() {
    // ‹x~ƒtƒŒ[ƒ€‚ğ‹²‚İ‚Â‚ÂˆÚ“®
    if (++m_timer < 60) {
        // rest
        m_frame = 0;
    }
    else {
        // walk
        if (m_timer > 80) m_timer = 0;
        m_frame = 1 + ((m_timer / 10) % 2);
        m_x += m_vx;
        if (m_x < 0 || m_x > 1920 - m_w) {
            m_vx = -m_vx;
            m_x += m_vx;
        }
    }
}

void SpikeSlime::Draw() {
    DrawGraph(m_x, m_y, m_graphs[m_frame], TRUE);
}
