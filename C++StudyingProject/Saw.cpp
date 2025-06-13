#include "Saw.h"
#include <math.h>

Saw::Saw(float startX, float startY)
    : EnemyBase(startX, startY, SAW)
    , sawState(SAW_RESTING)
    , restTimer(0.0f)
    , spinTimer(0.0f)
    , moveTimer(0.0f)
    , attackTimer(0.0f)
    , rotationAngle(0.0f)
    , attackDirection(1.0f)
{
    health = 60;
    maxHealth = 60;
    moveSpeed = 2.0f;
    detectionRange = 120.0f;
    attackRange = 100.0f;
    attackPower = 12;

    // �p�g���[���͈͐ݒ�
    patrolDistance = 150.0f;
    patrolStartX = startX - 75.0f;
    patrolEndX = startX + 75.0f;
}

void Saw::Initialize()
{
    LoadSprites();
    sawState = SAW_RESTING;
    currentState = IDLE;
}

void Saw::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/saw_";
    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "b.png").c_str());
    sprites.jump = sprites.walk_a;
    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.idle;
    sprites.flat = sprites.idle;
}

void Saw::UpdateBehavior(Player* player, StageManager* stageManager)
{
    if (currentState == DEAD || !active) return;

    velocityY = 0.0f; // �󒆓G
    UpdateSawMovement();
    HandlePlayerDetectionSaw(player);
    UpdateSawAnimation();
}

void Saw::UpdateSawMovement()
{
    switch (sawState) {
    case SAW_RESTING:
        velocityX = 0.0f;
        restTimer += 0.016f;
        if (restTimer >= REST_DURATION) {
            StartSpinning();
        }
        break;

    case SAW_SPINNING:
        spinTimer += 0.016f;
        rotationAngle += SPIN_SPEED;
        if (rotationAngle >= 2.0f * 3.14159265f) {
            rotationAngle = 0.0f;
        }

        if (spinTimer >= SPIN_DURATION) {
            StartMoving();
        }
        break;

    case SAW_MOVING:
        moveTimer += 0.016f;
        rotationAngle += SPIN_SPEED * 0.5f;

        velocityX = facingRight ? MOVE_SPEED : -MOVE_SPEED;

        if (moveTimer >= MOVE_DURATION) {
            sawState = SAW_RESTING;
            restTimer = 0.0f;
            moveTimer = 0.0f;
        }
        break;

    case SAW_ATTACKING:
        attackTimer += 0.016f;
        rotationAngle += SPIN_SPEED * 2.0f; // ������]

        velocityX = attackDirection * ATTACK_SPEED;

        if (attackTimer >= ATTACK_DURATION) {
            sawState = SAW_RESTING;
            restTimer = 0.0f;
            attackTimer = 0.0f;
            velocityX = 0.0f;
        }
        break;
    }
}

void Saw::HandlePlayerDetectionSaw(Player* player)
{
    if (!player || currentState == DAMAGED) return;

    float distance = GetDistanceToPlayer(player);

    if (distance <= attackRange && sawState == SAW_MOVING) {
        StartAttacking();
        attackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
        facingRight = (attackDirection > 0);
    }
}

void Saw::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    // �m�R�M���͔��Ɋ댯 - ���݂��s��
    float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
    player->TakeDamage(2, knockbackDirection); // ��_���[�W

    OutputDebugStringA("Saw: Caused heavy damage to player!\n");
}

void Saw::StartSpinning() {
    sawState = SAW_SPINNING;
    spinTimer = 0.0f;
    currentState = ATTACKING;
}

void Saw::StartMoving() {
    sawState = SAW_MOVING;
    moveTimer = 0.0f;
    currentState = WALKING;
}

void Saw::StartAttacking() {
    sawState = SAW_ATTACKING;
    attackTimer = 0.0f;
    currentState = ATTACKING;
}

void Saw::UpdateSawAnimation() {
    // �m�R�M���͏�ɉ�]�A�j���[�V����
    if (sawState == SAW_SPINNING || sawState == SAW_MOVING || sawState == SAW_ATTACKING) {
        animationTimer += 0.3f; // ������]
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }

        // ��Ԃɉ�����EnemyState�̐ݒ�
        switch (sawState) {
        case SAW_SPINNING:
            currentState = DAMAGED; // �������
            break;
        case SAW_MOVING:
            currentState = WALKING;
            break;
        case SAW_ATTACKING:
            currentState = ATTACKING;
            break;
        default:
            currentState = IDLE;
            break;
        }
    }
    else {
        animationTimer = 0.0f;
        animationFrame = false;
        currentState = IDLE;
    }
}

int Saw::GetCurrentSprite() {
    return GetSawSprite();
}

int Saw::GetSawSprite() {
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (sawState) {
    case SAW_RESTING:
        return sprites.idle;

    case SAW_SPINNING:
    case SAW_MOVING:
    case SAW_ATTACKING:
        return animationFrame ? sprites.walk_b : sprites.walk_a;

    default:
        return sprites.idle;
    }
}

// �m�R�M�����L�̃_���[�W����
void Saw::TakeDamage(int damage) {
    // �m�R�M���͔��Ɋ�� - �_���[�W�𔼌�
    EnemyBase::TakeDamage(damage / 2);

    if (health > 0) {
        // �_���[�W���󂯂Ă���]�𑱂���
        if (sawState == SAW_RESTING) {
            StartSpinning();
        }
    }
}