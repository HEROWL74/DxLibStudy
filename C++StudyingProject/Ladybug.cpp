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
{
    health = 40;
    maxHealth = 40;
    moveSpeed = 1.8f;
    detectionRange = 90.0f;
    attackRange = 50.0f;
    attackPower = 6;

    // パトロール範囲設定
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

    UpdateLadybugMovement();

    if (ladybugState != LADYBUG_FLYING) {
        HandleGroundCollisionLadybug(stageManager);
    }

    UpdateLadybugAnimation();
}

void Ladybug::UpdateLadybugMovement()
{
    switch (ladybugState) {
    case LADYBUG_RESTING:
        velocityX = 0.0f;
        restTimer += 0.016f;
        if (restTimer >= REST_DURATION) {
            StartWalking();
        }
        break;

    case LADYBUG_WALKING:
        walkTimer += 0.016f;
        UpdatePatrol();

        if (walkTimer >= WALK_DURATION) {
            StartFlying();
        }
        break;

    case LADYBUG_FLYING:
    {
        flyTimer += 0.016f;
        flyPhase += FLY_SPEED_PHASE;

        // 飛行中の上下動作
        float flyOffset = sinf(flyPhase) * FLY_AMPLITUDE;
        targetY = originalY - FLY_HEIGHT + flyOffset;
        y = Lerp(y, targetY, 0.08f);

        // 空中での水平移動
        velocityX = facingRight ? FLY_SPEED : -FLY_SPEED;

        if (flyTimer >= FLY_DURATION) {
            StartLanding();
        }
    }
    break;

    case LADYBUG_LANDING:
    {
        // 地面に向かって降下
        targetY = originalY;
        y = Lerp(y, targetY, 0.15f);
        velocityX *= 0.9f; // 減速

        if (fabsf(y - originalY) < 5.0f) {
            y = originalY;
            ladybugState = LADYBUG_RESTING;
            restTimer = 0.0f;
            velocityX = 0.0f;
        }
    }
    break;
    }
}

void Ladybug::HandleGroundCollisionLadybug(StageManager* stageManager)
{
    if (ladybugState == LADYBUG_FLYING) return;

    // 重力適用
    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }

    // 地面判定（簡易版）
    float groundY = FindGroundY(x, stageManager);
    if (groundY != -1 && y + COLLISION_HEIGHT / 2 >= groundY) {
        y = groundY - COLLISION_HEIGHT / 2;
        velocityY = 0.0f;
        onGround = true;
    }
    else {
        onGround = false;
    }
}

void Ladybug::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerVelY = player->GetVelocityY();
    bool isStompedFromAbove = (
        playerVelY > 0.5f &&
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

        // 接触したら飛行開始
        if (ladybugState == LADYBUG_WALKING) {
            StartFlying();
        }
    }
}

void Ladybug::StartWalking() {
    ladybugState = LADYBUG_WALKING;
    walkTimer = 0.0f;
    currentState = WALKING;
}

void Ladybug::StartFlying() {
    ladybugState = LADYBUG_FLYING;
    flyTimer = 0.0f;
    flyPhase = 0.0f;
    currentState = ATTACKING;
}

void Ladybug::StartLanding() {
    ladybugState = LADYBUG_LANDING;
    currentState = DAMAGED;
}

void Ladybug::UpdateLadybugAnimation() {
    switch (ladybugState) {
    case LADYBUG_RESTING:
        animationTimer = 0.0f;
        animationFrame = false;
        currentState = IDLE;
        break;

    case LADYBUG_WALKING:
        animationTimer += 0.12f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
        currentState = WALKING;
        break;

    case LADYBUG_FLYING:
    case LADYBUG_LANDING:
        animationTimer += 0.08f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
        currentState = ATTACKING;
        break;
    }
}

int Ladybug::GetCurrentSprite() {
    return GetLadybugSprite();
}

int Ladybug::GetLadybugSprite() {
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (ladybugState) {
    case LADYBUG_RESTING:
        return sprites.idle;

    case LADYBUG_WALKING:
        return animationFrame ? sprites.walk_b : sprites.walk_a;

    case LADYBUG_FLYING:
    case LADYBUG_LANDING:
        return sprites.jump; // fly sprite

    default:
        return sprites.idle;
    }
}