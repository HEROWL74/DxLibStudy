#include "CoinEffect.h"

using namespace std;

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
    // 弧を描くような移動（中間点を高めにする）
    float t = static_cast<float>(m_timer) / m_duration;
    if (t > 1.0f) t = 1.0f;

    // 中間制御点を計算（HUDよりちょっと上）
    float ctrlX = (m_startX + m_targetX) / 2;
    float ctrlY = min(m_startY, m_targetY) - 100;

    float x1 = (1 - t) * m_startX + t * ctrlX;
    float y1 = (1 - t) * m_startY + t * ctrlY;
    float x2 = (1 - t) * ctrlX + t * m_targetX;
    float y2 = (1 - t) * ctrlY + t * m_targetY;

    m_x = (1 - t) * x1 + t * x2;
    m_y = (1 - t) * y1 + t * y2;

}

void CoinEffect::Draw() const {
    if (m_timer < m_duration) {
        DrawGraph(static_cast<int>(m_x), static_cast<int>(m_y), m_handle, TRUE);
    }
}
