#include "CoinEffect.h"

CoinEffect::CoinEffect(int startX, int startY, int targetX, int targetY)
    : m_startX(startX), m_startY(startY),
    m_targetX(targetX), m_targetY(targetY),
    m_x(startX), m_y(startY),
    m_timer(0), m_duration(30)
{
    m_handle = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
}

CoinEffect::~CoinEffect() {
    DeleteGraph(m_handle);
}

void CoinEffect::Update() {
    m_timer++;
    float t = m_timer / static_cast<float>(m_duration);
    if (t > 1.0f) t = 1.0f;
    // イージング（シンプルな二次曲線）
    float et = t * t;
    m_x = m_startX + (m_targetX - m_startX) * et;
    m_y = m_startY + (m_targetY - m_startY) * et;
}

void CoinEffect::Draw() const {
    if (m_timer < m_duration) {
        DrawGraph(static_cast<int>(m_x), static_cast<int>(m_y), m_handle, TRUE);
    }
}
