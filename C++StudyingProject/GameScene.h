#pragma once
#include <memory>
#include <vector>
#include "Player.h"
#include "EnemyBase.h"
#include "Block.h"
#include "HUD.h"
#include "Coin.h"
#include "CoinEffect.h"
#include "Stage1Generate.h"

class GameScene {
public:
    GameScene();
    void Update();
    void Draw();
    void RestartGame();  // リスタート機能を追加

private:
    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<EnemyBase>> m_enemies;
    std::vector<std::unique_ptr<Block>> m_blocks;
    std::unique_ptr<HUD> m_hud;
    std::vector<std::unique_ptr<Coin>> m_coins;
    std::vector<std::unique_ptr<CoinEffect>> m_coinEffects;
    std::unique_ptr<Stage1Generate> m_stage1;

    bool m_fadeIn;
    int m_fadeAlpha;
    bool IsHitBottom(const Rect& r, int px, int py, int pw, int ph, float vy);

    // ゲームオーバー表示用
    bool   m_isGameOver = false;         // ゲームオーバー開始フラグ
    int    m_gameOverTimer;              // フレームカウント
    int    m_gameOverIndex;              // 現在表示済みの文字数
    std::string m_gameOverText = "GAMEOVER";

    // フォントハンドル
    int    m_fontHandle;
    int    m_coinSound;
};