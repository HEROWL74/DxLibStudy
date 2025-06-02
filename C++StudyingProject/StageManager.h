#pragma once
#include "Stage.h"
#include "GrassStage.h"
#include "StoneStage.h"
#include "SandStage.h"
#include "SnowStage.h"
#include "PurpleStage.h"
#include <memory>

class StageManager {
public:
    enum StageType {
        GRASS = 0,
        STONE = 1,
        SAND = 2,
        SNOW = 3,
        PURPLE = 4
    };

    StageManager();
    ~StageManager();

    void LoadStage(StageType type);
    void Update(float cameraX);
    void Draw(float cameraX);

    // 当たり判定
    bool CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight);
    float GetGroundY(float playerX, float playerWidth);

    // 現在のステージ取得
    Stage* GetCurrentStage() const { return currentStage.get(); }

private:
    std::unique_ptr<Stage> currentStage;
    StageType currentStageType;
};