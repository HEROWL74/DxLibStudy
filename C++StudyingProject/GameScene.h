#pragma once
#include "DxLib.h"
#include <memory>
#include <vector>
#include <string>

// �O���錾�̂�
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
    ~GameScene(); // �f�X�g���N�^�𖾎��I�ɐ錾

    void Update();
    void Draw();

private:
    // �Փ˔���w���p�[
    bool IsHitBottom(const Rect& r, int px, int py, int pw, int ph, float vy);
    void RestartGame();

    // �Q�[���I�u�W�F�N�g
    std::unique_ptr<Stage1Generate> m_stage1;
    std::unique_ptr<Player> m_player;
    std::unique_ptr<HUD> m_hud;
    std::unique_ptr<Camera> m_camera;

    std::vector<std::unique_ptr<Block>> m_blocks;
    std::vector<std::unique_ptr<Coin>> m_coins;
    std::vector<std::unique_ptr<CoinEffect>> m_coinEffects;
    std::vector<std::unique_ptr<EnemyBase>> m_enemies;

    // �G�t�F�N�g�EUI
    bool m_fadeIn;
    int m_fadeAlpha;

    // �Q�[���I�[�o�[
    bool m_isGameOver;
    int m_gameOverTimer;
    int m_gameOverIndex;
    std::string m_gameOverText = "GAME OVER";

    // �T�E���h�E�t�H���g
    int m_coinSound;
    int m_fontHandle;
};