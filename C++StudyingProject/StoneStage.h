#pragma once
#include "Stage.h"

class StoneStage : public Stage {
public:
    StoneStage();
    ~StoneStage() override;

    void Initialize() override;

private:
    void CreateMainTerrain();
    void CreatePlatforms();
};