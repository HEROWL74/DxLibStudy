#include "Player.h"
#include "DxLib.h"
#include <cmath>
Player::Player() :
    m_x(100), m_y(800), m_vx(0), m_vy(0),
    m_onGround(false), m_isJumping(false), m_jumpTime(0),
    m_animFrame(0), m_animTimer(0), m_state(State::Idle),
    m_maxHealth(6),m_health(m_maxHealth),m_coins(0)
{
    LoadImages();
    GetGraphSize(m_idle, &m_width, &m_height);
}

Player::~Player() {
    DeleteGraph(m_idle);
    DeleteGraph(m_jump);
    DeleteGraph(m_duck);
    for (int handle : m_walk) {
        DeleteGraph(handle);
    }
}

int Player::GetX() const { return static_cast<int>(m_x); }
int Player::GetY() const { return static_cast<int>(m_y); }
int Player::GetW() const { return m_width; }
int Player::GetH() const { return m_height; }

int Player::GetHealth()    const { return m_health; }
int Player::GetMaxHealth() const { return m_maxHealth; }
int Player::GetCoinCount() const { return m_coins; }

float Player::GetVY() const { return m_vy; }
void  Player::SetY(float y) { m_y = y; }
void  Player::SetVY(float vy) { m_vy = vy; }
void  Player::SetOnGround(bool g) { m_onGround = g; }

void Player::LoadImages() {
    m_idle = LoadGraph("Sprites/Characters/Default/character_green_idle.png");
    m_jump = LoadGraph("Sprites/Characters/Default/character_green_jump.png");
    m_duck = LoadGraph("Sprites/Characters/Default/character_green_duck.png");
    m_walk.push_back(LoadGraph("Sprites/Characters/Default/character_green_walk_a.png"));
    m_walk.push_back(LoadGraph("Sprites/Characters/Default/character_green_walk_b.png"));
}

void Player::Update() {
    m_vx = 0;

    // 入力
    if (CheckHitKey(KEY_INPUT_LEFT)) {
        m_vx = -moveSpeed;
        m_facingLeft = true;
    }
    else if (CheckHitKey(KEY_INPUT_RIGHT)) {
        m_vx = moveSpeed;
        m_facingLeft = false;
    }
    else {
        //キーを離していたら摩擦で減速
        m_vx *= friction;

        //  微小速度を0にする
        if (std::abs(m_vx) < 0.1f) {
            m_vx = 0.0f;
        }
    }
    //しゃがみと衝突判定
    if (m_onGround && CheckHitKey(KEY_INPUT_DOWN)) {
        m_state = State::Duck;
        m_vx = 0;
        return;
    }

    // ジャンプ開始
    if (m_onGround && CheckHitKey(KEY_INPUT_SPACE)) {
        m_isJumping = true;
        m_jumpTime = 0;
        m_vy = jumpPower;
        m_onGround = false;
    }

    // ジャンプ中制御
    if (m_isJumping) {
        m_jumpTime++;
        if (CheckHitKey(KEY_INPUT_SPACE)) {
            if (m_jumpTime < maxJumpTime) {
                m_vy += jumpPower / maxJumpTime;
            }
        }
        else {
            m_isJumping = false;
        }

        if (m_jumpTime >= maxJumpTime) {
            m_isJumping = false;
        }
    }

    // 重力
    m_vy += gravity;

    // 位置更新
    m_x += static_cast<int>(m_vx);
    m_y += static_cast<int>(m_vy);

    // 地面判定
    const int groundY = 1080 - 4 * 64 - 64;
    if (m_y >= groundY) {
        m_y = groundY;
        m_vy = 0;
        m_onGround = true;
        m_isJumping = false;
    }

    // 状態更新
    if (!m_onGround) {
        m_state = State::Jump;
    }
    else if (m_vx != 0) {
        m_state = State::Walk;
    }
    else {
        m_state = State::Idle;
    }

    // 歩行アニメーション進行
    if (m_state == State::Walk) {
        m_animTimer++;
        if (m_animTimer > 8) {
            m_animTimer = 0;
            m_animFrame = (m_animFrame + 1) % m_walk.size();
        }
    }
    else {
        m_animFrame = 0;
        m_animTimer = 0;
    }
}

void Player::Draw() {
    int handle = m_idle;

    switch (m_state) {
    case State::Idle:
        handle = m_idle;
        break;
    case State::Walk:
        handle = m_walk[m_animFrame];
        break;
    case State::Jump:
        handle = m_jump;
        break;
    case State::Duck:
        handle = m_duck;
        break;

    }

    if (m_facingLeft) {
        DrawTurnGraph(m_x, m_y, handle, TRUE);  // 左向き
    }
    else {
        DrawGraph(m_x, m_y, handle, TRUE);      // 右向き
    }

}

void Player::AddCoin(int amount) {
    m_coins += amount;
}

void Player::TakeDamage(int amount) {
    m_health -= amount;
    if (m_health < 0) m_health = 0;
}

void Player::Heal(int amount) {
    m_health += amount;
    if (m_health > m_maxHealth) m_health = m_maxHealth;
}
