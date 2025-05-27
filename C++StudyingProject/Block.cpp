#include "Block.h"
#include <cmath>

Block::Block(int x, int y)
    : m_x(x)
    , m_y(y)
    , m_state(State::Rest)
{
    // �摜�ǂݍ���
    m_handleRest = LoadGraph("Sprites/Enemies/Default/block_rest.png");
    m_handleIdle = LoadGraph("Sprites/Enemies/Default/block_idle.png");
    m_handleFall = LoadGraph("Sprites/Enemies/Default/block_fall.png");

    // �傫�����擾
    GetGraphSize(m_handleRest, &m_width, &m_height);
}

Block::~Block() {
    DeleteGraph(m_handleRest);
    DeleteGraph(m_handleIdle);
    DeleteGraph(m_handleFall);
}

void Block::Update(const Player& player) {
    // �v���C���[���擾
    int px = player.GetX();
    int py = player.GetY();
    int pw = player.GetW();
    int ph = player.GetH();

    // �ڐG����F�҂�����d�Ȃ����� Fall ��\��
    bool touching = !(px + pw  < m_x ||
        px      > m_x + m_width ||
        py + ph  < m_y ||
        py      > m_y + m_height);
    if (touching) {
        m_state = State::Fall;
        return;
    }

    // �Q�D�ڋߔ���F���S�ԋ����� Idle �\��
    float cx = px + pw * 0.5f;
    float cy = py + ph * 0.5f;
    float bx = m_x + m_width * 0.5f;
    float by = m_y + m_height * 0.5f;
    float dist = std::sqrt((cx - bx) * (cx - bx) + (cy - by) * (cy - by));

    m_state = (dist < idleDist) ? State::Idle : State::Rest;
}

void Block::Draw() const {
    int handle = m_handleRest;
    switch (m_state) {
    case State::Rest: handle = m_handleRest; break;
    case State::Idle: handle = m_handleIdle; break;
    case State::Fall: handle = m_handleFall; break;
    }
    DrawGraph(m_x, m_y, handle, TRUE);
}
