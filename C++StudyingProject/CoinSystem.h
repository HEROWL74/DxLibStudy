#pragma once
#include "DxLib.h"
#include "Player.h"
#include <vector>
#include <memory>

class CoinSystem {
public:
    // �R�C���̏��
    enum CoinState {
        COIN_IDLE,        // �ʏ���
        COIN_ATTRACTING,  // �z���񂹒�
        COIN_COLLECTED    // ���W�ς�
    };

    // �ʂ̃R�C��
    struct Coin {
        float x, y;                 // �ʒu
        float originalX, originalY; // ���̈ʒu
        float velocityX, velocityY; // �ړ����x
        CoinState state;            // ���
        float animationTimer;       // �A�j���[�V�����^�C�}�[
        float bobPhase;            // ���V���ʂ̃t�F�[�Y
        float attractTimer;        // �z���񂹃^�C�}�[
        float scale;               // �X�P�[��
        int alpha;                 // �����x
        bool active;               // �A�N�e�B�u�t���O

        Coin(float posX, float posY)
            : x(posX), y(posY), originalX(posX), originalY(posY)
            , velocityX(0), velocityY(0), state(COIN_IDLE)
            , animationTimer(0), bobPhase(0), attractTimer(0)
            , scale(1.0f), alpha(255), active(true) {}
    };

    CoinSystem();
    ~CoinSystem();

    void Initialize();
    void Update(Player* player, float hudCoinX, float hudCoinY);
    void Draw(float cameraX);

    // �R�C���Ǘ�
    void AddCoin(float x, float y);
    void ClearAllCoins();
    int GetCollectedCoinsCount() const { return collectedCoinsCount; }
    void ResetCollectedCount() { collectedCoinsCount = 0; }

    // �X�e�[�W�p�̎����z�u
    void GenerateCoinsForStage();

private:
    // �e�N�X�`��
    int coinTexture;
    int sparkleTexture; // ����߂����ʗp�i�I�v�V�����j

    // �R�C���Ǘ�
    std::vector<std::unique_ptr<Coin>> coins;
    int collectedCoinsCount;

    // �����E���o�萔
    static constexpr float COIN_SIZE = 64.0f;           // �R�C���T�C�Y�i48��64�Ɋg��j
    static constexpr float COLLECTION_DISTANCE = 80.0f; // ���W�J�n����
    static constexpr float ATTRACT_DISTANCE = 120.0f;   // �z���񂹊J�n����
    static constexpr float ATTRACT_SPEED = 4.0f;        // �z���񂹑��x�i8.0��4.0�Ɍ����j
    static constexpr float BOB_SPEED = 0.04f;           // ���V���x
    static constexpr float BOB_AMPLITUDE = 8.0f;        // ���V�U��
    static constexpr float ROTATE_SPEED = 0.02f;        // ��]���x�i�A�j���[�V�����p�j

    // �w���p�[�֐�
    void UpdateCoin(Coin& coin, float playerX, float playerY, float hudCoinX, float hudCoinY);
    void UpdateCoinPhysics(Coin& coin, float hudCoinX, float hudCoinY);
    void UpdateCoinAnimation(Coin& coin);
    void DrawCoin(const Coin& coin, float cameraX);
    void DrawCollectionEffect(const Coin& coin, float cameraX);

    float GetDistance(float x1, float y1, float x2, float y2);
    float Lerp(float a, float b, float t);
    void LoadTextures();
};