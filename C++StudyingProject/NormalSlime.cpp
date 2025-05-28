// NormalSlime.cpp
#include "NormalSlime.h"
#include "DxLib.h"

NormalSlime::NormalSlime(int startX, int startY, float floorY)
    : m_x(static_cast<float>(startX))
    , m_y(static_cast<float>(startY))
    , m_vx(3.0f)
    , m_vy(0.0f)
    , m_floorY(floorY)
    , m_w(64)
    , m_h(64)
    , m_frame(0)
    , m_timer(0)
{
    // 歩行アニメーション用画像を読み込む
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_normal_walk_a.png"));
    m_graphs.push_back(LoadGraph("Sprites/Enemies/Default/slime_normal_walk_b.png"));
}

NormalSlime::~NormalSlime() {
    for (int h : m_graphs) {
        DeleteGraph(h);
    }
}

void NormalSlime::Update() {
    // ── 垂直方向：重力適用・床との当たり判定 ──
    m_vy += gravity;
    m_y += m_vy;
    if (m_y > m_floorY - m_h) {
        m_y = m_floorY - m_h;
        m_vy = 0.0f;
    }

    // ── 水平方向：左右移動して端で折り返し ──
    m_x += m_vx;
    if (m_x < 0 || m_x > 1920 - m_w) {
        m_vx = -m_vx;
        m_x += m_vx;
    }

    // ── アニメーション更新 ──
    if (++m_timer > 20) {
        m_timer = 0;
        m_frame = (m_frame + 1) % static_cast<int>(m_graphs.size());
    }
}

void NormalSlime::Draw() {
    DrawGraph(static_cast<int>(m_x), static_cast<int>(m_y), m_graphs[m_frame], TRUE);
}
