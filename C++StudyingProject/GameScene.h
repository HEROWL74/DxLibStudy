#pragma once
#include <memory>
#include "TitleGenerate.h"

class GameScene {
public:
    GameScene();
    void Update();
    void Draw();
private:
    std::unique_ptr<TitleGenerate> m_generator;
};
