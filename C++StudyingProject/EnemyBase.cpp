#include "EnemyBase.h"
#include <math.h>
#include <algorithm>

EnemyBase::EnemyBase(float startX, float startY, EnemyType type)
    : x(startX)
    , y(startY)
    , velocityX(0.0f)
    , velocityY(0.0f)
    , facingRight(true)
    , currentState(IDLE)
    , enemyType(type)
    , active(true)
    , onGround(false)
    , health(100)
    , maxHealth(100)
    , moveSpeed(2.0f)
    , detectionRange(150.0f)
    , attackRange(60.0f)
    , attackPower(10)
    , animationTimer(0.0f)
    , animationFrame(false)
    , stateTimer(0.0f)
    , patrolStartX(startX - 128.0f)
    , patrolEndX(startX + 128.0f)
    , patrolDistance(256.0f)
    , patrolDirection(true)
{
    // �X�v���C�g�n���h����������
    sprites.idle = sprites.walk_a = sprites.walk_b = -1;
    sprites.attack = sprites.damaged = sprites.dead = sprites.flat = -1;
}

EnemyBase::~EnemyBase()
{
    // �X�v���C�g�̉��
    if (sprites.idle != -1) DeleteGraph(sprites.idle);
    if (sprites.walk_a != -1) DeleteGraph(sprites.walk_a);
    if (sprites.walk_b != -1) DeleteGraph(sprites.walk_b);
    if (sprites.attack != -1) DeleteGraph(sprites.attack);
    if (sprites.damaged != -1) DeleteGraph(sprites.damaged);
    if (sprites.dead != -1) DeleteGraph(sprites.dead);
    if (sprites.flat != -1) DeleteGraph(sprites.flat);
}

void EnemyBase::Update(Player* player, StageManager* stageManager)
{
    // **���S��Ԃ܂��͔�A�N�e�B�u�Ȃ牽�����Ȃ�**
    if (!active || currentState == DEAD) return;

    // ��ԃ^�C�}�[�̍X�V
    stateTimer += 0.016f; // 60FPS�z��

    // �������Z
    UpdatePhysics(stageManager);

    // �����蔻�菈��
    HandleCollisions(stageManager);

    // �v���C���[���o
    HandlePlayerDetection(player);

    // �G�ŗL�̍s��
    UpdateBehavior(player, stageManager);

    // �A�j���[�V�����X�V
    UpdateAnimation();

    // �v���C���[�Ƃ̓����蔻��
    if (CheckPlayerCollision(player)) {
        OnPlayerCollision(player);
    }

    // ��ʊO�ɗ������ꍇ�̏���
    if (y > 1400) {
        active = false;
        currentState = DEAD;
    }
}

void EnemyBase::Draw(float cameraX)
{
    if (!active) return;

    int currentSprite = GetCurrentSprite();
    if (currentSprite != -1) {
        DrawEnemySprite(cameraX, currentSprite);
    }

    // �f�o�b�O���̕`��
#ifdef _DEBUG
    if (CheckHitKey(KEY_INPUT_F2)) {
        DrawDebugInfo(cameraX);
    }
#endif
}

void EnemyBase::DrawEnemySprite(float cameraX, int spriteHandle)
{
    // ��ʍ��W�ɕϊ�
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // ��ʊO�`�F�b�N
    if (screenX < -ENEMY_WIDTH || screenX > 1920 + ENEMY_WIDTH) return;
    if (screenY < -ENEMY_HEIGHT || screenY > 1080 + ENEMY_HEIGHT) return;

    // �X�v���C�g�T�C�Y�擾
    int spriteWidth, spriteHeight;
    GetGraphSize(spriteHandle, &spriteWidth, &spriteHeight);

    // ���������ŕ`��ʒu����
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // ��Ԃɉ������`�����
    if (currentState == DAMAGED) {
        // �_���[�W���̓_�Ō���
        int alpha = (int)(255 * (0.5f + 0.5f * sinf(stateTimer * 20.0f)));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    }

    // ���E���]�`��
    if (facingRight) {
        DrawGraph(screenX, screenY, spriteHandle, TRUE);
    }
    else {
        DrawTurnGraph(screenX, screenY, spriteHandle, TRUE);
    }

    // �`�惂�[�h���Z�b�g
    if (currentState == DAMAGED) {
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void EnemyBase::DrawDebugInfo(float cameraX)
{
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // �����蔻��{�b�N�X
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(
        screenX - COLLISION_WIDTH / 2,
        screenY - COLLISION_HEIGHT / 2,
        screenX + COLLISION_WIDTH / 2,
        screenY + COLLISION_HEIGHT / 2,
        GetColor(255, 0, 0), FALSE
    );

    // ���o�͈�
    DrawCircle(screenX, screenY, (int)detectionRange, GetColor(255, 255, 0), FALSE);

    // �U���͈�
    DrawCircle(screenX, screenY, (int)attackRange, GetColor(255, 100, 100), FALSE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �X�e�[�^�X���
    std::string stateText = "State: ";
    switch (currentState) {
    case IDLE: stateText += "IDLE"; break;
    case WALKING: stateText += "WALKING"; break;
    case ATTACKING: stateText += "ATTACKING"; break;
    case DAMAGED: stateText += "DAMAGED"; break;
    case DYING: stateText += "DYING"; break;
    case DEAD: stateText += "DEAD"; break;
    }

    DrawString(screenX - 50, screenY - 80, stateText.c_str(), GetColor(255, 255, 255));

    std::string healthText = "HP: " + std::to_string(health) + "/" + std::to_string(maxHealth);
    DrawString(screenX - 50, screenY - 60, healthText.c_str(), GetColor(255, 255, 255));

    std::string activeText = "Active: " + std::string(active ? "YES" : "NO");
    DrawString(screenX - 50, screenY - 40, activeText.c_str(), GetColor(255, 255, 255));
}

void EnemyBase::UpdatePhysics(StageManager* stageManager)
{
    // �d�͓K�p
    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }

    // �ʒu�X�V
    x += velocityX;
    y += velocityY;
}

void EnemyBase::UpdateAnimation()
{
    // ��{�I�ȃA�j���[�V��������
    if (currentState == WALKING) {
        animationTimer += 0.1f;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            animationFrame = !animationFrame;
        }
    }
    else {
        animationTimer = 0.0f;
        animationFrame = false;
    }
}

void EnemyBase::HandleCollisions(StageManager* stageManager)
{
    // �n�ʂƂ̓����蔻��
    float groundY = FindGroundY(x, stageManager);
    if (groundY != -1 && y + COLLISION_HEIGHT / 2 >= groundY) {
        y = groundY - COLLISION_HEIGHT / 2;
        velocityY = 0.0f;
        onGround = true;
    }
    else {
        onGround = false;
    }

    // �ǂƂ̓����蔻��
    if (CheckWallCollision(x, y, stageManager)) {
        velocityX = 0.0f;
        // �p�g���[�������𔽓]
        patrolDirection = !patrolDirection;
        facingRight = patrolDirection;
    }
}

void EnemyBase::UpdatePatrol()
{
    if (currentState != WALKING) return;

    // �p�g���[���ړ�
    if (patrolDirection) {
        velocityX = moveSpeed;
        facingRight = true;
        if (x >= patrolEndX) {
            patrolDirection = false;
        }
    }
    else {
        velocityX = -moveSpeed;
        facingRight = false;
        if (x <= patrolStartX) {
            patrolDirection = true;
        }
    }
}

void EnemyBase::HandlePlayerDetection(Player* player)
{
    if (!player || currentState == DAMAGED || currentState == DYING) return;

    float distanceToPlayer = GetDistanceToPlayer(player);

    // �v���C���[�����o�͈͓��ɂ���ꍇ
    if (distanceToPlayer <= detectionRange) {
        FacePlayer(player);

        // �U���͈͓��̏ꍇ
        if (distanceToPlayer <= attackRange && currentState != ATTACKING) {
            currentState = ATTACKING;
            stateTimer = 0.0f;
        }
        // ���o�͈͓������U���͈͊O�̏ꍇ�͕��s
        else if (distanceToPlayer > attackRange && currentState == IDLE) {
            currentState = WALKING;
        }
    }
    else {
        // �v���C���[���͈͊O�̏ꍇ�̓p�g���[���ɖ߂�
        if (currentState == WALKING || currentState == ATTACKING) {
            currentState = IDLE;
            stateTimer = 0.0f;
        }
    }
}

bool EnemyBase::CheckPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();

    // AABB�����蔻��
    float enemyLeft = x - COLLISION_WIDTH / 2;
    float enemyRight = x + COLLISION_WIDTH / 2;
    float enemyTop = y - COLLISION_HEIGHT / 2;
    float enemyBottom = y + COLLISION_HEIGHT / 2;

    float playerLeft = playerX - 40.0f; // �v���C���[�̓����蔻��̔���
    float playerRight = playerX + 40.0f;
    float playerTop = playerY - 50.0f;
    float playerBottom = playerY + 50.0f;

    return (enemyLeft < playerRight && enemyRight > playerLeft &&
        enemyTop < playerBottom && enemyBottom > playerTop);
}

void EnemyBase::OnPlayerCollision(Player* player)
{
    // ��{�I�ȓ����蔻�菈���i�h���N���X�ŃI�[�o�[���C�h�\�j
    if (!player) return;

    float playerY = player->GetY();
    float enemyTop = y - COLLISION_HEIGHT / 2;

    // �v���C���[���G�̏ォ�瓥�񂾏ꍇ
    if (playerY < enemyTop && player->GetVelocityY() > 0) {
        TakeDamage(100); // �ꌂ�œ|��
        // �v���C���[�ɏ������˕Ԃ��^����i������Player���Łj
    }
    else {
        // �v���C���[�Ƀ_���[�W��^����i������Player���Łj
        // �����ł͏Փ˂����o���邾��
    }
}

// **���S�C����: TakeDamage�֐�**
void EnemyBase::TakeDamage(int damage)
{
    if (currentState == DEAD || currentState == DYING || !active) return;

    health -= damage;

    if (health <= 0) {
        health = 0;

        // **�����Ɏ��S��ԂɈڍs**
        currentState = DEAD;
        active = false;           // **�d�v: �����ɔ�A�N�e�B�u��**
        velocityX = 0.0f;
        velocityY = 0.0f;

        // **�f�o�b�O�o��**
        OutputDebugStringA("EnemyBase: Enemy killed by TakeDamage!\n");
    }
    else {
        // **�܂������Ă���ꍇ�̓_���[�W���**
        currentState = DAMAGED;
        stateTimer = 0.0f;
    }
}

// ���[�e�B���e�B�֐�

bool EnemyBase::CheckGroundCollision(float checkX, float checkY, StageManager* stageManager)
{
    float footY = checkY + COLLISION_HEIGHT / 2;
    return stageManager->CheckCollision(checkX - 16, footY, 32, 8);
}

bool EnemyBase::CheckWallCollision(float checkX, float checkY, StageManager* stageManager)
{
    return stageManager->CheckCollision(checkX - COLLISION_WIDTH / 2, checkY - COLLISION_HEIGHT / 2,
        COLLISION_WIDTH, COLLISION_HEIGHT);
}

float EnemyBase::FindGroundY(float checkX, StageManager* stageManager)
{
    const int TILE_SIZE = 64;
    const int MAX_SEARCH_TILES = 10;

    int startTileY = (int)(y / TILE_SIZE);
    int endTileY = startTileY + MAX_SEARCH_TILES;

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * TILE_SIZE;

        if (stageManager->CheckCollision(checkX - 16, checkY, 32, 8)) {
            return checkY;
        }
    }

    return -1;
}

float EnemyBase::GetDistanceToPlayer(Player* player)
{
    if (!player) return 999999.0f;

    float dx = player->GetX() - x;
    float dy = player->GetY() - y;
    return sqrtf(dx * dx + dy * dy);
}

void EnemyBase::FacePlayer(Player* player)
{
    if (!player) return;

    facingRight = (player->GetX() > x);
}

bool EnemyBase::IsPlayerInRange(Player* player, float range)
{
    return GetDistanceToPlayer(player) <= range;
}

float EnemyBase::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void EnemyBase::SetPosition(float newX, float newY)
{
    x = newX;
    y = newY;
}

int EnemyBase::GetCurrentSprite()
{
    // ��{�����i�h���N���X�ŃI�[�o�[���C�h�����j
    switch (currentState) {
    case IDLE: return sprites.idle;
    case WALKING: return animationFrame ? sprites.walk_b : sprites.walk_a;
    case ATTACKING: return sprites.attack;
    case DAMAGED: return sprites.damaged;
    case DYING:
    case DEAD: return sprites.dead;
    default: return sprites.idle;
    }
}