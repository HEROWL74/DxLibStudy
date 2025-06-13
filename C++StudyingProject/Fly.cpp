#include "Fly.h"
#include <math.h>

Fly::Fly(float startX, float startY)
    : EnemyBase(startX, startY, FLY)
    , flyState(FLY_RESTING)
    , buzzTimer(0.0f)
    , restTimer(0.0f)
    , chaseTimer(0.0f)
    , buzzPhaseX(0.0f)
    , buzzPhaseY(0.0f)
    , originalX(startX)
    , originalY(startY)
    , fleeTimer(0.0f)
{
    health = 20;
    maxHealth = 20;
    moveSpeed = 2.5f;
    detectionRange = 80.0f;
    attackRange = 40.0f;
    attackPower = 5;

    // パトロール範囲設定
    patrolDistance = 80.0f;
    patrolStartX = startX - 40.0f;
    patrolEndX = startX + 40.0f;
}

void Fly::Initialize()
{
    LoadSprites();
    flyState = FLY_RESTING;
    currentState = IDLE;
    originalX = x;
    originalY = y;
}

void Fly::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/fly_";
    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "b.png").c_str());
    sprites.jump = sprites.walk_a;
    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.idle;
    sprites.flat = sprites.idle;
}

void Fly::UpdateBehavior(Player* player, StageManager* stageManager)
{
    if (currentState == DEAD || !active) return;

    velocityY = 0.0f; // 空中敵
    UpdateFlyMovement();
    HandlePlayerDetectionFly(player);
    UpdateFlyAnimation();
}

void Fly::UpdateFlyMovement()
{
    switch (flyState) {
    case FLY_RESTING:
        velocityX = 0.0f;
        restTimer += 0.016f;
        if (restTimer >= REST_DURATION) {
            StartBuzzing();
        }
        break;

    case FLY_BUZZING:
    {
        buzzTimer += 0.016f;
        buzzPhaseX += BUZZ_SPEED_X;
        buzzPhaseY += BUZZ_SPEED_Y;

        float buzzOffsetX = sinf(buzzPhaseX) * BUZZ_AMPLITUDE_X;
        float buzzOffsetY = sinf(buzzPhaseY) * BUZZ_AMPLITUDE_Y;

        x = originalX + buzzOffsetX;
        y = originalY + buzzOffsetY;

        if (buzzTimer >= BUZZ_DURATION) {
            flyState = FLY_RESTING;
            restTimer = 0.0f;
            buzzTimer = 0.0f;
        }
    }
    break;

    case FLY_CHASING:
        chaseTimer += 0.016f;
        if (chaseTimer >= CHASE_DURATION) {
            StartFleeing();
        }
        break;

    case FLY_FLEEING:
        fleeTimer += 0.016f;
        if (fleeTimer >= FLEE_DURATION) {
            flyState = FLY_RESTING;
            restTimer = 0.0f;
            fleeTimer = 0.0f;
        }
        break;
    }
}

void Fly::HandlePlayerDetectionFly(Player* player)
{
    if (!player) return;

    float distance = GetDistanceToPlayer(player);

    if (distance <= detectionRange && flyState == FLY_BUZZING) {
        StartChasing();

        // プレイヤーに向かって移動
        float dirX = (player->GetX() > x) ? 1.0f : -1.0f;
        float dirY = (player->GetY() > y) ? 1.0f : -1.0f;

        velocityX = dirX * CHASE_SPEED;
        velocityY = dirY * CHASE_SPEED * 0.5f;
        facingRight = (dirX > 0);
    }
    else if (flyState == FLY_FLEEING) {
        // プレイヤーから逃げる
        float dirX = (player->GetX() > x) ? -1.0f : 1.0f;
        float dirY = (player->GetY() > y) ? -1.0f : 1.0f;

        velocityX = dirX * FLEE_SPEED;
        velocityY = dirY * FLEE_SPEED * 0.3f;
        facingRight = (dirX > 0);
    }
}

void Fly::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    // ハエは弱いので踏みつけで即死
    float playerVelY = player->GetVelocityY();
    if (playerVelY > 0.5f && player->GetY() < y) {
        TakeDamage(100);
        player->ApplyStompBounce(-8.0f);
        OutputDebugStringA("Fly: STOMPED and defeated!\n");
    }
    else {
        float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
        player->TakeDamage(1, knockbackDirection);
        StartFleeing(); // 接触したら逃げる
    }
}

void Fly::StartBuzzing() {
    flyState = FLY_BUZZING;
    buzzTimer = 0.0f;
    currentState = WALKING;
}

void Fly::StartChasing() {
    flyState = FLY_CHASING;
    chaseTimer = 0.0f;
    currentState = ATTACKING;
}

void Fly::StartFleeing() {
    flyState = FLY_FLEEING;
    fleeTimer = 0.0f;
    currentState = WALKING;
}

void Fly::UpdateFlyAnimation() {
    if (flyState == FLY_BUZZING || flyState == FLY_CHASING || flyState == FLY_FLEEING) {
        animationTimer += 0.25f; // 高速羽ばたき
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }

        // 状態に応じたEnemyStateの設定
        switch (flyState) {
        case FLY_BUZZING:
            currentState = WALKING;
            break;
        case FLY_CHASING:
            currentState = ATTACKING;
            break;
        case FLY_FLEEING:
            currentState = DAMAGED; // 逃走状態
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

int Fly::GetCurrentSprite() {
    return GetFlySprite();
}

int Fly::GetFlySprite() {
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (flyState) {
    case FLY_RESTING:
        return sprites.idle;
    case FLY_BUZZING:
    case FLY_CHASING:
    case FLY_FLEEING:
        return animationFrame ? sprites.walk_b : sprites.walk_a;
    default:
        return sprites.idle;
    }
}

void Fly::UpdateAnimation()
{
    // EnemyBaseの基本アニメーション処理
    if (currentState == WALKING) {
        animationTimer += 0.15f; // 基本の歩行アニメーション速度
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
    }
    else {
        animationTimer = 0.0f;
        animationFrame = false;
    }

    // ハエ固有のアニメーション更新
    UpdateFlyAnimation();
}