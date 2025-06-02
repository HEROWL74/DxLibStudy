#pragma once
#include "Stage.h"

class GrassStage : public Stage {
public:
    GrassStage();
    ~GrassStage() override;

    void Initialize() override;

private:
    void CreateMainTerrain();
    void CreatePlatforms();
};