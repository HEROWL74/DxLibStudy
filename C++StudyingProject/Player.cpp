#include "Player.h"
#include <algorithm>
#include "Stage1Generate.h"
using namespace std;

Player::Player()
    : m_x(100.0f)
    , m_y(400.0f)
    , m_vx(0.0f)
    , m_vy(0.0f)
    , m_state(State::Idle)
    , m_animFrame(0)
    , m_animTimer(0)
    , m_jumpTime(0)
    , m_hitTimer(0)
    , m_onGround(false)
    , m_isJumping(false)
    , m_facingLeft(false)
    , m_maxHealth(6)
    , m_health(m_maxHealth)
    , m_coins(0)
    , m_climbing(false),
    m_climbSpeed(2.0f), m_climbFrame(0), m_climbTimer(0)
{
    LoadImages();
    GetGraphSize(m_idle, &m_width, &m_height);
}

Player::~Player() {
    DeleteGraph(m_idle);
    DeleteGraph(m_walkA);
    DeleteGraph(m_walkB);
    DeleteGraph(m_jump);
    DeleteGraph(m_duck);
    DeleteGraph(m_hit);
}

void Player::LoadImages() {
    m_idle = LoadGraph("Sprites/Characters/Default/character_green_idle.png");
    m_walkA = LoadGraph("Sprites/Characters/Default/character_green_walk_a.png");
    m_walkB = LoadGraph("Sprites/Characters/Default/character_green_walk_b.png");
    m_jump = LoadGraph("Sprites/Characters/Default/character_green_jump.png");
    m_duck = LoadGraph("Sprites/Characters/Default/character_green_duck.png");
    m_hit = LoadGraph("Sprites/Characters/Default/character_green_hit.png");
    m_walk = { m_walkA, m_walkB };
    m_climbA = LoadGraph("Sprites/Characters/Default/character_green_climb_a.png");
    m_climbB = LoadGraph("Sprites/Characters/Default/character_green_climb_b.png");
    m_climb = { m_climbA, m_climbB };
}

void Player::Update() {

    // ヒット演出タイマー
    if (m_hitTimer > 0) --m_hitTimer;

    // 向きは常に更新
    if (CheckHitKey(KEY_INPUT_LEFT))  m_facingLeft = true;
    if (CheckHitKey(KEY_INPUT_RIGHT)) m_facingLeft = false;

    // 水平移動
    if (m_hitTimer == 0) {
        if (CheckHitKey(KEY_INPUT_LEFT)) {
            m_vx = max(m_vx - accel, -maxSpeed);
        }
        else if (CheckHitKey(KEY_INPUT_RIGHT)) {
            m_vx = min(m_vx + accel, maxSpeed);
        }
       
        else {
            m_vx *= friction;
            if (std::abs(m_vx) < 0.1f) m_vx = 0.0f;
        }
    }
    else {
        m_vx *= friction;
    }

    if (m_climbing) {
        // 上下キーで登降
        if (CheckHitKey(KEY_INPUT_UP)) {
            m_y -= m_climbSpeed;
        }
        else if (CheckHitKey(KEY_INPUT_DOWN)) {
            m_y += m_climbSpeed;
        }
        // アニメ更新
        if (++m_climbTimer > 12) {
            m_climbTimer = 0;
            m_climbFrame = (m_climbFrame + 1) % 2;
        }
        m_state = State::Climb;
        return;  // そのほかの処理はスキップ
    }

    // ジャンプ開始
    if (m_hitTimer == 0 && m_onGround && CheckHitKey(KEY_INPUT_SPACE)) {
        m_isJumping = true;
        m_jumpTime = 0;
        m_vy = jumpPower;
        m_onGround = false;
    }
    // ジャンプホールド
    if (m_isJumping) {
        ++m_jumpTime;
        if (CheckHitKey(KEY_INPUT_SPACE) && m_jumpTime < maxJumpTime) {
            m_vy += jumpPower / float(maxJumpTime);
        }
        else {
            m_isJumping = false;
        }
        if (m_jumpTime >= maxJumpTime) m_isJumping = false;
    }

    // 重力
    m_vy += gravity;

    // 位置更新
    m_x += m_vx;
    m_y += m_vy;

    // ステート決定
    if (m_hitTimer > 0)  m_state = State::Hit;
    else if (!m_onGround)  m_state = State::Jump;
    else if (m_vx != 0.0f)  m_state = State::Walk;
    else if (CheckHitKey(KEY_INPUT_DOWN))
        m_state = State::Duck;
    else   m_state = State::Idle;
    
    

    // 歩行アニメ
    if (m_state == State::Walk) {
        ++m_animTimer;
        if (m_animTimer > 8) {
            m_animTimer = 0;
            m_animFrame = (m_animFrame + 1) % static_cast<int>(m_walk.size());
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
    case State::Idle: handle = m_idle;               break;
    case State::Walk: handle = m_walk[m_animFrame];  break;
    case State::Jump: handle = m_jump;               break;
    case State::Duck: handle = m_duck;               break;
    case State::Hit:  handle = m_hit;                break;
    case State::Climb: handle = m_climb[m_climbFrame]; break;
    }
    if (m_state == State::Climb) {
        // はしごは反転不要なので通常描画
        DrawGraph((int)m_x, (int)m_y, handle, TRUE);
    }
    else {
        // 既存の左右反転分岐
        if (m_facingLeft)
            DrawTurnGraph((int)m_x, (int)m_y, handle, TRUE);
        else
            DrawGraph((int)m_x, (int)m_y, handle, TRUE);
    }
}

// アクセサ
int   Player::GetX()         const { return static_cast<int>(m_x); }
int   Player::GetY()         const { return static_cast<int>(m_y); }
int   Player::GetW()         const { return m_width; }
int   Player::GetH()         const { return m_height; }
float Player::GetVY()        const { return m_vy; }
int   Player::GetHealth()    const { return m_health; }
int   Player::GetMaxHealth() const { return m_maxHealth; }
int   Player::GetCoinCount() const { return m_coins; }

// セッター
void Player::SetX(float x) { m_x = x; }
void Player::SetY(float y) { m_y = y; }
void Player::SetVY(float vy) { m_vy = vy; }
void Player::SetVX(float vx) { m_vx = vx; }
void Player::SetOnGround(bool onGround) { m_onGround = onGround; }

// アイテム・ダメージ
void Player::AddCoin(int amount) { m_coins += amount; }

void Player::TakeDamage(int amount) {
    if (m_hitTimer == 0) {
        m_health = max(0, m_health - amount);
        m_hitTimer = hitDuration;
    }
}