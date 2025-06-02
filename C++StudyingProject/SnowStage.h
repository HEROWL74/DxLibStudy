#pragma once
#include "Stage.h"

class SnowStage : public Stage {
public:
    SnowStage();
    ~SnowStage() override;

    void Initialize() override;

private:
    void CreateMainTerrain();
    void CreatePlatforms();
};