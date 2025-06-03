#include "GoalSystem.h"
#include <math.h>

GoalSystem::GoalSystem()
    : goalX(0), goalY(0)
    , currentColor(FLAG_BLUE)
    , goalState(GOAL_ACTIVE)
    , goalExists(false)
    , animationTimer(0.0f)
    , currentFrame(false)
    , goalTouchTimer(0.0f)
    , glowIntensity(0.0f)
    , bobPhase(0.0f)
{
    // �e�N�X�`���n���h����������
    flagTextures.blue_a = flagTextures.blue_b = -1;
    flagTextures.yellow_a = flagTextures.yellow_b = -1;
    flagTextures.green_a = flagTextures.green_b = -1;
    flagTextures.red_a = flagTextures.red_b = -1;
}

GoalSystem::~GoalSystem()
{
    // �e�N�X�`���̉��
    if (flagTextures.blue_a != -1) DeleteGraph(flagTextures.blue_a);
    if (flagTextures.blue_b != -1) DeleteGraph(flagTextures.blue_b);
    if (flagTextures.yellow_a != -1) DeleteGraph(flagTextures.yellow_a);
    if (flagTextures.yellow_b != -1) DeleteGraph(flagTextures.yellow_b);
    if (flagTextures.green_a != -1) DeleteGraph(flagTextures.green_a);
    if (flagTextures.green_b != -1) DeleteGraph(flagTextures.green_b);
    if (flagTextures.red_a != -1) DeleteGraph(flagTextures.red_a);
    if (flagTextures.red_b != -1) DeleteGraph(flagTextures.red_b);
}

void GoalSystem::Initialize()
{
    LoadTextures();
    ClearGoal();
}

void GoalSystem::LoadTextures()
{
    // ���̃e�N�X�`����ǂݍ���
    flagTextures.blue_a = LoadGraph("Sprites/Tiles/flag_blue_a.png");
    flagTextures.blue_b = LoadGraph("Sprites/Tiles/flag_blue_b.png");
    flagTextures.yellow_a = LoadGraph("Sprites/Tiles/flag_yellow_a.png");
    flagTextures.yellow_b = LoadGraph("Sprites/Tiles/flag_yellow_b.png");
    flagTextures.green_a = LoadGraph("Sprites/Tiles/flag_green_a.png");
    flagTextures.green_b = LoadGraph("Sprites/Tiles/flag_green_b.png");
    flagTextures.red_a = LoadGraph("Sprites/Tiles/flag_red_a.png");
    flagTextures.red_b = LoadGraph("Sprites/Tiles/flag_red_b.png");
}

void GoalSystem::Update(Player* player)
{
    if (!goalExists || !player) return;

    // �A�j���[�V�����X�V
    UpdateAnimation();

    // �G�t�F�N�g�X�V
    UpdateEffects();

    // �v���C���[�Ƃ̏Փ˔���
    if (goalState == GOAL_ACTIVE && CheckPlayerCollision(player)) {
        goalState = GOAL_TOUCHED;
        goalTouchTimer = 0.0f;
    }

    // �S�[���^�b�`��̏���
    if (goalState == GOAL_TOUCHED) {
        goalTouchTimer += 0.016f; // 60FPS�z��

        if (goalTouchTimer >= GOAL_TOUCH_DURATION) {
            goalState = GOAL_COMPLETED;
        }
    }
}

void GoalSystem::Draw(float cameraX)
{
    if (!goalExists) return;

    // �S�[���̊���`��
    DrawGoalFlag(cameraX);

    // �G�t�F�N�g��`��
    DrawGoalEffects(cameraX);
}

void GoalSystem::UpdateAnimation()
{
    // �������Ƃ������̃A�j���[�V����
    animationTimer += ANIMATION_SPEED;
    if (animationTimer >= 1.0f) {
        animationTimer = 0.0f;
        currentFrame = !currentFrame; // �t���[����؂�ւ�
    }

    // �㉺���V����
    bobPhase += BOB_SPEED;
    if (bobPhase >= 2.0f * 3.14159265f) {
        bobPhase = 0.0f;
    }
}

void GoalSystem::UpdateEffects()
{
    // �O���[���ʂ̍X�V
    switch (goalState) {
    case GOAL_ACTIVE:
        // �ʏ펞�̂������Ƃ����O���[
        glowIntensity = 0.3f + sinf(bobPhase * 2.0f) * 0.2f;
        break;

    case GOAL_TOUCHED:
    {
        // �^�b�`���̋����O���[����
        float progress = goalTouchTimer / GOAL_TOUCH_DURATION;
        glowIntensity = 0.8f + sinf(progress * 20.0f) * 0.2f; // �_�Ō���
    }
    break;

    case GOAL_COMPLETED:
        glowIntensity = 1.0f;
        break;
    }
}

bool GoalSystem::CheckPlayerCollision(Player* player)
{
    float playerX = player->GetX();
    float playerY = player->GetY();

    // �S�[���ƃv���C���[�̋������`�F�b�N
    float distance = GetDistance(playerX, playerY, goalX, goalY);

    // �S�[������͈͓����`�F�b�N
    return distance <= (GOAL_DETECTION_WIDTH / 2);
}

void GoalSystem::DrawGoalFlag(float cameraX)
{
    // ��ʍ��W�ɕϊ�
    int screenX = (int)(goalX - cameraX);
    int screenY = (int)(goalY + sinf(bobPhase) * BOB_AMPLITUDE); // ���V����

    // ��ʊO�Ȃ�`�悵�Ȃ�
    if (screenX < -FLAG_WIDTH || screenX > 1920 + FLAG_WIDTH) return;

    // ���݂̃t���[���̃e�N�X�`�����擾
    int flagTexture = GetCurrentFlagTexture();
    if (flagTexture == -1) return;

    // �S�[���^�b�`���̓������
    if (goalState == GOAL_TOUCHED) {
        float progress = goalTouchTimer / GOAL_TOUCH_DURATION;
        float scale = 1.0f + sinf(progress * 10.0f) * 0.1f; // �U������

        int scaledWidth = (int)(FLAG_WIDTH * scale);
        int scaledHeight = (int)(FLAG_HEIGHT * scale);
        int offsetX = (scaledWidth - FLAG_WIDTH) / 2;
        int offsetY = (scaledHeight - FLAG_HEIGHT) / 2;

        DrawExtendGraph(
            screenX - offsetX, screenY - offsetY,
            screenX + scaledWidth - offsetX, screenY + scaledHeight - offsetY,
            flagTexture, TRUE
        );
    }
    else {
        // �ʏ�`��
        DrawExtendGraph(
            screenX, screenY,
            screenX + (int)FLAG_WIDTH, screenY + (int)FLAG_HEIGHT,
            flagTexture, TRUE
        );
    }
}

void GoalSystem::DrawGoalEffects(float cameraX)
{
    int screenX = (int)(goalX - cameraX);
    int screenY = (int)(goalY + sinf(bobPhase) * BOB_AMPLITUDE);

    // �O���[����
    if (glowIntensity > 0.01f) {
        int glowAlpha = (int)(glowIntensity * 150);
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);

        // �����̃��C���[�ŃO���[����
        for (int i = 0; i < 3; i++) {
            int offset = (i + 1) * 8;
            DrawBox(
                screenX - offset, screenY - offset,
                screenX + (int)FLAG_WIDTH + offset, screenY + (int)FLAG_HEIGHT + offset,
                GetColor(255, 255, 150), FALSE
            );
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �f�o�b�O�p�F�S�[������͈͂�\��
#ifdef _DEBUG
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawCircle(screenX + FLAG_WIDTH / 2, screenY + FLAG_HEIGHT / 2,
        (int)(GOAL_DETECTION_WIDTH / 2), GetColor(0, 255, 255), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

int GoalSystem::GetCurrentFlagTexture()
{
    // ���݂̐F�ƃt���[���ɉ����ăe�N�X�`����Ԃ�
    switch (currentColor) {
    case FLAG_BLUE:
        return currentFrame ? flagTextures.blue_b : flagTextures.blue_a;
    case FLAG_YELLOW:
        return currentFrame ? flagTextures.yellow_b : flagTextures.yellow_a;
    case FLAG_GREEN:
        return currentFrame ? flagTextures.green_b : flagTextures.green_a;
    case FLAG_RED:
        return currentFrame ? flagTextures.red_b : flagTextures.red_a;
    default:
        return flagTextures.blue_a;
    }
}

void GoalSystem::SetGoal(float x, float y, FlagColor color)
{
    goalX = x;
    goalY = y;
    currentColor = color;
    goalExists = true;
    goalState = GOAL_ACTIVE;
    goalTouchTimer = 0.0f;
    animationTimer = 0.0f;
    bobPhase = 0.0f;
}

void GoalSystem::ClearGoal()
{
    goalExists = false;
    goalState = GOAL_ACTIVE;
    goalTouchTimer = 0.0f;
}

void GoalSystem::PlaceGoalForStage(int stageIndex, StageManager* stageManager)
{
    ClearGoal();

    // �Ō�̃X�e�[�W�i4�Ԗځj�ɂ̓S�[����z�u���Ȃ�
    if (stageIndex >= 4) {
        return;
    }

    // �X�e�[�W�ɉ������F�ƈʒu��ݒ�
    FlagColor stageColors[] = {
        FLAG_BLUE,    // GrassStage
        FLAG_YELLOW,  // StoneStage  
        FLAG_GREEN,   // SandStage
        FLAG_RED      // SnowStage
    };

    // �X�e�[�W�̏I�[�߂��ɃS�[����z�u
    float goalWorldX = Stage::STAGE_WIDTH - 400; // �X�e�[�W�̉E�[����400px��O
    float groundLevel = FindGroundLevel(goalWorldX, stageManager);

    if (groundLevel > 0) {
        // �u���b�N�̏�ɔz�u�i���̒ꂪ�u���b�N�̏�ɗ���悤�Ɂj
        float goalWorldY = groundLevel - FLAG_HEIGHT;
        SetGoal(goalWorldX, goalWorldY, stageColors[stageIndex % 4]);
    }
}

float GoalSystem::FindGroundLevel(float x, StageManager* stageManager)
{
    // �w�肳�ꂽ�ʒu�ł̒n�ʃ��x��������
    // �ォ�牺�Ɍ������ă^�C�����`�F�b�N
    for (int y = 0; y < Stage::STAGE_HEIGHT; y += Stage::TILE_SIZE) {
        if (stageManager->CheckCollision(x, y, 1, 1)) {
            return (float)y; // �ŏ��Ɍ��������^�C���̈ʒu��Ԃ�
        }
    }
    return Stage::GROUND_LEVEL; // �f�t�H���g�̒n�ʃ��x��
}

float GoalSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}