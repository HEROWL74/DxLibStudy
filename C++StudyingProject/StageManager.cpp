#include "StageManager.h"

StageManager::StageManager() : currentStageType(GRASS) {
    // デフォルトでGrassStageを読み込み
    LoadStage(GRASS);
}

StageManager::~StageManager() {
    // スマートポインタが自動的にメモリ解放
}

void StageManager::LoadStage(StageType type) {
    currentStageType = type;

    // 古いステージを削除（スマートポインタが自動的に処理）
    currentStage.reset();

    // 新しいステージを作成
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

    // ステージを初期化
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
    return 800.0f; // デフォルト地面
}