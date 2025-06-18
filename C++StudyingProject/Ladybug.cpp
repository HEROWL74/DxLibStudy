// Ladybug.cpp - ���S�ɓ����蔻��𖳎������

#include "Ladybug.h"
#include <math.h>

Ladybug::Ladybug(float startX, float startY)
    : EnemyBase(startX, startY, LADYBUG)
    , ladybugState(LADYBUG_RESTING)
    , restTimer(0.0f)
    , walkTimer(0.0f)
    , flyTimer(0.0f)
    , flyPhase(0.0f)
    , targetY(startY)
    , originalY(startY)
    , lastPositionX(startX)
{
    health = 40;
    maxHealth = 40;
    moveSpeed = 1.8f;
    detectionRange = 90.0f;
    attackRange = 50.0f;
    attackPower = 6;

    // �p�g���[���͈͐ݒ�
    patrolDistance = 120.0f;
    patrolStartX = startX - 60.0f;
    patrolEndX = startX + 60.0f;
}

void Ladybug::Initialize()
{
    LoadSprites();
    ladybugState = LADYBUG_RESTING;
    currentState = IDLE;
    originalY = y;
    targetY = y;
    lastPositionX = x;
}

void Ladybug::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/ladybug_";
    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.jump = LoadGraph((basePath + "fly.png").c_str());
    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.idle;
    sprites.flat = sprites.idle;
}

void Ladybug::UpdateBehavior(Player* player, StageManager* stageManager)
{
    if (currentState == DEAD || !active) return;

    // **�����蔻������S�ɖ��������X�V**
    UpdateLadybugMovement();

    // **�ʒu�X�V�i�������Z�Ȃ��j**
    x += velocityX;
    y += velocityY;

    // **��ʋ��E�`�F�b�N�̂�**
    CheckScreenBoundaries();

    // **�A�j���[�V�����X�V**
    UpdateLadybugAnimation();

    // **�O�t���[���̈ʒu��ۑ�**
    lastPositionX = x;
}

void Ladybug::CheckScreenBoundaries()
{
    const float SCREEN_MARGIN = 100.0f;
    const float STAGE_WIDTH = 3000.0f;

    // X���W�̐���
    if (x < -SCREEN_MARGIN) {
        x = -SCREEN_MARGIN;
        facingRight = true;
    }
    else if (x > STAGE_WIDTH + SCREEN_MARGIN) {
        x = STAGE_WIDTH + SCREEN_MARGIN;
        facingRight = false;
    }

    // Y���W�̐���
    if (y > 1200.0f) {
        y = 1200.0f;
    }
    else if (y < -200.0f) {
        y = -200.0f;
    }
}

void Ladybug::UpdateLadybugMovement()
{
    switch (ladybugState) {
    case LADYBUG_RESTING:
    {
        // **���S�Î~**
        velocityX = 0.0f;
        velocityY = 0.0f;

        restTimer += 0.016f;
        if (restTimer >= REST_DURATION) {
            StartWalking();
        }
        break;
    }

    case LADYBUG_WALKING:
    {
        walkTimer += 0.016f;

        // **�����ړ��̂݁i�d�͂Ȃ��j**
        float walkSpeed = facingRight ? WALK_SPEED : -WALK_SPEED;
        velocityX = walkSpeed;
        velocityY = 0.0f; // **Y�ړ��Ȃ�**

        // **�p�g���[���͈̓`�F�b�N**
        if (x <= patrolStartX) {
            facingRight = true;
        }
        else if (x >= patrolEndX) {
            facingRight = false;
        }

        if (walkTimer >= WALK_DURATION) {
            StartFlying();
        }
        break;
    }

    case LADYBUG_FLYING:
    {
        flyTimer += 0.016f;
        flyPhase += FLY_SPEED_PHASE;

        // **���S���R��s**
        float flyOffset = sinf(flyPhase) * FLY_AMPLITUDE;
        targetY = originalY - FLY_HEIGHT + flyOffset;

        // **���炩�Ȑ����ړ�**
        velocityY = (targetY - y) * 0.08f;

        // **�����ړ�**
        float flySpeed = facingRight ? FLY_SPEED : -FLY_SPEED;
        velocityX = flySpeed;

        // **��s���̃p�g���[���͈̓`�F�b�N**
        if (x <= patrolStartX) {
            facingRight = true;
        }
        else if (x >= patrolEndX) {
            facingRight = false;
        }

        if (flyTimer >= FLY_DURATION) {
            StartLanding();
        }
        break;
    }

    case LADYBUG_LANDING:
    {
        // **�n�ʂ܂ō~��**
        targetY = originalY;
        velocityY = (targetY - y) * 0.15f;

        // **��������**
        velocityX *= 0.9f;

        // **���n����**
        if (fabsf(y - originalY) < 3.0f) {
            y = originalY;
            velocityY = 0.0f;
            velocityX = 0.0f;
            ladybugState = LADYBUG_RESTING;
            restTimer = 0.0f;
            OutputDebugStringA("Ladybug: Landed without collision checks\n");
        }
        break;
    }
    }
}

void Ladybug::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerVelY = player->GetVelocityY();
    bool isStompedFromAbove = (
        playerVelY >= 0.0f &&
        player->GetY() < y - 10.0f
        );

    if (isStompedFromAbove) {
        TakeDamage(100);
        player->ApplyStompBounce(-10.0f);
        OutputDebugStringA("Ladybug: STOMPED and defeated!\n");
    }
    else {
        float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
        player->TakeDamage(1, knockbackDirection);

        // **�ڐG�����瑦���ɔ�s�J�n**
        if (ladybugState != LADYBUG_FLYING) {
            StartFlying();
            OutputDebugStringA("Ladybug: Player contact - starting free flight\n");
        }
    }
}

void Ladybug::StartWalking()
{
    ladybugState = LADYBUG_WALKING;
    walkTimer = 0.0f;
    currentState = WALKING;
    OutputDebugStringA("Ladybug: Started walking (no collision)\n");
}

void Ladybug::StartFlying()
{
    ladybugState = LADYBUG_FLYING;
    flyTimer = 0.0f;
    flyPhase = 0.0f;
    currentState = ATTACKING;
    OutputDebugStringA("Ladybug: Started flying (no collision)\n");
}

void Ladybug::StartLanding()
{
    ladybugState = LADYBUG_LANDING;
    currentState = DAMAGED;
    OutputDebugStringA("Ladybug: Started landing (no collision)\n");
}

void Ladybug::UpdateLadybugAnimation()
{
    switch (ladybugState) {
    case LADYBUG_RESTING:
    {
        animationTimer = 0.0f;
        animationFrame = false;
        currentState = IDLE;
        break;
    }

    case LADYBUG_WALKING:
    {
        animationTimer += 0.12f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
        currentState = WALKING;
        break;
    }

    case LADYBUG_FLYING:
    case LADYBUG_LANDING:
    {
        animationTimer += 0.08f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
        currentState = ATTACKING;
        break;
    }
    }
}

int Ladybug::GetCurrentSprite()
{
    return GetLadybugSprite();
}

int Ladybug::GetLadybugSprite()
{
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (ladybugState) {
    case LADYBUG_RESTING:
    {
        return sprites.idle;
    }

    case LADYBUG_WALKING:
    {
        return animationFrame ? sprites.walk_b : sprites.walk_a;
    }

    case LADYBUG_FLYING:
    case LADYBUG_LANDING:
    {
        return sprites.jump; // fly sprite
    }

    default:
    {
        return sprites.idle;
    }
    }
}

void Ladybug::PrintDebugInfo()
{
    char debugMsg[256];
    sprintf_s(debugMsg, "Ladybug: State=%d, Pos(%.1f,%.1f), Vel(%.1f,%.1f), NoCollision=TRUE\n",
        ladybugState, x, y, velocityX, velocityY);
    OutputDebugStringA(debugMsg);
}