#include "Player.h"
#include "DxLib.h"

Player::Player() :
    m_x(100.0f),
    m_y(800.0f),  // ����Y�ʒu�i�X�e�[�W�̏�j
    m_vx(0.0f),
    m_vy(0.0f),
    m_onGround(false)
{
    m_handle = LoadGraph("Sprites/Characters/Default/character_green_front.png");  // �v���C���[�摜�p��
}

Player::~Player() {
    DeleteGraph(m_handle);
}

void Player::Update() {
    m_vx = 0.0f;

    // ���E�ړ�
    if (CheckHitKey(KEY_INPUT_LEFT)) {
        m_vx = -moveSpeed;
    }
    if (CheckHitKey(KEY_INPUT_RIGHT)) {
        m_vx = moveSpeed;
    }

    // �W�����v
    if (m_onGround && CheckHitKey(KEY_INPUT_Z)) {
        m_vy = jumpPower;
        m_onGround = false;
    }

    // �d��
    m_vy += gravity;

    // �ʒu�X�V
    m_x += m_vx;
    m_y += m_vy;

    // �n�ʂŒ�~
    const float groundY = 1080 - 4 * 64 - 64;  // �X�e�[�W��iY - �v���C���[����
    if (m_y >= groundY) {
        m_y = groundY;
        m_vy = 0;
        m_onGround = true;
    }
}

void Player::Draw() {
    DrawGraph(static_cast<int>(m_x), static_cast<int>(m_y), m_handle, TRUE);
}
