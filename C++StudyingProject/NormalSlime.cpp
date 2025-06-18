#include "NormalSlime.h"
#include <math.h>

NormalSlime::NormalSlime(float startX, float startY)
    : EnemyBase(startX, startY, NORMAL_SLIME)
    , slimeState(SLIME_RESTING)
    , jumpTimer(0.0f)
    , restTimer(0.0f)
    , flattenedTimer(0.0f)
    , bobPhase(0.0f)
    , flattenScale(1.0f)
    , flattenProgress(0.0f)
{
    // �m�[�}���X���C���̃p�����[�^�ݒ�
    health = 50;
    maxHealth = 50;
    moveSpeed = 1.5f;
    detectionRange = 120.0f;
    attackRange = 40.0f;
    attackPower = 5;

    // �p�g���[���͈͂������ݒ�
    patrolDistance = 128.0f;
    patrolStartX = startX - 64.0f;
    patrolEndX = startX + 64.0f;
}

void NormalSlime::Initialize()
{
    LoadSprites();
    slimeState = SLIME_RESTING;
    currentState = IDLE;
    restTimer = 0.0f;
}

void NormalSlime::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/slime_normal_";

    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.flat = LoadGraph((basePath + "flat.png").c_str());

    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.flat;
}

void NormalSlime::UpdateBehavior(Player* player, StageManager* stageManager)
{
    // **���S��ԂȂ牽�����Ȃ�**
    if (currentState == DEAD || !active) {
        return;
    }

    // **�ׂꂽ��Ԃ̏����i���ǔŁj**
    if (slimeState == SLIME_FLATTENED) {
        UpdateFlattenedAnimation();
        CheckFlattenedRecovery();
        return;
    }

    // �X���C���ŗL�̈ړ�����
    UpdateSlimeMovement();

    // �v���C���[���߂��ɂ���ꍇ�̓��ʂȍs��
    if (IsPlayerInRange(player, detectionRange)) {
        HandleJumpMovement(player);
    }
    else {
        // �ʏ�̃p�g���[��
        UpdatePatrol();
    }

    // �X���C���ŗL�̃A�j���[�V����
    UpdateSlimeAnimation();
}

void NormalSlime::UpdateFlattenedAnimation()
{
    // **�ׂ�A�j���[�V�����̍X�V**
    if (flattenProgress < 1.0f) {
        flattenProgress += 0.08f; // �������ƒׂ��
        if (flattenProgress > 1.0f) {
            flattenProgress = 1.0f;
        }

        // �ׂ��X�P�[���v�Z�iY�������ɏk�ށAX�������ɍL����j
        flattenScale = 1.0f - flattenProgress * 0.7f; // 30%�̍����܂ŏk��
    }
}

void NormalSlime::UpdateSlimeMovement()
{
    switch (slimeState) {
    case SLIME_RESTING:
        velocityX = 0.0f;
        restTimer += 0.016f;

        if (restTimer >= REST_DURATION) {
            slimeState = SLIME_ACTIVE;
            restTimer = 0.0f;
            currentState = WALKING;
        }
        break;

    case SLIME_ACTIVE:
        jumpTimer += 0.016f;

        if (jumpTimer >= JUMP_INTERVAL && onGround) {
            StartJump();
        }

        if (jumpTimer > JUMP_INTERVAL * 3) {
            slimeState = SLIME_RESTING;
            jumpTimer = 0.0f;
            currentState = IDLE;
        }
        break;

    case SLIME_JUMPING:
        if (onGround && velocityY >= 0) {
            slimeState = SLIME_ACTIVE;
            jumpTimer = 0.0f;
        }
        break;
    }
}

void NormalSlime::UpdateSlimeAnimation()
{
    if (slimeState == SLIME_RESTING) {
        bobPhase += BOB_SPEED;
        if (bobPhase >= 2.0f * 3.14159265f) {
            bobPhase = 0.0f;
        }
    }

    UpdateAnimation();
}

void NormalSlime::StartJump()
{
    if (!onGround) return;

    velocityY = JUMP_POWER;
    slimeState = SLIME_JUMPING;
    jumpTimer = 0.0f;
    onGround = false;
}

void NormalSlime::HandleJumpMovement(Player* player)
{
    if (!player || slimeState == SLIME_FLATTENED) return;

    float playerX = player->GetX();

    if (slimeState == SLIME_JUMPING) {
        float direction = (playerX > x) ? 1.0f : -1.0f;
        velocityX = direction * moveSpeed * 0.5f;
        facingRight = (direction > 0);
    }
    else if (slimeState == SLIME_ACTIVE) {
        float direction = (playerX > x) ? 1.0f : -1.0f;
        velocityX = direction * moveSpeed;
        facingRight = (direction > 0);
    }
}

void NormalSlime::CheckFlattenedRecovery()
{
    flattenedTimer += 0.016f;
    velocityX = 0.0f;

    // **���ǔŁF�ׂꂽ��Ԃ̏���**
    if (flattenedTimer >= FLATTENED_DISPLAY_DURATION) {
        // **���S�ɔj��**
        active = false;
        currentState = DEAD;
        health = 0;

        // **�f�o�b�O�o��**
        OutputDebugStringA("NormalSlime: Completely destroyed after flattening animation!\n");
    }
}

void NormalSlime::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();
    float enemyTop = y - COLLISION_HEIGHT / 2;

    // **���݂�����𖾊m��**
    bool isStompedFromAbove = (
        playerVelY >= 0.0f &&                   // �v���C���[���������Ɉړ�
        playerY < y - 10.0f &&                  // �v���C���[���G����
        playerY + 50.0f >= enemyTop             // �v���C���[�̑����G�̓����t��
        );

    if (isStompedFromAbove) {
        // **���݂������F�����ɓ|�����A�A�j���[�V�����J�n**
        slimeState = SLIME_FLATTENED;
        currentState = DAMAGED; // �ꎞ�I��DAMAGED���
        flattenedTimer = 0.0f;
        flattenProgress = 0.0f;
        flattenScale = 1.0f;
        velocityX = 0.0f;
        velocityY = 0.0f;

        // **�f�o�b�O�o��**
        OutputDebugStringA("NormalSlime: STOMPED! Starting flatten animation!\n");

        // **����: �v���C���[�̒��˕Ԃ��EnemyManager�ŏ����ς�**

    }
    else {
        // **������̐ڐG: �v���C���[�Ƀ_���[�W**

        // **�v���C���[�����G��ԂłȂ���΃_���[�W��^����**
        if (!player->IsInvulnerable()) {
            // �m�b�N�o�b�N�������v�Z
            float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;

            // **�C��: Player::TakeDamage �݂̂��g�p**
            // GameScene�̃��C�t�Ǘ��́AEnemyManager�o�R�ŏ��������z��
            player->TakeDamage(1, knockbackDirection);

            // **�f�o�b�O�o��**
            OutputDebugStringA("NormalSlime: Side collision - calling Player::TakeDamage!\n");
        }

        // �G�ɂ������m�b�N�o�b�N
        float enemyKnockback = (player->GetX() > x) ? -1.0f : 1.0f;
        velocityX = enemyKnockback * 1.5f;

        if (currentState != ATTACKING) {
            currentState = ATTACKING;
            stateTimer = 0.0f;
        }
    }
}

// **�`��֐����I�[�o�[���C�h�i�ׂ�A�j���[�V�����Ή��j**
void NormalSlime::Draw(float cameraX)
{
    if (!active) return;

    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // ��ʍ��W�ɕϊ�
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // ��ʊO�`�F�b�N
    if (screenX < -ENEMY_WIDTH || screenX > 1920 + ENEMY_WIDTH) return;
    if (screenY < -ENEMY_HEIGHT || screenY > 1080 + ENEMY_HEIGHT) return;

    // �X�v���C�g�T�C�Y�擾
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    // ���������ŕ`��ʒu����
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // **�ׂ�A�j���[�V�������̓��ʂȕ`��**
    if (slimeState == SLIME_FLATTENED) {
        // Y�������ɏk�݁AX�������ɍL����
        int flatWidth = (int)(spriteWidth * (1.0f + (1.0f - flattenScale) * 0.8f));
        int flatHeight = (int)(spriteHeight * flattenScale);

        // �ׂꂽ������Y�ʒu�𒲐��i�n�ʂɍ��킹��j
        int flatY = screenY + (spriteHeight - flatHeight);

        // �����x�̒����i���X�ɔ����Ȃ�j
        int alpha = (int)(255 * (1.0f - flattenProgress * 0.5f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // �ׂꂽ��Ԃŕ`��
        if (facingRight) {
            DrawExtendGraph(screenX - (flatWidth - spriteWidth) / 2, flatY,
                screenX - (flatWidth - spriteWidth) / 2 + flatWidth, flatY + flatHeight,
                currentSprite, TRUE);
        }
        else {
            // �������̏ꍇ��DrawTurnGraph���g�p�i�ʏ�T�C�Y�ŕ`�悵�Ă��甽�]�j
            DrawExtendGraph(screenX - (flatWidth - spriteWidth) / 2, flatY,
                screenX - (flatWidth - spriteWidth) / 2 + flatWidth, flatY + flatHeight,
                currentSprite, TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else {
        // **�ʏ�̕`�揈��**
        // ��Ԃɉ������`�����
        if (currentState == DAMAGED) {
            // �_���[�W���̓_�Ō���
            int alpha = (int)(255 * (0.5f + 0.5f * sinf(stateTimer * 20.0f)));
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        }

        // ���E���]�`��
        if (facingRight) {
            DrawGraph(screenX, screenY, currentSprite, TRUE);
        }
        else {
            DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
        }

        // �`�惂�[�h���Z�b�g
        if (currentState == DAMAGED) {
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // �f�o�b�O���̕`��
#ifdef _DEBUG
    if (CheckHitKey(KEY_INPUT_F2)) {
        DrawDebugInfo(cameraX);
    }
#endif
}

int NormalSlime::GetCurrentSprite()
{
    return GetSlimeSprite();
}

int NormalSlime::GetSlimeSprite()
{
    // **���S��Ԃ܂��ׂ͒ꂽ��Ԃ̏ꍇ�͏��flat�X�v���C�g**
    if (currentState == DEAD || !active || slimeState == SLIME_FLATTENED) {
        return sprites.flat;
    }

    switch (slimeState) {
    case SLIME_RESTING:
        return sprites.idle;

    case SLIME_ACTIVE:
    case SLIME_JUMPING:
        if (currentState == WALKING || slimeState == SLIME_JUMPING) {
            return animationFrame ? sprites.walk_b : sprites.walk_a;
        }
        return sprites.idle;

    default:
        return sprites.idle;
    }
}

