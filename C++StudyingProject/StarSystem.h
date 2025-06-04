#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <vector>
#include <memory>

class StarSystem {
public:
    // ���̏��
    enum StarState {
        STAR_IDLE,        // �ʏ���
        STAR_ATTRACTING,  // �����񂹒�
        STAR_COLLECTED    // ���W�ς�
    };

    // �ʂ̐�
    struct Star {
        float x, y;                 // �ʒu
        float originalX, originalY; // ���̈ʒu
        StarState state;            // ���
        float animationTimer;       // �A�j���[�V�����^�C�}�[
        float bobPhase;            // ���V���ʂ̃t�F�[�Y
        float attractTimer;        // �����񂹃^�C�}�[
        float scale;               // �X�P�[��
        float rotation;            // ��]�p�x
        int alpha;                 // �����x
        bool active;               // �A�N�e�B�u�t���O
        bool collected;            // ���W�ς݃t���O

        // **���W�A�j���[�V�����p�̕ϐ�**
        float collectionPhase;     // ���W�A�j���[�V�����̐i�s�x
        float burstIntensity;      // �o�[�X�g���ʂ̋��x
        int sparkleCount;          // �L���L���̐�

        Star(float posX, float posY)
            : x(posX), y(posY), originalX(posX), originalY(posY)
            , state(STAR_IDLE), animationTimer(0), bobPhase(0), attractTimer(0)
            , scale(1.0f), rotation(0.0f), alpha(255), active(true), collected(false)
            , collectionPhase(0.0f), burstIntensity(0.0f), sparkleCount(0) {}
    };

    StarSystem();
    ~StarSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // ���Ǘ�
    void AddStar(float x, float y);
    void ClearAllStars();
    int GetCollectedStarsCount() const { return collectedStarsCount; }
    void ResetCollectedCount() { collectedStarsCount = 0; }

    // �X�e�[�W�����̐��z�u���\�b�h
    void GenerateStarsForStageIndex(int stageIndex);
    void GenerateStarsForGrassStage();
    void GenerateStarsForStoneStage();
    void GenerateStarsForSandStage();
    void GenerateStarsForSnowStage();
    void GenerateStarsForPurpleStage();

private:
    // �e�N�X�`��
    int starTexture;
    int sparkleTexture;

    // ���Ǘ�
    std::vector<std::unique_ptr<Star>> stars;
    int collectedStarsCount;

    // �����E���o�萔
    static constexpr float STAR_SIZE = 64.0f;           // ���T�C�Y
    static constexpr float COLLECTION_DISTANCE = 100.0f; // ���W�J�n����
    static constexpr float ATTRACT_DISTANCE = 150.0f;   // �����񂹊J�n����
    static constexpr float BOB_SPEED = 0.03f;           // ���V���x
    static constexpr float BOB_AMPLITUDE = 12.0f;       // ���V�U��
    static constexpr float ROTATE_SPEED = 0.04f;        // ��]���x
    static constexpr float SPARKLE_DURATION = 2.0f;     // �_��+�p�[�e�B�N���p�����ԁi3��_�ŗp�j

    // �w���p�[�֐�
    void UpdateStar(Star& star, float playerX, float playerY);
    void UpdateStarPhysics(Star& star);
    void UpdateStarAnimation(Star& star);
    void DrawStar(const Star& star, float cameraX);
    void DrawCollectionEffect(const Star& star, float cameraX);

    // **���W�G�t�F�N�g�֐�**
    void DrawCollectionBurst(const Star& star, int screenX, int screenY, float cameraX);

    float GetDistance(float x1, float y1, float x2, float y2);
    void LoadTextures();
};