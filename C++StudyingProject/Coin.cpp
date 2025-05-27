#include "Coin.h"

Coin::Coin(int x, int y, int tx, int ty)
    : m_x(x), m_y(y), m_collected(false)
    , m_startX((float)x)
    , m_startY((float)y)
    , m_targetX(tx)
    , m_targetY(ty)
    , m_timer(0)
    , m_duration(30)     // ��30�t���[���Ŕ��
    , m_state(State::Idle)
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

    if (m_state == State::Idle) {
        // AABB �Փ˔���
        if (!(px + pw  < m_x ||
            px      > m_x + m_w ||
            py + ph  < m_y ||
            py      > m_y + m_h))
        {
            m_collected = true;
            player.AddCoin(1);  // �v���C���[�̃R�C������ +1
            // ����X�^�[�g
            m_state = State::MovingToHUD;
            m_startX = m_x;
            m_startY = m_y;
            m_timer = 0;
        }
    }
    else if (m_state == State::MovingToHUD) {
        // t �� [0,1] �Ői�s
        float t = (float)++m_timer / m_duration;
        if (t >= 1.0f) {
            t = 1.0f;
            m_state = State::Collected;
            player.AddCoin(1);  // �������ɃR�C������+1
        }
        // �C�[�W���O�Ȃ��̐��`���
        m_x = m_startX + (m_targetX - m_startX) * t;
        m_y = m_startY + (m_targetY - m_startY) * t;
    }

}

void Coin::Draw() const {
    if (m_state == State::Collected) return;
    DrawGraph((int)m_x, (int)m_y, m_handle, TRUE);
}
