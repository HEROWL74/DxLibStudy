#include "Bee.h"
#include <math.h>

Bee::Bee(float startX, float startY)
    : EnemyBase(startX, startY, BEE)
    , beeState(BEE_RESTING)
    , hoverTimer(0.0f)
    , restTimer(0.0f)
    , attackTimer(0.0f)
    , hoverPhase(0.0f)
    , originalY(startY)
    , targetY(startY)
    , attackStartY(startY)
{
    // �~�c�o�`�̃p�����[�^�ݒ�
    health = 30;
    maxHealth = 30;
    moveSpeed = 2.0f;
    detectionRange = 100.0f;
    attackRange = 80.0f;
    attackPower = 8;

    // �󒆓G�Ȃ̂ŏd�͖���
    // �p�g���[���͈͐ݒ�
    patrolDistance = 100.0f;
    patrolStartX = startX - 50.0f;
    patrolEndX = startX + 50.0f;
}

void Bee::Initialize()
{
    LoadSprites();
    beeState = BEE_RESTING;
    currentState = IDLE;
    restTimer = 0.0f;
    originalY = y;
    targetY = y;
}

void Bee::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/bee_";

    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "b.png").c_str());
    sprites.jump = sprites.walk_a;

    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.idle;
    sprites.flat = sprites.idle;
}

void Bee::UpdateBehavior(Player* player, StageManager* stageManager)
{
    if (currentState == DEAD || !active) {
        return;
    }

    // �d�͂𖳌����i�󒆓G�j
    velocityY = 0.0f;

    // �~�c�o�`�ŗL�̈ړ�����
    UpdateBeeMovement();

    // �v���C���[���o�ƍU������
    HandlePlayerDetectionBee(player);

    // �~�c�o�`�ŗL�̃A�j���[�V����
    UpdateBeeAnimation();
}

void Bee::UpdateBeeMovement()
{
    switch (beeState) {
    case BEE_RESTING:
        velocityX = 0.0f;
        velocityY = 0.0f;
        restTimer += 0.016f;

        if (restTimer >= REST_DURATION) {
            StartHovering();
        }
        break;

    case BEE_HOVERING:
    {
        // �z�o�����O����
        hoverTimer += 0.016f;
        hoverPhase += HOVER_SPEED;

        // �㉺�ɂӂ�ӂ�Ɠ���
        float hoverOffset = sinf(hoverPhase) * HOVER_AMPLITUDE;
        targetY = originalY + hoverOffset;

        // Y���W�����炩�ɕ��
        y = Lerp(y, targetY, 0.1f);

        // ���E�̃p�g���[��
        UpdatePatrol();

        if (hoverTimer >= HOVER_DURATION) {
            beeState = BEE_RESTING;
            restTimer = 0.0f;
            hoverTimer = 0.0f;
        }
    }
    break;

    case BEE_ATTACKING:
        // �}�~���U��
        attackTimer += 0.016f;
        velocityY = ATTACK_SPEED;

        // �n�ʂɋ߂Â����畜�A�J�n
        if (y > originalY + ATTACK_RANGE_Y || attackTimer > 2.0f) {
            StartReturning();
        }
        break;

    case BEE_RETURNING:
    {
        // ���̈ʒu�ɖ߂�
        float returnDirY = (originalY < y) ? -1.0f : 1.0f;
        velocityY = returnDirY * RETURN_SPEED;

        // ���̈ʒu�t�߂ɖ߂�����x�e��Ԃ�
        if (fabsf(y - originalY) < 10.0f) {
            y = originalY;
            beeState = BEE_RESTING;
            restTimer = 0.0f;
            velocityY = 0.0f;
        }
    }
    break;
    }
}

void Bee::UpdateBeeAnimation()
{
    if (beeState == BEE_HOVERING || beeState == BEE_ATTACKING || beeState == BEE_RETURNING) {
        animationTimer += 0.2f; // �����H�΂���
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }

        // ��Ԃɉ�����EnemyState�̐ݒ�
        switch (beeState) {
        case BEE_HOVERING:
            currentState = WALKING;
            break;
        case BEE_ATTACKING:
            currentState = ATTACKING;
            break;
        case BEE_RETURNING:
            currentState = DAMAGED; // ���A���
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

void Bee::StartHovering()
{
    beeState = BEE_HOVERING;
    hoverTimer = 0.0f;
    hoverPhase = 0.0f;
    currentState = WALKING;
}

void Bee::StartAttack()
{
    beeState = BEE_ATTACKING;
    attackTimer = 0.0f;
    attackStartY = y;
    velocityX = 0.0f; // �U�����͐����ړ���~
    currentState = ATTACKING;
}

void Bee::StartReturning()
{
    beeState = BEE_RETURNING;
    attackTimer = 0.0f;
    velocityX = 0.0f;
}

void Bee::HandlePlayerDetectionBee(Player* player)
{
    if (!player || currentState == DAMAGED) return;

    float distanceToPlayer = GetDistanceToPlayer(player);
    float playerY = player->GetY();

    // �v���C���[�����o�͈͓��ŁA���������ɂ���ꍇ
    if (distanceToPlayer <= detectionRange &&
        playerY > y &&
        (playerY - y) <= ATTACK_RANGE_Y &&
        beeState == BEE_HOVERING) {

        FacePlayer(player);
        StartAttack();
    }
}

void Bee::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // ���݂�����
    bool isStompedFromAbove = (
        playerVelY >= 0.0f &&
        playerY < y - 10.0f &&
        playerY + 50.0f >= y - COLLISION_HEIGHT / 2
        );

    if (isStompedFromAbove) {
        // ���݂��Ō��j
        TakeDamage(100);
        player->ApplyStompBounce(-10.0f);

        OutputDebugStringA("Bee: STOMPED and defeated!\n");
    }
    else {
        // ������̐ڐG�Ń_���[�W
        float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
        player->TakeDamage(1, knockbackDirection);

        OutputDebugStringA("Bee: Damaged player!\n");
    }
}

int Bee::GetCurrentSprite()
{
    return GetBeeSprite();
}

int Bee::GetBeeSprite()
{
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (beeState) {
    case BEE_RESTING:
        return sprites.idle;

    case BEE_HOVERING:
    case BEE_ATTACKING:
    case BEE_RETURNING:
        return animationFrame ? sprites.walk_b : sprites.walk_a;

    default:
        return sprites.idle;
    }
}

void Bee::UpdateAnimation()
{
    // EnemyBase�̊�{�A�j���[�V��������
    if (currentState == WALKING) {
        animationTimer += 0.15f; // ��{�̕��s�A�j���[�V�������x
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
    }
    else {
        animationTimer = 0.0f;
        animationFrame = false;
    }

    // �~�c�o�`�ŗL�̃A�j���[�V�����X�V
    UpdateBeeAnimation();
}
