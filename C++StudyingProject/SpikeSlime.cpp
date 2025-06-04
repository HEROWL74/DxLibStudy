#include "SpikeSlime.h"
#include <math.h>

SpikeSlime::SpikeSlime(float startX, float startY)
    : EnemyBase(startX, startY, SPIKE_SLIME)
    , spikeState(SPIKE_DORMANT)
    , chargeTimer(0.0f)
    , alertTimer(0.0f)
    , cooldownTimer(0.0f)
    , stunTimer(0.0f)
    , pulsePhase(0.0f)
    , isSpikesOut(false)
{
    // トゲスライムのパラメータ設定
    health = 80;
    maxHealth = 80;
    moveSpeed = 1.0f;
    detectionRange = 180.0f;
    attackRange = 80.0f;
    attackPower = 15;

    // パトロール範囲を狭く設定（あまり動き回らない）
    patrolDistance = 64.0f;
    patrolStartX = startX - 32.0f;
    patrolEndX = startX + 32.0f;
}

void SpikeSlime::Initialize()
{
    LoadSprites();
    spikeState = SPIKE_DORMANT;
    currentState = IDLE;
    isSpikesOut = false;
}

void SpikeSlime::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/slime_spike_";

    sprites.idle = LoadGraph((basePath + "rest.png").c_str());        // 休息状態
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());    // 歩行1
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());    // 歩行2
    sprites.flat = LoadGraph((basePath + "flat.png").c_str());       // 潰れた状態

    // 攻撃時はwalk_bを使用（トゲが出た状態）
    sprites.attack = sprites.walk_b;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.flat;
}

void SpikeSlime::UpdateBehavior(Player* player, StageManager* stageManager)
{
    // トゲスライム固有の移動処理
    UpdateSpikeSlimeMovement();

    // プレイヤー検出と行動判断
    CheckPlayerInAlertRange(player);

    // 状態に応じた処理
    switch (spikeState) {
    case SPIKE_DORMANT:
        // 休眠中は基本的なパトロール
        if (currentState == IDLE) {
            UpdatePatrol();
        }
        break;

    case SPIKE_ALERT:
        alertTimer += 0.016f;
        velocityX = 0.0f; // 警戒中は停止
        FacePlayer(player);

        if (alertTimer >= ALERT_DURATION) {
            if (IsPlayerInRange(player, attackRange)) {
                StartChargeAttack(player);
            }
            else {
                spikeState = SPIKE_DORMANT;
                alertTimer = 0.0f;
            }
        }
        break;

    case SPIKE_CHARGING:
        chargeTimer += 0.016f;
        velocityX = 0.0f; // チャージ中は停止

        if (chargeTimer >= CHARGE_DURATION) {
            ExecuteChargeAttack(player);
        }
        break;

    case SPIKE_ATTACKING:
        chargeTimer += 0.016f;

        if (chargeTimer >= CHARGE_DURATION + ATTACK_DURATION) {
            EnterCooldownState();
        }
        break;

    case SPIKE_COOLDOWN:
        cooldownTimer += 0.016f;
        velocityX = 0.0f; // クールダウン中は停止

        if (cooldownTimer >= COOLDOWN_DURATION) {
            spikeState = SPIKE_DORMANT;
            cooldownTimer = 0.0f;
            isSpikesOut = false;
        }
        break;

    case SPIKE_STUNNED:
        HandleStunState();
        break;
    }

    // トゲスライム固有のアニメーション
    UpdateSpikeSlimeAnimation();
}

void SpikeSlime::UpdateSpikeSlimeMovement()
{
    // 脈動効果の更新
    pulsePhase += PULSE_SPEED;
    if (pulsePhase >= 2.0f * 3.14159265f) {
        pulsePhase = 0.0f;
    }
}

void SpikeSlime::UpdateSpikeSlimeAnimation()
{
    // 基本アニメーション更新
    UpdateAnimation();

    // チャージ中の特別な演出
    if (spikeState == SPIKE_CHARGING) {
        // 点滅効果などを追加可能
    }
}

void SpikeSlime::CheckPlayerInAlertRange(Player* player)
{
    if (!player || spikeState == SPIKE_STUNNED || spikeState == SPIKE_COOLDOWN) return;

    float distanceToPlayer = GetDistanceToPlayer(player);

    if (distanceToPlayer <= detectionRange && spikeState == SPIKE_DORMANT) {
        spikeState = SPIKE_ALERT;
        alertTimer = 0.0f;
        currentState = IDLE;
    }
}

void SpikeSlime::StartChargeAttack(Player* player)
{
    spikeState = SPIKE_CHARGING;
    chargeTimer = 0.0f;
    currentState = ATTACKING;
    FacePlayer(player);
}

void SpikeSlime::ExecuteChargeAttack(Player* player)
{
    spikeState = SPIKE_ATTACKING;
    isSpikesOut = true;

    // プレイヤーの方向に突進
    if (player) {
        float direction = (player->GetX() > x) ? 1.0f : -1.0f;
        velocityX = direction * CHARGE_SPEED;
        facingRight = (direction > 0);
    }
}

void SpikeSlime::EnterCooldownState()
{
    spikeState = SPIKE_COOLDOWN;
    cooldownTimer = 0.0f;
    chargeTimer = 0.0f;
    velocityX = 0.0f;
    currentState = IDLE;
}

void SpikeSlime::HandleStunState()
{
    stunTimer += 0.016f;
    velocityX = 0.0f;
    isSpikesOut = false;

    if (stunTimer >= STUN_DURATION) {
        spikeState = SPIKE_DORMANT;
        stunTimer = 0.0f;
        currentState = IDLE;
    }
}

// **修正: プレイヤーとの衝突処理 - SpikeSlimeは踏んでもダメージ**
void SpikeSlime::OnPlayerCollision(Player* player)
{
    if (!player) return;

    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();
    float enemyTop = y - COLLISION_HEIGHT / 2;

    // **詳細な踏み判定: プレイヤーが上から踏んだ場合**
    bool isStompedFromAbove = (playerY < enemyTop && playerVelY > 0);

    if (isStompedFromAbove) {
        if (isSpikesOut) {
            // **トゲが出ている時に踏まれるとプレイヤーがダメージ**
            // 実際のダメージ処理はGameScene側で実装される
            // ここでは敵側の反応のみ実装

            // **トゲ接触のエフェクトやサウンド**
            // PlaySoundEffect("spike_contact");
            // SpawnParticleEffect(x, y, "spike_spark");

            // **敵は特にダメージを受けない（トゲで守られている）**

        }
        else {
            // **トゲが出ていない時に踏まれるとスタン状態になる**
            spikeState = SPIKE_STUNNED;
            stunTimer = 0.0f;
            currentState = DAMAGED;
            velocityX = 0.0f;
            isSpikesOut = false;

            // **スタン時のエフェクト**
            // PlaySoundEffect("slime_stun");
            // SpawnParticleEffect(x, y, "stun_effect");
        }
    }
    else {
        // **横や下からの接触**
        if (isSpikesOut) {
            // **トゲが出ている時の横接触は大ダメージ**
            // 実際のダメージ処理はGameScene側で実装される

            // **強力な接触エフェクト**
            // PlaySoundEffect("spike_damage");
            // SpawnParticleEffect(x, y, "spike_burst");

        }
        else {
            // **通常のダメージ**
            // 実際のダメージ処理はGameScene側で実装される

            // **通常接触時の敵の反応（軽いノックバック）**
            float knockbackDirection = (player->GetX() > x) ? -1.0f : 1.0f;
            velocityX = knockbackDirection * 1.5f;
        }
    }
}

int SpikeSlime::GetCurrentSprite()
{
    return GetSpikeSlimeSprite();
}

int SpikeSlime::GetSpikeSlimeSprite()
{
    switch (spikeState) {
    case SPIKE_STUNNED:
        return sprites.flat;

    case SPIKE_CHARGING:
    case SPIKE_ATTACKING:
        return sprites.attack; // トゲが出た状態

    case SPIKE_ALERT:
        return sprites.idle;

    case SPIKE_DORMANT:
    case SPIKE_COOLDOWN:
        if (currentState == WALKING) {
            return animationFrame ? sprites.walk_b : sprites.walk_a;
        }
        return sprites.idle;

    default:
        return sprites.idle;
    }
}

void SpikeSlime::DrawSpikeEffect(float cameraX)
{
    if (!isSpikesOut) return;

    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // 画面外チェック
    if (screenX < -100 || screenX > 1920 + 100) return;

    // トゲの危険エフェクト
    float intensity = 0.5f + 0.5f * sinf(pulsePhase * 3.0f);
    int alpha = (int)(100 * intensity);

    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
    DrawCircle(screenX, screenY, 40, GetColor(255, 100, 100), FALSE);
    DrawCircle(screenX, screenY, 35, GetColor(255, 150, 150), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}