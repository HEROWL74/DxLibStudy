#pragma once
#include "Stage.h"

class SandStage : public Stage {
public:
    SandStage();
    ~SandStage() override;

    void Initialize() override;

private:
    void CreateMainTerrain();
    void CreatePlatforms();
};