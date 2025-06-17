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
    // �g�Q�X���C���̃p�����[�^�ݒ�
    health = 80;
    maxHealth = 80;
    moveSpeed = 1.0f;
    detectionRange = 180.0f;
    attackRange = 80.0f;
    attackPower = 15;

    // �p�g���[���͈͂������ݒ�i���܂蓮�����Ȃ��j
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

    sprites.idle = LoadGraph((basePath + "rest.png").c_str());        // �x�����
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());    // ���s1
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());    // ���s2
    sprites.flat = LoadGraph((basePath + "flat.png").c_str());       // �ׂꂽ���

    // �U������walk_b���g�p�i�g�Q���o����ԁj
    sprites.attack = sprites.walk_b;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.flat;
}

void SpikeSlime::UpdateBehavior(Player* player, StageManager* stageManager)
{
    // �g�Q�X���C���ŗL�̈ړ�����
    UpdateSpikeSlimeMovement();

    // �v���C���[���o�ƍs�����f
    CheckPlayerInAlertRange(player);

    // ��Ԃɉ���������
    switch (spikeState) {
    case SPIKE_DORMANT:
        // �x�����͊�{�I�ȃp�g���[��
        if (currentState == IDLE) {
            UpdatePatrol();
        }
        break;

    case SPIKE_ALERT:
        alertTimer += 0.016f;
        velocityX = 0.0f; // �x�����͒�~
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
        velocityX = 0.0f; // �`���[�W���͒�~

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
        velocityX = 0.0f; // �N�[���_�E�����͒�~

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

    // �g�Q�X���C���ŗL�̃A�j���[�V����
    UpdateSpikeSlimeAnimation();
}

void SpikeSlime::UpdateSpikeSlimeMovement()
{
    // �������ʂ̍X�V
    pulsePhase += PULSE_SPEED;
    if (pulsePhase >= 2.0f * 3.14159265f) {
        pulsePhase = 0.0f;
    }
}

void SpikeSlime::UpdateSpikeSlimeAnimation()
{
    // ��{�A�j���[�V�����X�V
    UpdateAnimation();

    // �`���[�W���̓��ʂȉ��o
    if (spikeState == SPIKE_CHARGING) {
        // �_�Ō��ʂȂǂ�ǉ��\
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

    // �v���C���[�̕����ɓːi
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

void SpikeSlime::OnPlayerCollision(Player* player)
{
    if (!player) return;

    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();
    float enemyTop = y - COLLISION_HEIGHT / 2;

    // **�ڍׂȓ��ݔ���: �v���C���[���ォ�瓥�񂾏ꍇ**
    bool isStompedFromAbove = (
        playerVelY > 0.5f &&                    // �������̑��x
        playerY < y - 10.0f &&                  // �v���C���[���G����
        playerY + 50.0f >= enemyTop             // �����G�̓����t��
        );

    if (isStompedFromAbove) {
        if (isSpikesOut) {
            // **�g�Q���o�Ă���Ƃ��ɓ��܂��ƃv���C���[���_���[�W**

            if (!player->IsInvulnerable()) {
                // �m�b�N�o�b�N�������v�Z
                float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;

                // **�v���C���[�Ƀ_���[�W��^����**
                player->TakeDamage(2, knockbackDirection); // �g�Q�_���[�W��2

                OutputDebugStringA("SpikeSlime: Player stepped on spikes - taking damage!\n");
            }

            // **�G�͓��Ƀ_���[�W���󂯂Ȃ��i�g�Q�Ŏ���Ă���j**

        }
        else {
            // **�g�Q����������ł���Ƃ��ɓ��܂��ƃX�^����Ԃ�**
            spikeState = SPIKE_STUNNED;
            stunTimer = 0.0f;
            currentState = DAMAGED;
            velocityX = 0.0f;
            isSpikesOut = false;

            OutputDebugStringA("SpikeSlime: Stomped while spikes retracted - stunned!\n");

            // **����: �v���C���[�̒��˕Ԃ��EnemyManager�ŏ����ς�**
        }
    }
    else {
        // **���≺����̐ڐG**
        if (isSpikesOut) {
            // **�g�Q���o�Ă���Ƃ��̉��ڐG�͑�_���[�W**

            if (!player->IsInvulnerable()) {
                // �m�b�N�o�b�N�������v�Z
                float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;

                // **���͂ȃ_���[�W**
                player->TakeDamage(2, knockbackDirection); // �g�Q�_���[�W��2

                OutputDebugStringA("SpikeSlime: Player hit spikes from side - heavy damage!\n");
            }

        }
        else {
            // **�ʏ�̃_���[�W**

            if (!player->IsInvulnerable()) {
                // �m�b�N�o�b�N�������v�Z
                float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;

                // **�ʏ�_���[�W**
                player->TakeDamage(1, knockbackDirection);

                OutputDebugStringA("SpikeSlime: Normal side collision - standard damage!\n");
            }

            // **�ʏ�ڐG���̓G�̔����i�y���m�b�N�o�b�N�j**
            float enemyKnockback = (player->GetX() > x) ? -1.0f : 1.0f;
            velocityX = enemyKnockback * 1.5f;
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
        return sprites.attack; // �g�Q���o�����

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

    // ��ʊO�`�F�b�N
    if (screenX < -100 || screenX > 1920 + 100) return;

    // �g�Q�̊댯�G�t�F�N�g
    float intensity = 0.5f + 0.5f * sinf(pulsePhase * 3.0f);
    int alpha = (int)(100 * intensity);

    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
    DrawCircle(screenX, screenY, 40, GetColor(255, 100, 100), FALSE);
    DrawCircle(screenX, screenY, 35, GetColor(255, 150, 150), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}