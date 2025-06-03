#include "Player.h"
#include <math.h>
#include <algorithm>
#include <cmath>

Player::Player()
    : x(300.0f)
    , y(200.0f)
    , velocityX(0.0f)
    , velocityY(0.0f)
    , facingRight(true)
    , currentState(FALLING)
    , onGround(false)
    , animationTimer(0.0f)
    , walkAnimFrame(false)
    , bobPhase(0.0f)
    , characterIndex(0)
{
    // �X�v���C�g�n���h����������
    sprites.front = sprites.idle = sprites.walk_a = sprites.walk_b = -1;
    sprites.jump = sprites.duck = sprites.hit = sprites.climb_a = sprites.climb_b = -1;
}

Player::~Player()
{
    // �X�v���C�g�̉��
    if (sprites.front != -1) DeleteGraph(sprites.front);
    if (sprites.idle != -1) DeleteGraph(sprites.idle);
    if (sprites.walk_a != -1) DeleteGraph(sprites.walk_a);
    if (sprites.walk_b != -1) DeleteGraph(sprites.walk_b);
    if (sprites.jump != -1) DeleteGraph(sprites.jump);
    if (sprites.duck != -1) DeleteGraph(sprites.duck);
    if (sprites.hit != -1) DeleteGraph(sprites.hit);
    if (sprites.climb_a != -1) DeleteGraph(sprites.climb_a);
    if (sprites.climb_b != -1) DeleteGraph(sprites.climb_b);
}

void Player::Initialize(int characterIndex)
{
    this->characterIndex = characterIndex;
    characterColorName = GetCharacterColorName(characterIndex);

    // �L�����N�^�[�X�v���C�g�ǂݍ���
    LoadCharacterSprites(characterIndex);

    // �����ʒu�ݒ�
    ResetPosition();
}

void Player::Update(StageManager* stageManager)
{
    // �������Z�X�V
    UpdatePhysics(stageManager);

    // �����蔻�菈��
    HandleCollisions(stageManager);

    // �A�j���[�V�����X�V
    UpdateAnimation();
}

void Player::Draw(float cameraX)
{
    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // �A�C�h�����̏㉺�h�����
    float bobOffset = (currentState == IDLE) ? sinf(bobPhase) * 2.0f : 0.0f;

    // ��ʍ��W�v�Z
    int screenX = (int)(x - cameraX);
    int screenY = (int)(y + bobOffset);

    // �L�����N�^�[�T�C�Y�擾
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    // ���������ŕ`��ʒu����
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // ���E���]�`��
    if (facingRight) {
        DrawGraph(screenX, screenY, currentSprite, TRUE);
    }
    else {
        DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
    }
}

void Player::UpdatePhysics(StageManager* stageManager)
{
    // **���ǂ��ꂽ�����ړ������i�C�[�W���O�t���j**
    bool leftPressed = CheckHitKey(KEY_INPUT_LEFT) && currentState != DUCKING;
    bool rightPressed = CheckHitKey(KEY_INPUT_RIGHT) && currentState != DUCKING;

    if (leftPressed) {
        // �������ւ̉���
        velocityX -= ACCELERATION;
        if (velocityX < -MAX_HORIZONTAL_SPEED) {
            velocityX = -MAX_HORIZONTAL_SPEED;
        }
        facingRight = false;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else if (rightPressed) {
        // �E�����ւ̉���
        velocityX += ACCELERATION;
        if (velocityX > MAX_HORIZONTAL_SPEED) {
            velocityX = MAX_HORIZONTAL_SPEED;
        }
        facingRight = true;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else {
        // �L�[���͂��Ȃ��ꍇ�̎��R�Ȍ����i���C�j
        velocityX *= FRICTION;

        // �ɒx�̑��x��0�ɂ���i���S��~�j
        if (fabsf(velocityX) < 0.1f) {
            velocityX = 0.0f;
        }

        // �n��ł̏�Ԍ���
        if (onGround && currentState != JUMPING && currentState != FALLING) {
            if (fabsf(velocityX) < 0.5f) {
                currentState = IDLE;
            }
            else {
                currentState = WALKING; // �܂������Ă���
            }
        }
    }

    // ���Ⴊ�ݏ���
    if (CheckHitKey(KEY_INPUT_DOWN) && onGround) {
        currentState = DUCKING;
        velocityX *= 0.8f; // ���Ⴊ�ݎ��͂�苭������
    }

    // �W�����v�����i�ӂ���Ƃ��������ɒ����j
    static bool spaceWasPressed = false;
    bool spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;

    if (spacePressed && !spaceWasPressed && onGround) {
        velocityY = JUMP_POWER;
        currentState = JUMPING;
        onGround = false;
    }
    spaceWasPressed = spacePressed;

    // �d�͓K�p�i�ӂ���Ƃ��������ɒ����j
    if (!onGround) {
        // �W�����v�{�^���𗣂����Ƃ��̑��������i��莩�R�ȑ��슴�j
        if (currentState == JUMPING && !spacePressed && velocityY < 0) {
            velocityY *= 0.7f; // �㏸�𑁂߂Ɏ~�߂�
        }

        velocityY += GRAVITY;

        // ��C��R�ɂ��ӂ���Ɗ��̉��o
        if (velocityY < 0) { // �㏸��
            velocityY *= AIR_RESISTANCE;
        }

        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }

        // �W�����v���痎���ւ̏�ԕω�
        if (velocityY > 0 && currentState == JUMPING) {
            currentState = FALLING;
        }
    }
}

void Player::HandleCollisions(StageManager* stageManager)
{
    // �v���C���[�̎��ۂ̏Փ˔���T�C�Y
    // �^�C���T�C�Y(64px)�ɑ΂��ēK�؂ȃT�C�Y�ɐݒ�
    const float COLLISION_WIDTH = 48.0f;   // �^�C���T�C�Y��3/4
    const float COLLISION_HEIGHT = 60.0f;  // �^�C���T�C�Y��菭��������

    // ===== X�����̈ړ��ƏՓ˔��� =====
    float newX = x + velocityX;

    // �X�e�[�W���E�`�F�b�N
    if (newX - COLLISION_WIDTH / 2 < 0) {
        x = COLLISION_WIDTH / 2;
        velocityX = 0.0f;
    }
    else if (newX + COLLISION_WIDTH / 2 > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - COLLISION_WIDTH / 2;
        velocityX = 0.0f;
    }
    else {
        // X�����̏ڍׂȏՓ˃`�F�b�N
        if (CheckXCollision(newX, y, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager)) {
            // �ǂɂԂ������ꍇ�A�s�N�Z���P�ʂŒ���
            velocityX = 0.0f;
            newX = AdjustXPosition(x, velocityX > 0, COLLISION_WIDTH, stageManager);
        }
        x = newX;
    }

    // ===== Y�����̈ړ��ƏՓ˔��� =====
    float newY = y + velocityY;

    if (velocityY > 0) {
        // �������ړ��i�����j
        HandleDownwardMovement(newY, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager);
    }
    else if (velocityY < 0) {
        // ������ړ��i�W�����v�j
        HandleUpwardMovement(newY, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager);
    }
    else {
        // Y�����̑��x��0�̏ꍇ�A�n�ʃ`�F�b�N
        if (onGround && !IsOnGround(x, y, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager)) {
            onGround = false;
            currentState = FALLING;
        }
        y = newY;
    }

    // ��ʊO�����̋~��
    if (y > 1400) {
        ResetPosition();
    }
}

bool Player::CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager)
{
    // �v���C���[�̏㕔�A�����A������3�_�Ń`�F�b�N
    float checkPoints[] = {
        currentY - height / 2 + 8,  // �㕔�i�������j
        currentY,                   // ����
        currentY + height / 2 - 8   // �����i������j
    };

    for (float checkY : checkPoints) {
        if (CheckPointCollision(newX, checkY, width, 4.0f, stageManager)) {
            return true;
        }
    }
    return false;
}

void Player::HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // �����̕����_�Ń`�F�b�N
    float footY = newY + height / 2;
    float leftFoot = x - width / 3;
    float rightFoot = x + width / 3;
    float centerFoot = x;

    // 3�_�Œn�ʃ`�F�b�N
    bool leftHit = CheckPointCollision(leftFoot, footY, 4.0f, 4.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 4.0f, 4.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 4.0f, 4.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // �n�ʂɒ��n
        float groundY = FindPreciseGroundY(x, newY, width, stageManager);
        if (groundY != -1) {
            y = groundY - height / 2;
            velocityY = 0.0f;
            onGround = true;

            // ���n��̏�Ԍ���
            if (CheckHitKey(KEY_INPUT_DOWN)) {
                currentState = DUCKING;
            }
            else if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                currentState = WALKING;
            }
            else {
                currentState = IDLE;
            }
        }
    }
    else {
        // ���R����
        y = newY;
        if (onGround) {
            onGround = false;
            if (currentState != JUMPING) {
                currentState = FALLING;
            }
        }
    }
}

void Player::HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // ����̕����_�Ń`�F�b�N
    float headY = newY - height / 2;
    float leftHead = x - width / 3;
    float rightHead = x + width / 3;
    float centerHead = x;

    // 3�_�œV��`�F�b�N
    bool leftHit = CheckPointCollision(leftHead, headY, 4.0f, 4.0f, stageManager);
    bool centerHit = CheckPointCollision(centerHead, headY, 4.0f, 4.0f, stageManager);
    bool rightHit = CheckPointCollision(rightHead, headY, 4.0f, 4.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // �V��ɂԂ�����
        float ceilingY = FindPreciseCeilingY(x, newY, width, stageManager);
        if (ceilingY != -1) {
            y = ceilingY + height / 2;
            velocityY = 0.0f;  // ������̑��x�����S�ɒ�~
            currentState = FALLING;
            onGround = false;
        }
    }
    else {
        // ���R�ړ�
        y = newY;
    }
}

bool Player::CheckPointCollision(float centerX, float centerY, float width, float height, StageManager* stageManager)
{
    float left = centerX - width / 2;
    float top = centerY - height / 2;
    return stageManager->CheckCollision(left, top, width, height);
}

float Player::AdjustXPosition(float currentX, bool movingRight, float width, StageManager* stageManager)
{
    // �s�N�Z���P�ʂŒ������āA�ǂɂ߂荞�܂Ȃ��ʒu��������
    float adjustedX = currentX;
    float step = movingRight ? -1.0f : 1.0f;

    for (int i = 0; i < 32; i++) { // �ő�32�s�N�Z������
        adjustedX += step;
        if (!CheckXCollision(adjustedX, y, width, 60.0f, stageManager)) {
            return adjustedX;
        }
    }

    return currentX; // �����ł��Ȃ��ꍇ�͌��̈ʒu
}

float Player::FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // �^�C���T�C�Y���l���������m�Ȓn�ʈʒu�̌���
    int tileSize = Stage::TILE_SIZE; // 64px

    // �v���C���[�̑������ӂ̃^�C���𒲂ׂ�
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY + 3; // ��������3�^�C�����`�F�b�N

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * tileSize;

        // �v���C���[�̕��Œn�ʂ��`�F�b�N
        float leftX = playerX - width / 2 + 4;
        float rightX = playerX + width / 2 - 4;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY, 4, 4) ||
            stageManager->CheckCollision(centerX, checkY, 4, 4) ||
            stageManager->CheckCollision(rightX, checkY, 4, 4)) {

            // ���̃^�C���̏�[���n��
            return checkY;
        }
    }

    return -1; // �n�ʂ�������Ȃ�
}

float Player::FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // �^�C���T�C�Y���l���������m�ȓV��ʒu�̌���
    int tileSize = Stage::TILE_SIZE; // 64px

    // �v���C���[�̓�����ӂ̃^�C���𒲂ׂ�
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY - 3; // �������3�^�C�����`�F�b�N

    for (int tileY = startTileY; tileY >= endTileY && tileY >= 0; tileY--) {
        float checkY = (tileY + 1) * tileSize; // �^�C���̉��[

        // �v���C���[�̕��œV����`�F�b�N
        float leftX = playerX - width / 2 + 4;
        float rightX = playerX + width / 2 - 4;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY - 4, 4, 4) ||
            stageManager->CheckCollision(centerX, checkY - 4, 4, 4) ||
            stageManager->CheckCollision(rightX, checkY - 4, 4, 4)) {

            // ���̃^�C���̉��[���V��
            return checkY;
        }
    }

    return -1; // �V�䂪������Ȃ�
}

bool Player::IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager)
{
    // �n�ʂƂ̐ڐG����
    float footY = playerY + height / 2 + 2; // ������菭����
    float leftFoot = playerX - width / 3;
    float rightFoot = playerX + width / 3;
    float centerFoot = playerX;

    return CheckPointCollision(leftFoot, footY, 4.0f, 4.0f, stageManager) ||
        CheckPointCollision(centerFoot, footY, 4.0f, 4.0f, stageManager) ||
        CheckPointCollision(rightFoot, footY, 4.0f, 4.0f, stageManager);
}

// ===== �����̊֐��i�݊����̂��ߎc���j =====

bool Player::CheckCollision(float checkX, float checkY, StageManager* stageManager)
{
    float collisionWidth = 48.0f;
    float collisionHeight = 60.0f;
    float left = checkX - collisionWidth / 2;
    float top = checkY - collisionHeight / 2;
    return stageManager->CheckCollision(left, top, collisionWidth, collisionHeight);
}

float Player::GetGroundY(float checkX, StageManager* stageManager)
{
    float footWidth = 48.0f;
    return stageManager->GetGroundY(checkX - footWidth / 2, footWidth);
}

bool Player::CheckCollisionRect(float checkX, float checkY, float width, float height, StageManager* stageManager)
{
    float left = checkX - width / 2;
    float top = checkY - height / 2;
    return stageManager->CheckCollision(left, top, width, height);
}

int Player::FindGroundTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager)
{
    float groundY = FindPreciseGroundY(playerX, playerY, playerWidth, stageManager);
    return (groundY != -1) ? (int)groundY : -1;
}

int Player::FindCeilingTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager)
{
    float ceilingY = FindPreciseCeilingY(playerX, playerY, playerWidth, stageManager);
    return (ceilingY != -1) ? (int)ceilingY : -1;
}

// ===== ���̑��̊����֐� =====

void Player::UpdateAnimation()
{
    if (currentState == WALKING) {
        animationTimer += WALK_ANIM_SPEED;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            walkAnimFrame = !walkAnimFrame;
        }
    }
    else {
        animationTimer = 0.0f;
        walkAnimFrame = false;
    }

    if (currentState == IDLE) {
        bobPhase += 0.02f;
        if (bobPhase >= 2.0f * 3.14159265359f) {
            bobPhase = 0.0f;
        }
    }
}

int Player::GetCurrentSprite()
{
    switch (currentState) {
    case IDLE: return sprites.idle;
    case WALKING: return walkAnimFrame ? sprites.walk_b : sprites.walk_a;
    case JUMPING: return sprites.jump;
    case FALLING: return sprites.jump;
    case DUCKING: return sprites.duck;
    default: return sprites.idle;
    }
}

void Player::LoadCharacterSprites(int characterIndex)
{
    std::string colorName = GetCharacterColorName(characterIndex);
    std::string basePath = "Sprites/Characters/character_" + colorName + "_";

    sprites.front = LoadGraph((basePath + "front.png").c_str());
    sprites.idle = LoadGraph((basePath + "idle.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.jump = LoadGraph((basePath + "jump.png").c_str());
    sprites.duck = LoadGraph((basePath + "duck.png").c_str());
    sprites.hit = LoadGraph((basePath + "hit.png").c_str());
    sprites.climb_a = LoadGraph((basePath + "climb_a.png").c_str());
    sprites.climb_b = LoadGraph((basePath + "climb_b.png").c_str());
}

std::string Player::GetCharacterColorName(int index)
{
    switch (index) {
    case 0: return "beige";
    case 1: return "green";
    case 2: return "pink";
    case 3: return "purple";
    case 4: return "yellow";
    default: return "beige";
    }
}

void Player::DrawShadow(float cameraX, StageManager* stageManager)
{
    int shadowX = (int)(x - cameraX);
    int shadowY = (int)(y + PLAYER_HEIGHT / 2 + 5);

    int shadowRadiusX = 20;
    int shadowRadiusY = 8;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawOval(shadowX, shadowY, shadowRadiusX, shadowRadiusY, GetColor(30, 30, 30), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void Player::SetPosition(float newX, float newY)
{
    x = newX;
    y = newY;
}

void Player::ResetPosition()
{
    x = 300.0f;
    y = 200.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    currentState = FALLING;
    onGround = false;
    facingRight = true;
}

void Player::DrawDebugInfo(float cameraX)
{
    const float COLLISION_WIDTH = 48.0f;
    const float COLLISION_HEIGHT = 60.0f;

    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // �Փ˔���{�b�N�X�\��
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(
        screenX - COLLISION_WIDTH / 2,
        screenY - COLLISION_HEIGHT / 2,
        screenX + COLLISION_WIDTH / 2,
        screenY + COLLISION_HEIGHT / 2,
        GetColor(255, 0, 0), FALSE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �^�C���O���b�h�\��
    int tileSize = Stage::TILE_SIZE;
    int startTileX = (int)((x - cameraX - 200) / tileSize) * tileSize;
    int endTileX = (int)((x - cameraX + 200) / tileSize) * tileSize;
    int startTileY = (int)((y - 200) / tileSize) * tileSize;
    int endTileY = (int)((y + 200) / tileSize) * tileSize;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    for (int tx = startTileX; tx <= endTileX; tx += tileSize) {
        for (int ty = startTileY; ty <= endTileY; ty += tileSize) {
            int screenTileX = tx - (int)cameraX;
            DrawBox(screenTileX, ty, screenTileX + tileSize, ty + tileSize, GetColor(0, 255, 0), FALSE);
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **�ڍ׏��\���i�C�[�W���O���܂ށj**
    std::string debugText =
        "Pos: (" + std::to_string((int)x) + "," + std::to_string((int)y) + ") " +
        "Vel: (" + std::to_string(velocityX) + "," + std::to_string(velocityY) + ") " +
        "Ground: " + (onGround ? "YES" : "NO");
    DrawString(screenX - 100, screenY - 140, debugText.c_str(), GetColor(255, 255, 0));

    // **���x�̎��o���i���ŕ\���j**
    if (fabsf(velocityX) > 0.1f || fabsf(velocityY) > 0.1f) {
        int arrowEndX = screenX + (int)(velocityX * 5);
        int arrowEndY = screenY + (int)(velocityY * 5);
        DrawLine(screenX, screenY, arrowEndX, arrowEndY, GetColor(255, 0, 255));
        DrawCircle(arrowEndX, arrowEndY, 3, GetColor(255, 0, 255), TRUE);
    }
}