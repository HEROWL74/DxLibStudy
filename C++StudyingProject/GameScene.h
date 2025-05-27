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
    void RestartGame();  // ���X�^�[�g�@�\��ǉ�

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

    // �Q�[���I�[�o�[�\���p
    bool   m_isGameOver = false;         // �Q�[���I�[�o�[�J�n�t���O
    int    m_gameOverTimer;              // �t���[���J�E���g
    int    m_gameOverIndex;              // ���ݕ\���ς݂̕�����
    std::string m_gameOverText = "GAMEOVER";

    // �t�H���g�n���h��
    int    m_fontHandle;
    int    m_coinSound;
};