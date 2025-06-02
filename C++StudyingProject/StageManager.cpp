#include "StageManager.h"

StageManager::StageManager() : currentStageType(GRASS) {
    // �f�t�H���g��GrassStage��ǂݍ���
    LoadStage(GRASS);
}

StageManager::~StageManager() {
    // �X�}�[�g�|�C���^�������I�Ƀ��������
}

void StageManager::LoadStage(StageType type) {
    currentStageType = type;

    // �Â��X�e�[�W���폜�i�X�}�[�g�|�C���^�������I�ɏ����j
    currentStage.reset();

    // �V�����X�e�[�W���쐬
    switch (type) {
    case GRASS:
        currentStage = std::make_unique<GrassStage>();
        break;
    case STONE:
        currentStage = std::make_unique<StoneStage>();
        break;
    case SAND:
        currentStage = std::make_unique<SandStage>();
        break;
    case SNOW:
        currentStage = std::make_unique<SnowStage>();
        break;
    case PURPLE:
        currentStage = std::make_unique<PurpleStage>();
        break;
    default:
        currentStage = std::make_unique<GrassStage>();
        break;
    }

    // �X�e�[�W��������
    if (currentStage) {
        currentStage->Initialize();
    }
}

void StageManager::Update(float cameraX) {
    if (currentStage) {
        currentStage->Update(cameraX);
    }
}

void StageManager::Draw(float cameraX) {
    if (currentStage) {
        currentStage->Draw(cameraX);
    }
}

bool StageManager::CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight) {
    if (currentStage) {
        return currentStage->CheckCollision(playerX, playerY, playerWidth, playerHeight);
    }
    return false;
}

float StageManager::GetGroundY(float playerX, float playerWidth) {
    if (currentStage) {
        return currentStage->GetGroundY(playerX, playerWidth);
    }
    return 800.0f; // �f�t�H���g�n��
}