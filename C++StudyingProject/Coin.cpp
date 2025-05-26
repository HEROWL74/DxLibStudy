#include "Coin.h"

Coin::Coin(int x, int y)
    : m_x(x), m_y(y), m_collected(false)
{
    m_handle = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    GetGraphSize(m_handle, &m_w, &m_h);
}

Coin::~Coin() {
    DeleteGraph(m_handle);
}

void Coin::Update(Player& player) {
    if (m_collected) return;

    int px = player.GetX();
    int py = player.GetY();
    int pw = player.GetW();
    int ph = player.GetH();

    // AABB 衝突判定
    if (!(px + pw  < m_x ||
        px      > m_x + m_w ||
        py + ph  < m_y ||
        py      > m_y + m_h))
    {
        m_collected = true;
        player.AddCoin(1);  // プレイヤーのコイン数を +1
    }
}

void Coin::Draw() const {
    if (m_collected) return;
    DrawGraph(m_x, m_y, m_handle, TRUE);
}
