#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>
#include <string>

// 前方宣言のみ
class Stage1Generate;
class Player;
class HUD;
class Block;
class Coin;
class CoinEffect;
class EnemyBase;
class Camera;

struct Rect;

class GameScene {
public:
    GameScene();
    ~GameScene(); // デストラクタを明示的に宣言

    void Update();
    void Draw();

private:
    // 衝突判定ヘルパー
    bool IsHitBottom(const Rect& r, int px, int py, int pw, int ph, float vy);
    void RestartGame();

    // ゲームオブジェクト
    std::unique_ptr<Stage1Generate> m_stage1;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<HUD> m_hud;
    std::unique_ptr<Camera> m_camera;

    std::vector<std::unique_ptr<Block>> m_blocks;
    std::vector<std::unique_ptr<Coin>> m_coins;
    std::vector<std::unique_ptr<CoinEffect>> m_coinEffects;
    std::vector<std::unique_ptr<EnemyBase>> m_enemies;

    // エフェクト・UI
    bool m_fadeIn;
    int m_fadeAlpha;

    // ゲームオーバー
    bool m_isGameOver;
    int m_gameOverTimer;
    int m_gameOverIndex;
    std::string m_gameOverText = "GAME OVER";

    // サウンド・フォント
    int m_coinSound;
    int m_fontHandle;
};