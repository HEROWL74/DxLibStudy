#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include "HUDSystem.h"
#include "SoundManager.h"
#include <vector>
#include <string>

class BlockAthleticsScene
{
public:
    BlockAthleticsScene();
    ~BlockAthleticsScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }
    bool IsCompleted() const { return gameCompleted; }

private:
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �u���b�N�T�C�Y�i128x128�̑f�ނɍ��킹��j
    static const int BLOCK_SIZE = 128;

    // �X�e�[�W�T�C�Y
    static int STAGE_WIDTH; // ���I�ɕύX�\�ɂ���
    static const int STAGE_HEIGHT = 1080;

    // �u���b�N�^�C�v
    enum BlockType {
        BLOCK_NONE = 0,
        BLOCK_DIRT = 1,
        BLOCK_DIRT_TOP = 2,
        BLOCK_STONE = 3,
        BLOCK_STONE_TOP = 4,
        BLOCK_CRATE = 5,
        BLOCK_EXPLOSIVE = 6,
        BLOCK_COIN = 7
    };

    // �A�X���`�b�N�v�f�̍\����
    struct Block {
        BlockType type;
        float x, y;
        bool isActive;
        bool isCollectable;  // �R�C����N���[�g�̏ꍇ
        float animationPhase;  // �A�j���[�V�����p
        bool isBreakable;     // �j��\��
    };

    // �L�����N�^�[���
    struct BlockCharacter {
        int characterIndex;
        int graphicHandle;
        float x, y;
        float velocityX, velocityY;
        bool onGround;
        bool facingRight;
        int animationFrame;
        float animationTimer;
        int jumpPower;
        float moveSpeed;
    };

    // �Q�[�����
    enum GameState {
        STATE_PLAYING,
        STATE_COMPLETED,
        STATE_PAUSED
    };

    // �G�t�F�N�g
    struct Particle {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        int color;
        float scale;
    };

    // �S�[���G���A�֘A
    struct GoalArea {
        float x, y;
        float width, height;
        bool isActive;
        float animationPhase;
        int goalTextureHandle;
        int flagTextureHandle;
    };

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int fontHandle, largeFontHandle;

    // �u���b�N�p�e�N�X�`��
    int dirtHandle, dirtTopHandle, dirtDetailHandle;
    int stoneHandle, stoneTopHandle, stoneDetailHandle;
    int crateHandle, explosiveCrateHandle;
    int coinHandle;

    // �L�����N�^�[�e�N�X�`���i5��ށj
    int characterHandles[5];

    // �Q�[���I�u�W�F�N�g
    std::vector<Block> blocks;
    BlockCharacter player;
    HUDSystem hudSystem;

    // �J�����V�X�e��
    float cameraX;
    float previousPlayerX;
    static constexpr float CAMERA_FOLLOW_SPEED = 0.08f;

    // ��ԊǗ�
    bool exitRequested;
    bool gameCompleted;
    GameState currentState;
    int selectedCharacterIndex;
    std::string characterName;

    // �Q�[���i�s
    int coinsCollected;
    int totalCoins;
    int cratesDestroyed;
    int totalCrates;
    int playerLife;
    float gameTimer;
    float bestTime;

    // ���͊Ǘ�
    bool escPressed, escPressedPrev;
    bool spacePressed, spacePressedPrev;
    bool leftPressed, rightPressed;
    bool downPressed, downPressedPrev;
    bool bPressed, bPressedPrev;

    // �G�t�F�N�g
    std::vector<Particle> particles;

    struct BlockFragment {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        float rotation, rotationSpeed;
        int color;
        float scale;
        int fragmentType; // �j�Ђ̎�ށi0-3��4�����j
        int graphicHandle; // **�ǉ�: �j�Зp�̃e�N�X�`���n���h��**
        BlockType originalBlockType; // **�ǉ�: ���̃u���b�N�^�C�v**
    };

    std::vector<BlockFragment> blockFragments;

    // **�ǉ�: �j�Зp�e�N�X�`���n���h���z��**
    int fragmentHandles[7][4]; // [�u���b�N�^�C�v][�j�Дԍ�] ��2�����z��

    // �S�[���֘A
    GoalArea goalArea;
    bool showGoalHint;
    float goalHintTimer;

    void CreateFragmentFromTexture(int originalHandle, BlockType blockType);

    // �v���C�x�[�g���\�b�h
    void InitializeStage();
    void InitializePlayer();
    void InitializeGoalArea();
    void LoadTextures();
    void LoadFragmentTextures();
    void UpdateInput();
    void UpdatePlayer();
    void UpdateCamera();
    void UpdateBlocks();
    void UpdateEffects();
    void UpdateCollisions();
    void UpdateBlockDestruction();
    void UpdateGoalArea();

    void HandlePlayerBlockCollision(const Block& block, int blockIndex);
    void CollectCoin(int blockIndex);
    void DestroyCrate(int blockIndex);
    void ExplodeCrate(int blockIndex);
    void CreateParticles(float x, float y, int color, int count = 8);

    // �V�����G�t�F�N�g�֐�
    void CreateSparkleEffect(float x, float y);
    void CreateExplosionEffect(float x, float y);
    void CreateDebrisEffect(float x, float y);
    void CreateMovementParticles();
    void CreateJumpParticles();
    void CreateLandingParticles();
    void CreateHardLandingParticles();
    void CreateFastFallParticles();
    void CreateWallHitParticles();
    void RespawnPlayer();

    void DrawBackground();
    void DrawBlocks();
    void DrawPlayer();
    void DrawUI();
    void DrawEffects();
    void DrawParticles();
    void DrawGoalArea();
    void DrawGoalStatus();
    void DrawProgressBar(int x, int y, int width, int height, float progress, int fillColor, int bgColor);

    bool IsBlockCollision(float playerX, float playerY, float playerW, float playerH);
    Block* GetBlockAt(float x, float y);

    // ���[�e�B���e�B
    std::string GetCharacterDisplayName(int index);
    float Lerp(float a, float b, float t);
    void ResetGame();

    void HandleBlockDestruction();
    void DestroyBlock(int blockIndex);
    void CreateBlockFragments(float x, float y, BlockType blockType);
    void UpdateBlockFragments();
    void DrawBlockFragments();
};