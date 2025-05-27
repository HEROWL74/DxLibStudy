#pragma once
#include <memory>
#include <vector>
#include "TitleGenerate.h"
#include "Player.h"
#include "EnemyBase.h"
#include "Block.h"
#include "HUD.h"
#include "Coin.h"
#include "CoinEffect.h"


class GameScene {
public:
    GameScene();
    void Update();
    void Draw();

private:
    std::unique_ptr<TitleGenerate> m_generator;
    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<EnemyBase>> m_enemies;
    std::vector < std::unique_ptr<Block>> m_blocks;
    std::unique_ptr<HUD> m_hud;
    std::vector<std::unique_ptr<Coin>> m_coins;
    std::vector<std::unique_ptr<CoinEffect>> m_coinEffects;

    bool m_fadeIn;
    int m_fadeAlpha;
};
