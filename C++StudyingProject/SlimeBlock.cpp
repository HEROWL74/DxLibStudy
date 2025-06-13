#include "SlimeBlock.h"
#include <math.h>

SlimeBlock::SlimeBlock(float startX, float startY)
    : EnemyBase(startX, startY, SLIME_BLOCK)
    , blockState(BLOCK_RESTING)
    , restTimer(0.0f)
    , jumpTimer(0.0f)
    , hardenTimer(0.0f)
    , bounceTimer(0.0f)
    , bobPhase(0.0f)
    , isHardened(false)
{
    health = 80;
    maxHealth = 80;
    moveSpeed = 1.2f;
    detectionRange = 100.0f;
    attackRange = 60.0f;
    attackPower = 8;

    // �p�g���[���͈͐ݒ�
    patrolDistance = 100.0f;
    patrolStartX = startX - 50.0f;
    patrolEndX = startX + 50.0f;
}

void SlimeBlock::Initialize()
{
    LoadSprites();
    blockState = BLOCK_RESTING;
    currentState = IDLE;
    isHardened = false;
}

void SlimeBlock::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/slime_block_";
    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.jump = LoadGraph((basePath + "jump.png").c_str());
    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.idle;
    sprites.flat = sprites.idle;
}

void SlimeBlock::UpdateBehavior(Player* player, StageManager* stageManager)
{
    if (currentState == DEAD || !active) return;

    UpdateSlimeBlockMovement();

    // �v���C���[���߂��ɂ���ꍇ�̓��ʂȍs��
    if (IsPlayerInRange(player, detectionRange)) {
        if (blockState == BLOCK_RESTING && !isHardened) {
            StartHardening();
        }
    }

    UpdateSlimeBlockAnimation();
}

void SlimeBlock::UpdateSlimeBlockMovement()
{
    switch (blockState) {
    case BLOCK_RESTING:
    {
        velocityX = 0.0f;
        restTimer += 0.016f;

        if (isHardened) {
            hardenTimer += 0.016f;
            if (hardenTimer >= HARDEN_DURATION) {
                isHardened = false;
                hardenTimer = 0.0f;
            }
        }

        if (restTimer >= REST_DURATION && !isHardened) {
            StartJumping();
        }
    }
    break;

    case BLOCK_ACTIVE:
        jumpTimer += 0.016f;
        UpdatePatrol();

        if (jumpTimer >= JUMP_INTERVAL && onGround) {
            velocityY = JUMP_POWER;
            blockState = BLOCK_JUMPING;
            onGround = false;
            currentState = ATTACKING;
        }
        break;

    case BLOCK_JUMPING:
        if (onGround && velocityY >= 0) {
            blockState = BLOCK_ACTIVE;
            jumpTimer = 0.0f;
            currentState = WALKING;
        }
        break;

    case BLOCK_BOUNCING:
        bounceTimer += 0.016f;
        if (bounceTimer >= 0.5f) {
            blockState = BLOCK_RESTING;
            restTimer = 0.0f;
            bounceTimer = 0.0f;
            currentState = IDLE;
        }
        break;
    }
}

void SlimeBlock::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerVelY = player->GetVelocityY();
    bool isStompedFromAbove = (
        playerVelY > 0.5f &&
        player->GetY() < y - 10.0f
        );

    if (isStompedFromAbove) {
        if (isHardened) {
            // �d����Ԃł͓��݂������A�v���C���[�𒵂˕Ԃ�
            HandlePlayerBounce(player);
            OutputDebugStringA("SlimeBlock: Player bounced off hardened slime!\n");
        }
        else {
            // �ʏ��Ԃł͓��݂��Ō��j�\
            TakeDamage(100);
            player->ApplyStompBounce(-12.0f);
            OutputDebugStringA("SlimeBlock: STOMPED and defeated!\n");
        }
    }
    else {
        // ������̐ڐG
        float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;
        player->TakeDamage(1, knockbackDirection);
    }
}

void SlimeBlock::HandlePlayerBounce(Player* player)
{
    if (!player) return;

    // �v���C���[���������˕Ԃ�
    player->ApplyStompBounce(BOUNCE_FORCE);

    // �������������˕Ԃ�
    blockState = BLOCK_BOUNCING;
    bounceTimer = 0.0f;
    velocityY = BOUNCE_FORCE * 0.3f;
}

void SlimeBlock::StartJumping() {
    blockState = BLOCK_ACTIVE;
    jumpTimer = 0.0f;
    restTimer = 0.0f;
    currentState = WALKING;
}

void SlimeBlock::StartHardening() {
    isHardened = true;
    hardenTimer = 0.0f;
    currentState = DAMAGED; // �d����Ԃ�����

    OutputDebugStringA("SlimeBlock: Hardened state activated!\n");
}

void SlimeBlock::UpdateSlimeBlockAnimation() {
    if (blockState == BLOCK_RESTING) {
        // �x�e���̏㉺�h��
        bobPhase += BOB_SPEED;
        if (bobPhase >= 2.0f * 3.14159265f) {
            bobPhase = 0.0f;
        }

        animationTimer = 0.0f;
        animationFrame = false;
        currentState = isHardened ? DAMAGED : IDLE;
    }
    else if (blockState == BLOCK_ACTIVE || blockState == BLOCK_JUMPING) {
        animationTimer += 0.1f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }

        if (blockState == BLOCK_JUMPING) {
            currentState = ATTACKING;
        }
        else {
            currentState = WALKING;
        }
    }
}

int SlimeBlock::GetCurrentSprite() {
    return GetSlimeBlockSprite();
}

int SlimeBlock::GetSlimeBlockSprite() {
    if (currentState == DEAD || !active) {
        return sprites.dead;
    }

    switch (blockState) {
    case BLOCK_RESTING:
        if (isHardened) {
            return sprites.damaged; // �d����Ԃ͓��ʂȐF
        }
        return sprites.idle;

    case BLOCK_ACTIVE:
        return animationFrame ? sprites.walk_b : sprites.walk_a;

    case BLOCK_JUMPING:
        return sprites.jump;

    case BLOCK_BOUNCING:
        return sprites.jump;

    default:
        return sprites.idle;
    }
}

// ���ʂȕ`�揈��
void SlimeBlock::Draw(float cameraX) {
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

    // �x�e���̏㉺�h�����
    if (blockState == BLOCK_RESTING) {
        float bobOffset = sinf(bobPhase) * BOB_AMPLITUDE;
        screenY += (int)bobOffset;
    }

    // �d����Ԃ̓��ʂȕ`�����
    if (isHardened) {
        // �����������
        SetDrawBright(150, 150, 255);

        // �_�Ō���
        int alpha = (int)(255 * (0.8f + 0.2f * sinf(hardenTimer * 15.0f)));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    }

    // �o�E���X���̓��ʌ���
    if (blockState == BLOCK_BOUNCING) {
        float bounceIntensity = 1.0f - (bounceTimer / 0.5f);
        int blueShift = (int)(50 * bounceIntensity);
        SetDrawBright(255, 255 - blueShift, 255);
    }

    // ���E���]�`��
    if (facingRight) {
        DrawGraph(screenX, screenY, currentSprite, TRUE);
    }
    else {
        DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
    }

    // �`�惂�[�h���Z�b�g
    if (isHardened || blockState == BLOCK_BOUNCING) {
        SetDrawBright(255, 255, 255);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �f�o�b�O���̕`��
#ifdef _DEBUG
    if (CheckHitKey(KEY_INPUT_F2)) {
        DrawDebugInfo(cameraX);

        // �d����Ԃ̃f�o�b�O���
        if (isHardened) {
            std::string hardenInfo = "HARDENED: " + std::to_string(HARDEN_DURATION - hardenTimer).substr(0, 3) + "s";
            DrawString(screenX - 30, screenY - 100, hardenInfo.c_str(), GetColor(100, 100, 255));
        }
    }
#endif
}

// �u���b�N�X���C�����L�̃_���[�W����
void SlimeBlock::TakeDamage(int damage) {
    if (isHardened) {
        // �d����Ԃł̓_���[�W��啝�y��
        EnemyBase::TakeDamage(damage / 4);

        // �_���[�W���󂯂���d������
        isHardened = false;
        hardenTimer = 0.0f;

        OutputDebugStringA("SlimeBlock: Hardened state broken by damage!\n");
    }
    else {
        EnemyBase::TakeDamage(damage);
    }
}