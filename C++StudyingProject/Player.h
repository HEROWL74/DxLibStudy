#pragma once
#include <vector>
#include <cmath>
#include "DxLib.h"

class Player {
public:
    Player();
    ~Player();

    // ���t���[���X�V�E�`��
    void Update();
    void Draw();
    void Draw(int cameraX);

     void SetCameraOffset(int cameraX) { m_cameraOffsetX = cameraX; }

    // �����蔻��p�A�N�Z�T
    int   GetX() const;
    int   GetY() const;
    int   GetW() const;
    int   GetH() const;
    float GetVY() const;

    // HP�E�R�C���Ǘ�
    int   GetHealth()    const;
    int   GetMaxHealth() const;
    int   GetCoinCount() const;
    void  AddCoin(int amount);
    void  TakeDamage(int damage);

    // �m�b�N�o�b�N�E�ʒu�����p�Z�b�^�[
    void  SetX(float x);
    void  SetY(float y);
    void  SetVY(float vy);
    void  SetVX(float vx);
    void  SetOnGround(bool onGround);

    void SetClimbing(bool climbing) { m_climbing = climbing; }

private:
    enum class State { Idle, Walk, Jump, Duck, Hit, Climb };
    State m_state;

    // ���W�E���x
    float m_x, m_y;
    float m_vx, m_vy;

    int m_cameraOffsetX = 0;

    // �X�e�[�g�^�C�}�[
    int m_animFrame;
    int m_animTimer;
    int m_jumpTime;
    int m_hitTimer;

    // �t���O
    bool m_onGround;
    bool m_isJumping;
    bool m_facingLeft;

    // HP�E�R�C��
    int m_health;
    int m_maxHealth;
    int m_coins;

    //�[�[�͂����[�[//
    bool m_climbing;
    float m_climbSpeed;
    int   m_climbFrame, m_climbTimer;
    int   m_climbA, m_climbB;
    std::vector<int> m_climb;

    // �`��p�n���h��
    int m_idle;
    int m_walkA;
    int m_walkB;
    int m_jump;
    int m_duck;
    int m_hit;
    std::vector<int> m_walk;

    // �����蔻��T�C�Y
    int m_width;
    int m_height;

    // �萔
    static constexpr float gravity = 0.5f;
    static constexpr float accel = 0.5f;
    static constexpr float maxSpeed = 6.0f;
    static constexpr float friction = 0.85f;
    static constexpr float jumpPower = -10.0f;
    static constexpr int   maxJumpTime = 15;
    static constexpr int   hitDuration = 30;

    //���G����
    int m_invincibleTimer;
    static constexpr int invicibleDuration = 60;
    static constexpr int blinkInterval = 6;   // �_�ŊԊu�i�t���[�����j
    bool  m_visible;                    // �_�ŕ\���t���O

    //��_�����ʉ��n���h��
    int m_hurtSound;

    // �摜���[�h
    void LoadImages();
    //���ʉ����[�h
    void LoadSounds();
};