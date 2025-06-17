#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>

// **前方宣言**
class Player;

class BlockSystem {
public:
    // ブロックの種類
    enum BlockType {
        COIN_BLOCK,     // コインブロック
        BRICK_BLOCK     // レンガブロック
    };

    // ブロックの状態
    enum BlockState {
        ACTIVE,         // アクティブ状態
        EMPTY,          // 空状態（コインブロック用）
        DESTROYED       // 破壊された状態（レンガブロック用）
    };

    // 破片の構造体
    struct BlockFragment {
        float x, y;                 // 位置
        float velocityX, velocityY; // 速度
        float rotation;             // 回転角度
        float rotationSpeed;        // 回転速度
        float life;                 // 生存期間
        float maxLife;              // 最大生存期間
        int textureHandle;          // テクスチャハンドル
        bool bounced;               // 地面にバウンスしたかのフラグ

        BlockFragment(float posX, float posY, float velX, float velY, int texture)
            : x(posX), y(posY), velocityX(velX), velocityY(velY)
            , rotation(0), rotationSpeed((rand() % 20 - 10) * 0.1f)
            , life(FRAGMENT_LIFE), maxLife(FRAGMENT_LIFE), textureHandle(texture), bounced(false) {}
    };

    // 個別のブロック
    struct Block {
        float x, y;                     // 位置
        BlockType type;                 // ブロックの種類
        BlockState state;               // ブロックの状態
        int textureHandle;              // 現在のテクスチャハンドル
        float bounceAnimation;          // バウンスアニメーション用
        float bounceTimer;              // バウンスタイマー
        bool wasHit;                    // ヒットされたかのフラグ

        Block(float posX, float posY, BlockType blockType)
            : x(posX), y(posY), type(blockType), state(ACTIVE)
            , textureHandle(-1), bounceAnimation(0.0f), bounceTimer(0.0f), wasHit(false) {}
    };

    BlockSystem();
    ~BlockSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // ブロック管理
    void AddCoinBlock(float x, float y);
    void AddBrickBlock(float x, float y);
    void ClearAllBlocks();

    // **プレイヤーとの当たり判定**
    bool CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight);
    void HandlePlayerCollision(Player* player, float newX, float newY);
    void CheckAndResolvePlayerCollisions(Player* player);

    // **ブロック着地関連の関数**
    void HandleBlockLandingOnly(Player* player);
    bool CheckPlayerLandingOnBlocks(float playerX, float playerY, float playerWidth, float playerHeight);
    bool CheckPlayerLandingOnBlocksImproved(float playerX, float playerY, float playerWidth, float playerHeight); // **追加**
    void HandleBlockLanding(Player* player);
    float FindNearestBlockTop(float playerX, float playerY, float playerWidth);

    // **ヒット判定関連**
    bool CheckPlayerHitFromBelowImproved(const Block& block, Player* player);

    // ステージ別のブロック配置
    void GenerateBlocksForStageIndex(int stageIndex);
    void GenerateBlocksForGrassStage();
    void GenerateBlocksForStoneStage();
    void GenerateBlocksForSandStage();
    void GenerateBlocksForSnowStage();
    void GenerateBlocksForPurpleStage();

    // 獲得したコイン数を取得
    int GetCoinsFromBlocks() const { return coinsFromBlocks; }
    void ResetCoinCount() { coinsFromBlocks = 0; }

    

private:
    // テクスチャハンドル
    int coinBlockActiveTexture;     // block_coin_active.png
    int coinBlockEmptyTexture;      // block_coin.png
    int brickBlockTexture;          // block_empty.png（レンガブロック用）

    // 破片用テクスチャ（レンガブロックを4分割したもの）
    int brickFragmentTextures[4];

    // ブロック管理
    std::vector<std::unique_ptr<Block>> blocks;
    std::vector<std::unique_ptr<BlockFragment>> fragments;
    int coinsFromBlocks;

    // 物理・演出定数
    static constexpr float BLOCK_SIZE = 64.0f;          // ブロックサイズ
    static constexpr float HIT_DETECTION_SIZE = 80.0f;  // 当たり判定サイズ
    static constexpr float BOUNCE_HEIGHT = 8.0f;        // バウンス高さ
    static constexpr float BOUNCE_DURATION = 0.3f;      // バウンス期間
    static constexpr float FRAGMENT_GRAVITY = 0.5f;     // 破片の重力
    static constexpr float FRAGMENT_LIFE = 2.0f;        // 破片の生存期間

    // ヘルパー関数
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

    // **詳細な当たり判定ヘルパー**
    bool CheckAABBCollision(float x1, float y1, float w1, float h1,
        float x2, float y2, float w2, float h2);
    void ResolveCollision(Player* player, const Block& block, float playerX, float playerY);
    bool IsBlockSolid(const Block& block);

    float GetDistance(float x1, float y1, float x2, float y2);

    // **統合衝突処理用のヘルパー関数**
    void HandleBlockLandingStable(Player* player);
    void HandleBlockCeilingCollision(Player* player);
    void StabilizeGroundState(Player* player);

    void UpdateBlockSafely(Block& block, Player* player);
};