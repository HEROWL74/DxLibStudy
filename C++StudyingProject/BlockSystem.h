#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

// **�O���錾**
class Player;

class BlockSystem {
public:
    // �u���b�N�̎��
    enum BlockType {
        COIN_BLOCK,     // �R�C���u���b�N
        BRICK_BLOCK     // �����K�u���b�N
    };

    // �u���b�N�̏��
    enum BlockState {
        ACTIVE,         // �A�N�e�B�u���
        EMPTY,          // ���ԁi�R�C���u���b�N�p�j
        DESTROYED       // �j�󂳂ꂽ��ԁi�����K�u���b�N�p�j
    };

    // �j�Ђ̍\����
    struct BlockFragment {
        float x, y;                 // �ʒu
        float velocityX, velocityY; // ���x
        float rotation;             // ��]�p�x
        float rotationSpeed;        // ��]���x
        float life;                 // ��������
        float maxLife;              // �ő吶������
        int textureHandle;          // �e�N�X�`���n���h��
        bool bounced;               // �n�ʂɃo�E���X�������̃t���O

        BlockFragment(float posX, float posY, float velX, float velY, int texture)
            : x(posX), y(posY), velocityX(velX), velocityY(velY)
            , rotation(0), rotationSpeed((rand() % 20 - 10) * 0.1f)
            , life(FRAGMENT_LIFE), maxLife(FRAGMENT_LIFE), textureHandle(texture), bounced(false) {}
    };

    // �ʂ̃u���b�N
    struct Block {
        float x, y;                     // �ʒu
        BlockType type;                 // �u���b�N�̎��
        BlockState state;               // �u���b�N�̏��
        int textureHandle;              // ���݂̃e�N�X�`���n���h��
        float bounceAnimation;          // �o�E���X�A�j���[�V�����p
        float bounceTimer;              // �o�E���X�^�C�}�[
        bool wasHit;                    // �q�b�g���ꂽ���̃t���O

        Block(float posX, float posY, BlockType blockType)
            : x(posX), y(posY), type(blockType), state(ACTIVE)
            , textureHandle(-1), bounceAnimation(0.0f), bounceTimer(0.0f), wasHit(false) {}
    };

    BlockSystem();
    ~BlockSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // �u���b�N�Ǘ�
    void AddCoinBlock(float x, float y);
    void AddBrickBlock(float x, float y);
    void ClearAllBlocks();

    // **�v���C���[�Ƃ̓����蔻��**
    bool CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight);
    void HandlePlayerCollision(Player* player, float newX, float newY);
    void CheckAndResolvePlayerCollisions(Player* player);

    // **�u���b�N���n�֘A�̊֐�**
    void HandleBlockLandingOnly(Player* player);
    bool CheckPlayerLandingOnBlocks(float playerX, float playerY, float playerWidth, float playerHeight);
    bool CheckPlayerLandingOnBlocksImproved(float playerX, float playerY, float playerWidth, float playerHeight); // **�ǉ�**
    void HandleBlockLanding(Player* player);
    float FindNearestBlockTop(float playerX, float playerY, float playerWidth);

    // **�q�b�g����֘A**
    bool CheckPlayerHitFromBelowImproved(const Block& block, Player* player);

    // �X�e�[�W�ʂ̃u���b�N�z�u
    void GenerateBlocksForStageIndex(int stageIndex);
    void GenerateBlocksForGrassStage();
    void GenerateBlocksForStoneStage();
    void GenerateBlocksForSandStage();
    void GenerateBlocksForSnowStage();
    void GenerateBlocksForPurpleStage();

    // �l�������R�C�������擾
    int GetCoinsFromBlocks() const { return coinsFromBlocks; }
    void ResetCoinCount() { coinsFromBlocks = 0; }

    

private:
    // �e�N�X�`���n���h��
    int coinBlockActiveTexture;     // block_coin_active.png
    int coinBlockEmptyTexture;      // block_coin.png
    int brickBlockTexture;          // block_empty.png�i�����K�u���b�N�p�j

    // �j�Зp�e�N�X�`���i�����K�u���b�N��4�����������́j
    int brickFragmentTextures[4];

    // �u���b�N�Ǘ�
    std::vector<std::unique_ptr<Block>> blocks;
    std::vector<std::unique_ptr<BlockFragment>> fragments;
    int coinsFromBlocks;

    // �����E���o�萔
    static constexpr float BLOCK_SIZE = 64.0f;          // �u���b�N�T�C�Y
    static constexpr float HIT_DETECTION_SIZE = 80.0f;  // �����蔻��T�C�Y
    static constexpr float BOUNCE_HEIGHT = 8.0f;        // �o�E���X����
    static constexpr float BOUNCE_DURATION = 0.3f;      // �o�E���X����
    static constexpr float FRAGMENT_GRAVITY = 0.5f;     // �j�Ђ̏d��
    static constexpr float FRAGMENT_LIFE = 2.0f;        // �j�Ђ̐�������

    // �w���p�[�֐�
    void LoadTextures();
    void CreateBrickFragmentTextures();
    void UpdateBlock(Block& block, Player* player);
    void UpdateFragments();
    void HandleCoinBlockHit(Block& block);
    void HandleBrickBlockHit(Block& block);
    void CreateBrickFragments(float blockX, float blockY);
    void DrawBlock(const Block& block, float cameraX);
    void DrawFragments(float cameraX);
    bool CheckPlayerHitFromBelow(const Block& block, Player* player);

    // **�ڍׂȓ����蔻��w���p�[**
    bool CheckAABBCollision(float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2);
    void ResolveCollision(Player* player, const Block& block, float playerX, float playerY);
    bool IsBlockSolid(const Block& block);

    float GetDistance(float x1, float y1, float x2, float y2);

    // **�����Փˏ����p�̃w���p�[�֐�**
    void HandleBlockLandingStable(Player* player);
    void HandleBlockCeilingCollision(Player* player);
    void StabilizeGroundState(Player* player);

    void UpdateBlockSafely(Block& block, Player* player);
};