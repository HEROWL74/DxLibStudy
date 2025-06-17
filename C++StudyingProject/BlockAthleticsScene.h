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
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // ブロックサイズ（128x128の素材に合わせる）
    static const int BLOCK_SIZE = 128;

    // ステージサイズ
    static int STAGE_WIDTH; // 動的に変更可能にする
    static const int STAGE_HEIGHT = 1080;

    // ブロックタイプ
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

    // アスレチック要素の構造体
    struct Block {
        BlockType type;
        float x, y;
        bool isActive;
        bool isCollectable;  // コインやクレートの場合
        float animationPhase;  // アニメーション用
        bool isBreakable;     // 破壊可能か
    };

    // キャラクター情報
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

    // ゲーム状態
    enum GameState {
        STATE_PLAYING,
        STATE_COMPLETED,
        STATE_PAUSED
    };

    // エフェクト
    struct Particle {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        int color;
        float scale;
    };

    // ゴールエリア関連
    struct GoalArea {
        float x, y;
        float width, height;
        bool isActive;
        float animationPhase;
        int goalTextureHandle;
        int flagTextureHandle;
    };

    // テクスチャハンドル
    int backgroundHandle;
    int fontHandle, largeFontHandle;

    // ブロック用テクスチャ
    int dirtHandle, dirtTopHandle, dirtDetailHandle;
    int stoneHandle, stoneTopHandle, stoneDetailHandle;
    int crateHandle, explosiveCrateHandle;
    int coinHandle;

    // キャラクターテクスチャ（5種類）
    int characterHandles[5];

    // ゲームオブジェクト
    std::vector<Block> blocks;
    BlockCharacter player;
    HUDSystem hudSystem;

    // カメラシステム
    float cameraX;
    float previousPlayerX;
    static constexpr float CAMERA_FOLLOW_SPEED = 0.08f;

    // 状態管理
    bool exitRequested;
    bool gameCompleted;
    GameState currentState;
    int selectedCharacterIndex;
    std::string characterName;

    // ゲーム進行
    int coinsCollected;
    int totalCoins;
    int cratesDestroyed;
    int totalCrates;
    int playerLife;
    float gameTimer;
    float bestTime;

    // 入力管理
    bool escPressed, escPressedPrev;
    bool spacePressed, spacePressedPrev;
    bool leftPressed, rightPressed;
    bool downPressed, downPressedPrev;
    bool bPressed, bPressedPrev;

    // エフェクト
    std::vector<Particle> particles;

    struct BlockFragment {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        float rotation, rotationSpeed;
        int color;
        float scale;
        int fragmentType; // 破片の種類（0-3の4分割）
        int graphicHandle; // **追加: 破片用のテクスチャハンドル**
        BlockType originalBlockType; // **追加: 元のブロックタイプ**
    };

    std::vector<BlockFragment> blockFragments;

    // **追加: 破片用テクスチャハンドル配列**
    int fragmentHandles[7][4]; // [ブロックタイプ][破片番号] の2次元配列

    // ゴール関連
    GoalArea goalArea;
    bool showGoalHint;
    float goalHintTimer;

    void CreateFragmentFromTexture(int originalHandle, BlockType blockType);

    // プライベートメソッド
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

    // 新しいエフェクト関数
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

    // ユーティリティ
    std::string GetCharacterDisplayName(int index);
    float Lerp(float a, float b, float t);
    void ResetGame();

    void HandleBlockDestruction();
    void DestroyBlock(int blockIndex);
    void CreateBlockFragments(float x, float y, BlockType blockType);
    void UpdateBlockFragments();
    void DrawBlockFragments();
};