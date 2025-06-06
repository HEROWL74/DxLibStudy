#include "DoorSystem.h"
#include <math.h>
#include <algorithm>

DoorSystem::DoorSystem()
    : doorX(0), doorY(0)
    , doorState(DOOR_CLOSED)
    , doorExists(false)
    , openingProgress(0.0f)
    , enteringProgress(0.0f)
    , animationTimer(0.0f)
    , playerStartX(0.0f)
    , playerTargetX(0.0f)
    , playerWalkSpeed(PLAYER_WALK_SPEED)
{
    // �e�N�X�`���n���h����������
    doorTextures.closedBottom = doorTextures.closedTop = -1;
    doorTextures.openBottom = doorTextures.openTop = -1;
}

DoorSystem::~DoorSystem()
{
    // �e�N�X�`���̉��
    if (doorTextures.closedBottom != -1) DeleteGraph(doorTextures.closedBottom);
    if (doorTextures.closedTop != -1) DeleteGraph(doorTextures.closedTop);
    if (doorTextures.openBottom != -1) DeleteGraph(doorTextures.openBottom);
    if (doorTextures.openTop != -1) DeleteGraph(doorTextures.openTop);
}

void DoorSystem::Initialize()
{
    LoadTextures();
    ClearDoor();
}

void DoorSystem::LoadTextures()
{
    // �h�A�e�N�X�`����ǂݍ���
    doorTextures.closedBottom = LoadGraph("Sprites/Tiles/door_closed.png");
    doorTextures.closedTop = LoadGraph("Sprites/Tiles/door_closed_top.png");
    doorTextures.openBottom = LoadGraph("Sprites/Tiles/door_open.png");
    doorTextures.openTop = LoadGraph("Sprites/Tiles/door_open_top.png");
}

void DoorSystem::Update(Player* player)
{
    if (!doorExists || !player) return;

    // �A�j���[�V�����X�V
    UpdateDoorAnimation();

    // �v���C���[�i������
    if (doorState == DOOR_OPEN) {
        // �h�A���J���Ă��ăv���C���[���߂��ɂ���ꍇ
        if (CheckPlayerNearDoor(player)) {
            StartPlayerEntering(player);
        }
    }
    else if (doorState == DOOR_PLAYER_ENTERING) {
        UpdatePlayerEntering(player);
    }
}

void DoorSystem::UpdateDoorAnimation()
{
    animationTimer += 0.016f; // 60FPS�z��

    switch (doorState) {
    case DOOR_OPENING:
        openingProgress += 0.016f / OPENING_DURATION;
        if (openingProgress >= 1.0f) {
            openingProgress = 1.0f;
            doorState = DOOR_OPEN;
        }
        break;

    case DOOR_PLAYER_ENTERING:
        enteringProgress += 0.016f / ENTERING_DURATION;
        if (enteringProgress >= 1.0f) {
            enteringProgress = 1.0f;
        }
        break;

    default:
        break;
    }
}
void DoorSystem::UpdatePlayerEntering(Player* player)
{
    if (!player) return;

    // **�������s���L�����m�F**
    if (!player->IsAutoWalking()) {
        // �������s�������ɂȂ��Ă���ꍇ�͍ėL����
        player->SetAutoWalking(true);
        OutputDebugStringA("DoorSystem: Re-enabled auto walking\n");
    }

    // **�h�A�܂ł̋������`�F�b�N**
    float currentX = player->GetX();
    float targetX = doorX + DOOR_WIDTH / 2; // �h�A�̒���
    float distance = abs(currentX - targetX);

    // **�f�o�b�O�o��**
    char debugMsg[256];
    sprintf_s(debugMsg, "DoorSystem: Player X:%.1f, Target X:%.1f, Distance:%.1f\n",
        currentX, targetX, distance);
    OutputDebugStringA(debugMsg);

    // **�h�A�ɏ\���߂Â����ꍇ**
    if (distance <= 30.0f) { // ���苗����30�s�N�Z���Ɋg��
        // **�������s���~**
        player->SetAutoWalking(false);

        // **�v���C���[���h�A�̒����ɔz�u�iY���W�͕ύX���Ȃ��j**
        player->SetPosition(targetX, player->GetY());

        // �h�A�i������
        enteringProgress = 1.0f;

        OutputDebugStringA("DoorSystem: Player reached door center!\n");
    }
}


bool DoorSystem::CheckPlayerNearDoor(Player* player)
{
    if (!player) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();

    // **�h�A���o�͈͂��g��**
    float doorCenterX = doorX + DOOR_WIDTH / 2;
    float doorCenterY = doorY + DOOR_HEIGHT / 2;

    // **X������Y�������ʂɃ`�F�b�N**
    float distanceX = abs(playerX - doorCenterX);
    float distanceY = abs(playerY - doorCenterY);

    // **�h�A���o�������ɘa**
    bool nearX = distanceX <= DETECTION_WIDTH * 2.0f; // X�����̌��o�͈͂��g��
    bool nearY = distanceY <= DOOR_HEIGHT * 1.5f;     // Y�����̌��o�͈�

    bool isNear = nearX && nearY;

    // **�f�o�b�O�o��**
    if (isNear) {
        char debugMsg[256];
        sprintf_s(debugMsg, "DoorSystem: Player near door! X:%.1f, Y:%.1f, DoorX:%.1f, DoorY:%.1f\n",
            playerX, playerY, doorCenterX, doorCenterY);
        OutputDebugStringA(debugMsg);
    }

    return isNear;
}

void DoorSystem::StartPlayerEntering(Player* player)
{
    if (!player) return;

    doorState = DOOR_PLAYER_ENTERING;
    enteringProgress = 0.0f;
    playerStartX = player->GetX();
    playerTargetX = doorX + DOOR_WIDTH / 2;
    animationTimer = 0.0f;
}

void DoorSystem::Draw(float cameraX)
{
    if (!doorExists) return;

    DrawDoorSprites(cameraX);
    DrawDoorEffects(cameraX);
}

void DoorSystem::DrawDoorSprites(float cameraX)
{
    // ��ʍ��W�ɕϊ�
    int screenX = (int)(doorX - cameraX);
    int screenY = (int)doorY;

    // ��ʊO�Ȃ�`�悵�Ȃ�
    if (screenX < -DOOR_WIDTH || screenX > 1920 + DOOR_WIDTH) return;

    // �h�A�̏�Ԃɉ����ăe�N�X�`����I��
    int bottomTexture = -1;
    int topTexture = -1;

    if (doorState == DOOR_CLOSED) {
        bottomTexture = doorTextures.closedBottom;
        topTexture = doorTextures.closedTop;
    }
    else {
        // �J���Ă����ԁiDOOR_OPENING, DOOR_OPEN, DOOR_PLAYER_ENTERING�j
        bottomTexture = doorTextures.openBottom;
        topTexture = doorTextures.openTop;
    }

    // �J���A�j���[�V��������
    float scaleEffect = 1.0f;
    int alphaEffect = 255;

    if (doorState == DOOR_OPENING) {
        // �J�����̉��o
        scaleEffect = 1.0f + openingProgress * 0.1f; // �y���g�����
        alphaEffect = 255;
    }

    // �����h�A�`��
    if (bottomTexture != -1) {
        int drawWidth = (int)(DOOR_WIDTH * scaleEffect);
        int drawHeight = (int)(DOOR_WIDTH * scaleEffect); // 64x64�z��
        int offsetX = (drawWidth - DOOR_WIDTH) / 2;
        int offsetY = (drawHeight - DOOR_WIDTH) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaEffect);
        DrawExtendGraph(
            screenX - offsetX, screenY + 64 - offsetY,
            screenX - offsetX + drawWidth, screenY + 64 - offsetY + drawHeight,
            bottomTexture, TRUE
        );
    }

    // �㕔�h�A�`��
    if (topTexture != -1) {
        int drawWidth = (int)(DOOR_WIDTH * scaleEffect);
        int drawHeight = (int)(DOOR_WIDTH * scaleEffect);
        int offsetX = (drawWidth - DOOR_WIDTH) / 2;
        int offsetY = (drawHeight - DOOR_WIDTH) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaEffect);
        DrawExtendGraph(
            screenX - offsetX, screenY - offsetY,
            screenX - offsetX + drawWidth, screenY - offsetY + drawHeight,
            topTexture, TRUE
        );
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void DoorSystem::DrawDoorEffects(float cameraX)
{
    int screenX = (int)(doorX - cameraX);
    int screenY = (int)doorY;

    // �J�����̃G�t�F�N�g
    if (doorState == DOOR_OPENING && openingProgress < 1.0f) {
        // ����G�t�F�N�g
        float glowIntensity = sinf(animationTimer * 8.0f) * 0.3f + 0.7f;
        int glowAlpha = (int)(120 * glowIntensity * (1.0f - openingProgress));

        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);

        // �h�A���ӂ̌�
        for (int i = 0; i < 3; i++) {
            int offset = (i + 1) * 8;
            DrawBox(
                screenX - offset, screenY - offset,
                screenX + (int)DOOR_WIDTH + offset, screenY + (int)DOOR_HEIGHT + offset,
                GetColor(255, 255, 100), FALSE
            );
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �v���C���[�i�����̃G�t�F�N�g
    if (doorState == DOOR_PLAYER_ENTERING) {
        // ���炩�����̃G�t�F�N�g
        float enterGlow = sinf(enteringProgress * 3.14159f) * 0.8f;
        int enterAlpha = (int)(80 * enterGlow);

        SetDrawBlendMode(DX_BLENDMODE_ADD, enterAlpha);
        DrawCircle(screenX + DOOR_WIDTH / 2, screenY + DOOR_HEIGHT / 2, 60,
            GetColor(100, 200, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

#ifdef _DEBUG
    // �f�o�b�O�p�F�h�A���o�͈͂�\��
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawCircle(screenX + DOOR_WIDTH / 2, screenY + DOOR_HEIGHT / 2,
        (int)DETECTION_WIDTH, GetColor(0, 255, 255), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

void DoorSystem::SetDoor(float x, float y)
{
    doorX = x;
    doorY = y;
    doorExists = true;
    doorState = DOOR_CLOSED;
    openingProgress = 0.0f;
    enteringProgress = 0.0f;
    animationTimer = 0.0f;
}

void DoorSystem::ClearDoor()
{
    doorExists = false;
    doorState = DOOR_CLOSED;
    openingProgress = 0.0f;
    enteringProgress = 0.0f;
    animationTimer = 0.0f;
}

void DoorSystem::OpenDoor()
{
    if (!doorExists || doorState != DOOR_CLOSED) return;

    doorState = DOOR_OPENING;
    animationTimer = 0.0f;
    openingProgress = 0.0f;
}

bool DoorSystem::IsPlayerFullyEntered() const
{
    return doorState == DOOR_PLAYER_ENTERING && enteringProgress >= 1.0f;
}

// **�C��: �h�A�z�u���̒n�ʃ��x�����o�����P**
void DoorSystem::PlaceDoorForStage(int stageIndex, StageManager* stageManager)
{
    ClearDoor();

    // �X�e�[�W5�i�C���f�b�N�X4�j�ȊO�Ƀh�A��z�u
    if (stageIndex >= 4) {
        // �X�e�[�W5�ɂ͔z�u���Ȃ�
        return;
    }

    // **�X�e�[�W���Ƃ̃h�A�z�u�ʒu���ʂɐݒ�**
    float doorWorldX;

    switch (stageIndex) {
    case 0: // �X�e�[�W1�iGrass Stage�j
        // ���̑O�ɔz�u�i���̈ʒu���班���ߑO�j
        doorWorldX = Stage::STAGE_WIDTH - 500; // �����ߑO
        break;
    case 1: // �X�e�[�W2�iStone Stage�j
        doorWorldX = Stage::STAGE_WIDTH - 350;
        break;
    case 2: // �X�e�[�W3�iSand Stage�j
        doorWorldX = Stage::STAGE_WIDTH - 400;
        break;
    case 3: // �X�e�[�W4�iSnow Stage�j
        doorWorldX = Stage::STAGE_WIDTH - 380;
        break;
    default:
        doorWorldX = Stage::STAGE_WIDTH - 400;
        break;
    }

    // **���ǂ��ꂽ�n�ʌ����Ő��m�Ȓn�ʃ��x�����擾**
    float groundLevel = FindGroundLevelAccurate(doorWorldX, stageManager);

    if (groundLevel > 0) {
        // **�h�A�̒ꕔ���n�ʂƓ��������ɂȂ�悤�ɔz�u**
        // �h�A��2�^�C�����̍����i128px�j�Ȃ̂ŁA��^�C���̈ʒu���v�Z
        float doorWorldY = groundLevel - DOOR_HEIGHT; // ��^�C�����n�ʂ��128px���
        SetDoor(doorWorldX, doorWorldY);

        // **�f�o�b�O�o��**
        char debugMsg[256];
        sprintf_s(debugMsg, "DoorSystem: Placed door at Stage %d - X:%.1f, Y:%.1f, Ground:%.1f\n",
            stageIndex + 1, doorWorldX, doorWorldY, groundLevel);
        OutputDebugStringA(debugMsg);
    }
}

float DoorSystem::FindGroundLevel(float x, StageManager* stageManager)
{
    // �w�肳�ꂽ�ʒu�ł̒n�ʃ��x����T��
    // �ォ�牺�Ɍ������ă^�C�����`�F�b�N
    for (int y = 0; y < Stage::STAGE_HEIGHT; y += Stage::TILE_SIZE) {
        if (stageManager->CheckCollision(x, y, 1, 1)) {
            return (float)y; // �ŏ��Ɍ��������^�C���̈ʒu��Ԃ�
        }
    }
    return Stage::GROUND_LEVEL; // �f�t�H���g�̒n�ʃ��x��
}

// **�V�ǉ�: ��萳�m�Ȓn�ʌ����֐�**
float DoorSystem::FindGroundLevelAccurate(float x, StageManager* stageManager)
{
    const int TILE_SIZE = 64; // Stage::TILE_SIZE
    const int MAX_SEARCH_TILES = 20; // �ő�20�^�C�����܂Ō���

    // �h�A�̕��͈̔͂Œn�ʂ������i�����肵���z�u�̂��߁j
    float searchPositions[] = {
        x,                    // ����
        x + DOOR_WIDTH / 4,   // �E���
        x - DOOR_WIDTH / 4    // �����
    };

    // �e�ʒu�Œn�ʂ�����
    for (float searchX : searchPositions) {
        // �ォ�牺�Ɍ������Ēn�ʂ�����
        for (int tileY = 0; tileY < MAX_SEARCH_TILES; tileY++) {
            float checkY = tileY * TILE_SIZE;

            // ���̈ʒu�Ɍő̃u���b�N�����邩�`�F�b�N
            if (stageManager->CheckCollision(searchX, checkY, 8, 8)) {
                // �n�ʔ����F���̃^�C���̏�[��Ԃ�
                return checkY;
            }
        }
    }

    // �n�ʂ�������Ȃ��ꍇ�̓f�t�H���g�l
    return 12 * TILE_SIZE; // �^�C��12���̍����i768px�j
}

float DoorSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}