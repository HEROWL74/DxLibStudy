#include "Player.h"
#include "DxLib.h"

Player::Player() :
    m_x(100.0f),
    m_y(800.0f),  // 初期Y位置（ステージの上）
    m_vx(0.0f),
    m_vy(0.0f),
    m_onGround(false)
{
    m_handle = LoadGraph("Sprites/Characters/Default/character_green_front.png");  // プレイヤー画像用意
}

Player::~Player() {
    DeleteGraph(m_handle);
}

void Player::Update() {
    m_vx = 0.0f;

    // 左右移動
    if (CheckHitKey(KEY_INPUT_LEFT)) {
        m_vx = -moveSpeed;
    }
    if (CheckHitKey(KEY_INPUT_RIGHT)) {
        m_vx = moveSpeed;
    }

    // ジャンプ
    if (m_onGround && CheckHitKey(KEY_INPUT_Z)) {
        m_vy = jumpPower;
        m_onGround = false;
    }

    // 重力
    m_vy += gravity;

    // 位置更新
    m_x += m_vx;
    m_y += m_vy;

    // 地面で停止
    const float groundY = 1080 - 4 * 64 - 64;  // ステージ上段Y - プレイヤー高さ
    if (m_y >= groundY) {
        m_y = groundY;
        m_vy = 0;
        m_onGround = true;
    }
}

void Player::Draw() {
    DrawGraph(static_cast<int>(m_x), static_cast<int>(m_y), m_handle, TRUE);
}
