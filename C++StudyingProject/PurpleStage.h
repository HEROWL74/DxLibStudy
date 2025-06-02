#pragma once
#include "Stage.h"

class PurpleStage : public Stage {
public:
    PurpleStage();
    ~PurpleStage() override;

    void Initialize() override;

private:
    void CreateMainTerrain();
    void CreatePlatforms();
};