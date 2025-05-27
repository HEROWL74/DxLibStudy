#include "Coin.h"
#include "Player.h"

Coin::Coin(int x, int y)
    : m_x(x), m_y(y), m_collected(false)
{
    m_handle = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    GetGraphSize(m_handle, &m_w, &m_h);
}

Coin::~Coin() {
    DeleteGraph(m_handle);
}

void Coin::Update(const Player& player) {
    if (m_collected) return;
    // AABB è’ìÀ
    int px = player.GetX(), py = player.GetY();
    int pw = player.GetW(), ph = player.GetH();
    if (!(px + pw  < m_x ||
        px      > m_x + m_w ||
        py + ph  < m_y ||
        py      > m_y + m_h)) {
        m_collected = true;
    }
}

void Coin::Draw() const {
    if (!m_collected) {
        DrawGraph(m_x, m_y, m_handle, TRUE);
    }
}
