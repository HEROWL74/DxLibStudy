#pragma once
#include <memory>
#include "TitleGenerate.h"
#include "Player.h"
class GameScene {
public:
    GameScene();
    void Update();
    void Draw();
private:
    std::unique_ptr<TitleGenerate> m_generator;
    std::unique_ptr<Player> m_player;
    bool m_fadeIn;
    int m_fadeAlpha;
};
