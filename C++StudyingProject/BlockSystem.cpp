#include "BlockSystem.h"
#include "SoundManager.h"
#include <math.h>
#include <algorithm>
#include "Player.h"
#include "BlockAthleticsScene.h"
using namespace std;

BlockSystem::BlockSystem()
    : coinBlockActiveTexture(-1)
    , coinBlockEmptyTexture(-1)
    , brickBlockTexture(-1)
    , coinsFromBlocks(0)
{
    for (int i = 0; i < 4; i++) {
        brickFragmentTextures[i] = -1;
    }
}

BlockSystem::~BlockSystem()
{
    if (coinBlockActiveTexture != -1) DeleteGraph(coinBlockActiveTexture);
    if (coinBlockEmptyTexture != -1) DeleteGraph(coinBlockEmptyTexture);
    if (brickBlockTexture != -1) DeleteGraph(brickBlockTexture);

    for (int i = 0; i < 4; i++) {
        if (brickFragmentTextures[i] != -1) DeleteGraph(brickFragmentTextures[i]);
    }
}

void BlockSystem::Initialize()
{
    // **既存のデータをクリア**
    blocks.clear();
    fragments.clear();

    LoadTextures();
    CreateBrickFragmentTextures();
    coinsFromBlocks = 0;

    // **初期化完了の確認**
    OutputDebugStringA("BlockSystem: Initialize completed successfully\n");
}

void BlockSystem::LoadTextures()
{
    // ブロックテクスチャの読み込み
    coinBlockActiveTexture = LoadGraph("Sprites/Tiles/block_coin_active.png");
    coinBlockEmptyTexture = LoadGraph("Sprites/Tiles/block_coin.png");
    brickBlockTexture = LoadGraph("Sprites/Tiles/block_empty.png");

    // デバッグ出力
    if (coinBlockActiveTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_coin_active.png\n");
    }
    if (coinBlockEmptyTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_coin.png\n");
    }
    if (brickBlockTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_empty.png\n");
    }
}

void BlockSystem::CreateBrickFragmentTextures()
{
    if (brickBlockTexture == -1) return;

    // レンガブロックを4分割した破片テクスチャを作成
    int originalWidth, originalHeight;
    GetGraphSize(brickBlockTexture, &originalWidth, &originalHeight);

    int fragmentWidth = originalWidth / 2;
    int fragmentHeight = originalHeight / 2;

    for (int i = 0; i < 4; i++) {
        // 破片用のテクスチャをDerivationGraphで作成
        int srcX = (i % 2) * fragmentWidth;
        int srcY = (i / 2) * fragmentHeight;

        brickFragmentTextures[i] = DerivationGraph(
            srcX, srcY,                    // 切り出し開始位置
            fragmentWidth, fragmentHeight, // 切り出しサイズ
            brickBlockTexture             // 元画像
        );

        if (brickFragmentTextures[i] == -1) {
            OutputDebugStringA("BlockSystem: Failed to create brick fragment texture\n");
        }
    }

    OutputDebugStringA("BlockSystem: Created brick fragment textures using DerivationGraph\n");
}
void BlockSystem::Update(Player* player)
{
    if (!player) return;

    // **修正: より安全なイテレーション**
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        if (!*it) {
            OutputDebugStringA("BlockSystem::Update: Invalid block pointer found\n");
            continue;
        }

        if ((*it)->state != DESTROYED) {
            try {
                UpdateBlockSafely(**it, player);
            }
            catch (...) {
                OutputDebugStringA("BlockSystem::Update: Exception in UpdateBlock\n");
                continue;
            }
        }
    }

    // フラグメント更新
    try {
        UpdateFragments();
    }
    catch (...) {
        OutputDebugStringA("BlockSystem::Update: Exception in UpdateFragments\n");
    }
}

void BlockSystem::UpdateBlockSafely(Block& block, Player* player)
{
    // **修正: より確実な下からのヒット判定**
    bool isHitFromBelow = CheckPlayerHitFromBelowImproved(block, player);

    if (!block.wasHit && isHitFromBelow) {
        block.wasHit = true;

        switch (block.type) {
        case COIN_BLOCK:
            HandleCoinBlockHit(block);
            break;
        case BRICK_BLOCK:
            HandleBrickBlockHit(block);
            break;
        }

        // **修正: ヒット成功のデバッグ出力**
        char debugMsg[128];
        sprintf_s(debugMsg, "BlockSystem: Block hit successfully! Type: %d, Position: (%.1f, %.1f)\n",
            block.type, block.x, block.y);
        OutputDebugStringA(debugMsg);
    }

    // バウンスアニメーションの更新
    if (block.bounceTimer > 0.0f) {
        block.bounceTimer -= 0.016f;

        if (block.bounceTimer <= 0.0f) {
            block.bounceTimer = 0.0f;
            block.bounceAnimation = 0.0f;
        }
        else {
            float progress = 1.0f - (block.bounceTimer / BOUNCE_DURATION);
            block.bounceAnimation = sinf(progress * 3.14159265f) * BOUNCE_HEIGHT;
        }
    }

    // **修正: より信頼性の高いヒットフラグリセット**
    if (block.wasHit && !isHitFromBelow) {
        float distance = GetDistance(block.x + BLOCK_SIZE / 2, block.y + BLOCK_SIZE / 2,
            player->GetX(), player->GetY());
        if (distance > HIT_DETECTION_SIZE * 1.5f) {  // より大きな範囲
            block.wasHit = false;
        }
    }
}

void BlockSystem::UpdateBlock(Block& block, Player* player)
{
    // **修正: より正確な下からの衝突判定を使用**
    if (!block.wasHit && CheckPlayerHitFromBelowImproved(block, player)) {
        block.wasHit = true;

        switch (block.type) {
        case COIN_BLOCK:
            HandleCoinBlockHit(block);
            break;
        case BRICK_BLOCK:
            HandleBrickBlockHit(block);
            break;
        }
    }

    // バウンスアニメーションの更新
    if (block.bounceTimer > 0.0f) {
        block.bounceTimer -= 0.016f;

        if (block.bounceTimer <= 0.0f) {
            block.bounceTimer = 0.0f;
            block.bounceAnimation = 0.0f;
        }
        else {
            float progress = 1.0f - (block.bounceTimer / BOUNCE_DURATION);
            block.bounceAnimation = sinf(progress * 3.14159265f) * BOUNCE_HEIGHT;
        }
    }

    // **修正: ヒットフラグのリセット条件を改善**
    if (block.wasHit && !CheckPlayerHitFromBelowImproved(block, player)) {
        float distance = GetDistance(block.x, block.y, player->GetX(), player->GetY());
        if (distance > HIT_DETECTION_SIZE * 2.0f) {
            block.wasHit = false;
        }
    }
}
void BlockSystem::UpdateFragments()
{
    for (auto it = fragments.begin(); it != fragments.end();) {
        // **unique_ptrの有効性チェック**
        if (!*it) {
            OutputDebugStringA("BlockSystem::UpdateFragments: Invalid fragment pointer\n");
            it = fragments.erase(it);
            continue;
        }

        auto& fragment = *it;

        // 物理更新
        fragment->x += fragment->velocityX;
        fragment->y += fragment->velocityY;
        fragment->velocityY += FRAGMENT_GRAVITY;

        // 地面との衝突処理
        const float GROUND_LEVEL = 800.0f;
        if (fragment->y >= GROUND_LEVEL && fragment->velocityY > 0 && !fragment->bounced) {
            fragment->y = GROUND_LEVEL;
            fragment->velocityY *= -0.4f;
            fragment->velocityX *= 0.8f;
            fragment->bounced = true;
            fragment->rotationSpeed *= 0.6f;
        }

        fragment->velocityX *= 0.995f;
        fragment->rotation += fragment->rotationSpeed;
        fragment->life -= 0.016f;

        // **生存時間チェックと安全な削除**
        if (fragment->life <= 0.0f) {
            it = fragments.erase(it);
        }
        else {
            ++it;
        }
    }
}

void BlockSystem::HandleCoinBlockHit(Block& block)
{
    if (block.state == ACTIVE) {
        // コインブロックをアクティブから空に変更
        block.state = EMPTY;
        block.textureHandle = coinBlockEmptyTexture;

        // バウンスアニメーション開始
        block.bounceTimer = BOUNCE_DURATION;

        // **修正: コイン値得を確実に実行**
        coinsFromBlocks++;

        // サウンド再生
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

        // **詳細なデバッグ出力**
        char debugMsg[128];
        sprintf_s(debugMsg, "BlockSystem: COIN BLOCK HIT! Total coins from blocks: %d\n", coinsFromBlocks);
        OutputDebugStringA(debugMsg);

        // **成功確認のデバッグ**
        OutputDebugStringA("BlockSystem: Coin count increased successfully!\n");
    }
    else {
        // 既に空のブロックを叩いた場合
        OutputDebugStringA("BlockSystem: Hit empty coin block (no effect)\n");
    }
}

void BlockSystem::HandleBrickBlockHit(Block& block)
{
    // レンガブロックを破壊
    block.state = DESTROYED;

    // 破片を生成
    CreateBrickFragments(block.x, block.y);


     SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK_BLOCK); // 必要に応じて追加

    // デバッグ出力
    OutputDebugStringA("BlockSystem: Brick block destroyed!\n");
}

void BlockSystem::CreateBrickFragments(float blockX, float blockY)
{
    // 4つの破片を作成（左上、右上、左下、右下）
    for (int i = 0; i < 4; i++) {
        // 破片の初期位置（ブロック内の4分割位置）
        float fragmentX = blockX + (i % 2) * (BLOCK_SIZE / 2) + (BLOCK_SIZE / 4);
        float fragmentY = blockY + (i / 2) * (BLOCK_SIZE / 2) + (BLOCK_SIZE / 4);

        // より自然な破片の飛び散り方を計算
        float baseVelX = (i % 2 == 0) ? -1.0f : 1.0f;  // 左右方向
        float baseVelY = (i / 2 == 0) ? -1.5f : -0.8f; // 上下方向（上の破片がより高く飛ぶ）

        // ランダム要素を追加
        float randomFactorX = (rand() % 100 - 50) * 0.01f; // -0.5 ~ 0.5
        float randomFactorY = (rand() % 50) * 0.01f;       // 0.0 ~ 0.5（上向きのみ）

        float velocityX = baseVelX * (2.0f + randomFactorX);
        float velocityY = baseVelY - randomFactorY;

        // 破片を追加
        if (brickFragmentTextures[i] != -1) {
            auto fragment = std::make_unique<BlockFragment>(
                fragmentX, fragmentY, velocityX, velocityY, brickFragmentTextures[i]
            );

            // **より自然な回転速度を設定**
            fragment->rotationSpeed = (rand() % 40 - 20) * 0.05f; // -1.0 ~ 1.0 rad/frame

            fragments.push_back(std::move(fragment));
        }
    }

    // **ブロック破壊時のエフェクト音**
     SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK_BLOCK);

    OutputDebugStringA("BlockSystem: Created 4 brick fragments with improved physics\n");
}

void BlockSystem::Draw(float cameraX)
{
    // ブロックの描画
    for (const auto& block : blocks) {
        if (block->state != DESTROYED) {
            DrawBlock(*block, cameraX);
        }
    }

    // 破片の描画
    DrawFragments(cameraX);
}

void BlockSystem::DrawBlock(const Block& block, float cameraX)
{
    // 画面座標に変換
    int screenX = (int)(block.x - cameraX);
    int screenY = (int)(block.y - block.bounceAnimation);

    // 画面外なら描画しない（最適化）
    if (screenX < -BLOCK_SIZE || screenX > 1920 + BLOCK_SIZE) return;

    // テクスチャハンドルを決定
    int textureHandle = -1;
    switch (block.type) {
    case COIN_BLOCK:
        textureHandle = (block.state == ACTIVE) ? coinBlockActiveTexture : coinBlockEmptyTexture;
        break;
    case BRICK_BLOCK:
        textureHandle = brickBlockTexture;
        break;
    }

    // ブロック描画
    if (textureHandle != -1) {
        DrawExtendGraph(
            screenX, screenY,
            screenX + (int)BLOCK_SIZE, screenY + (int)BLOCK_SIZE,
            textureHandle, TRUE
        );
    }

#ifdef _DEBUG
    // デバッグ用当たり判定表示
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    DrawBox(screenX, screenY, screenX + (int)BLOCK_SIZE, screenY + (int)BLOCK_SIZE,
        GetColor(255, 0, 0), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

void BlockSystem::DrawFragments(float cameraX)
{
    for (const auto& fragment : fragments) {
        // **unique_ptrの有効性チェック**
        if (!fragment) {
            OutputDebugStringA("BlockSystem::DrawFragments: Invalid fragment pointer\n");
            continue;
        }

        // 画面座標に変換
        int screenX = (int)(fragment->x - cameraX);
        int screenY = (int)fragment->y;

        // 画面外なら描画しない
        if (screenX < -64 || screenX > 1920 + 64) continue;

        // 透明度を生存時間に応じて設定
        float lifeRatio = fragment->life / FRAGMENT_LIFE;
        int alpha = (int)(255 * lifeRatio);

        if (alpha > 0 && fragment->textureHandle != -1) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            int fragmentSize = 32;

            if (fragment->rotation != 0.0f) {
                int centerX = screenX + fragmentSize / 2;
                int centerY = screenY + fragmentSize / 2;

                DrawRotaGraph(
                    centerX, centerY,
                    1.0,
                    fragment->rotation,
                    fragment->textureHandle,
                    TRUE
                );
            }
            else {
                DrawExtendGraph(
                    screenX, screenY,
                    screenX + fragmentSize, screenY + fragmentSize,
                    fragment->textureHandle, TRUE
                );
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }
}

bool BlockSystem::CheckPlayerHitFromBelow(const Block& block, Player* player)
{
    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // プレイヤーが上向きに移動している場合のみ判定
    if (playerVelY >= 0) return false;

    // X軸の重なり判定
    const float PLAYER_WIDTH = 80.0f;
    bool xOverlap = (playerX + PLAYER_WIDTH / 2 > block.x &&
        playerX - PLAYER_WIDTH / 2 < block.x + BLOCK_SIZE);

    if (!xOverlap) return false;

    // Y軸の位置関係判定（プレイヤーがブロックの下にいる）
    const float PLAYER_HEIGHT = 100.0f;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float blockBottom = block.y + BLOCK_SIZE;

    // プレイヤーの頭がブロックの下面付近にある
    float distance = abs(playerTop - blockBottom);

    return (distance <= 20.0f); // 20ピクセル以内で判定
}

// **新追加：プレイヤーとブロックの当たり判定**
bool BlockSystem::CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight)
{
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        if (CheckAABBCollision(playerX - playerWidth / 2, playerY - playerHeight / 2, playerWidth, playerHeight,
            block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {
            return true;
        }
    }
    return false;
}

// **新追加：プレイヤーとの衝突処理**
void BlockSystem::HandlePlayerCollision(Player* player, float newX, float newY)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        // 新しい位置での当たり判定
        if (CheckAABBCollision(newX - PLAYER_WIDTH / 2, newY - PLAYER_HEIGHT / 2, PLAYER_WIDTH, PLAYER_HEIGHT,
            block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {
            ResolveCollision(player, *block, newX, newY);
        }
    }
}

// **新追加：AABB当たり判定**
bool BlockSystem::CheckAABBCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2);
}

void BlockSystem::ResolveCollision(Player* player, const Block& block, float playerX, float playerY)
{
    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float currentX = player->GetX();
    float currentY = player->GetY();
    float velX = player->GetVelocityX();
    float velY = player->GetVelocityY();

    // プレイヤーの境界
    float playerLeft = playerX - PLAYER_WIDTH / 2;
    float playerRight = playerX + PLAYER_WIDTH / 2;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float playerBottom = playerY + PLAYER_HEIGHT / 2;

    // ブロックの境界
    float blockLeft = block.x;
    float blockRight = block.x + BLOCK_SIZE;
    float blockTop = block.y;
    float blockBottom = block.y + BLOCK_SIZE;

    // 重なりの計算
    float overlapLeft = playerRight - blockLeft;
    float overlapRight = blockRight - playerLeft;
    float overlapTop = playerBottom - blockTop;
    float overlapBottom = blockBottom - playerTop;

    // 最小の重なりを見つけて、その方向に押し返す
    float minOverlapX = min(overlapLeft, overlapRight);
    float minOverlapY = min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY) {
        // 横方向の衝突
        if (overlapLeft < overlapRight) {
            // 右から左へ衝突
            player->SetPosition(blockLeft - PLAYER_WIDTH / 2 - 1, currentY);
        }
        else {
            // 左から右へ衝突
            player->SetPosition(blockRight + PLAYER_WIDTH / 2 + 1, currentY);
        }

        // 横方向の速度をリセット
        player->SetVelocityX(0.0f);
    }
    else {
        // 縦方向の衝突
        if (overlapTop < overlapBottom) {
            // **下から上へ衝突（プレイヤーがブロックの上に乗る）**
            player->SetPosition(currentX, blockTop - PLAYER_HEIGHT / 2 - 1);
            player->SetVelocityY(0.0f);
            player->SetOnGround(true);

            // **着地後の状態設定**
            if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
                if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                    player->SetState(Player::WALKING);
                }
                else {
                    player->SetState(Player::IDLE);
                }
            }

            OutputDebugStringA("BlockSystem: Player landed on block via collision resolution!\n");
        }
        else {
            // **上から下へ衝突（プレイヤーがブロックの下にぶつかる）**
            player->SetPosition(currentX, blockBottom + PLAYER_HEIGHT / 2 + 1);
            player->SetVelocityY(0.0f);

            // ジャンプ中に天井にぶつかった場合は落下状態に
            if (player->GetState() == Player::JUMPING) {
                player->SetState(Player::FALLING);
            }

            OutputDebugStringA("BlockSystem: Player hit block from below via collision resolution!\n");
        }
    }
}
// **新追加：ブロックが固体かどうかの判定**
bool BlockSystem::IsBlockSolid(const Block& block)
{
    // 破壊されたブロックは固体ではない
    if (block.state == DESTROYED) return false;

    // コインブロックは空になっても固体
    return true;
}

void BlockSystem::AddCoinBlock(float x, float y)
{
    try {
        auto block = std::make_unique<Block>(x, y, COIN_BLOCK);
        if (block) {  // **作成確認**
            block->textureHandle = coinBlockActiveTexture;
            blocks.push_back(std::move(block));
        }
    }
    catch (...) {
        OutputDebugStringA("BlockSystem::AddCoinBlock: Failed to create coin block\n");
    }
}

void BlockSystem::AddBrickBlock(float x, float y)
{
    try {
        auto block = std::make_unique<Block>(x, y, BRICK_BLOCK);
        if (block) {  // **作成確認**
            block->textureHandle = brickBlockTexture;
            blocks.push_back(std::move(block));
        }
    }
    catch (...) {
        OutputDebugStringA("BlockSystem::AddBrickBlock: Failed to create brick block\n");
    }
}

void BlockSystem::ClearAllBlocks()
{
    blocks.clear();
    fragments.clear();
    coinsFromBlocks = 0;
}

void BlockSystem::GenerateBlocksForStageIndex(int stageIndex)
{
    switch (stageIndex) {
    case 0: // Grass Stage
        GenerateBlocksForGrassStage();
        break;
    case 1: // Stone Stage
        GenerateBlocksForStoneStage();
        break;
    case 2: // Sand Stage
        GenerateBlocksForSandStage();
        break;
    case 3: // Snow Stage
        GenerateBlocksForSnowStage();
        break;
    case 4: // Purple Stage
        GenerateBlocksForPurpleStage();
        break;
    default:
        GenerateBlocksForGrassStage(); // デフォルト
        break;
    }
}

void BlockSystem::GenerateBlocksForGrassStage()
{
    ClearAllBlocks();

    // **修正: ステージのプラットフォームと被らない位置にブロック配置**
    // コインブロックの配置（空中の安全な場所）
    std::vector<std::pair<float, float>> coinBlocks = {
        {320, 480},   // 初期エリア上空（グリッド5上空）
        {960, 420},   // 中間エリア（グリッド15上空）
        {1920, 360},  // ギャップエリア（グリッド30上空）
        {3200, 400},  // 後半エリア（グリッド50上空）
        {4480, 320},  // 終盤エリア（グリッド70上空）
        {5760, 380}   // ゴール前（グリッド90上空）
    };

    // レンガブロックの配置（障害物として機能）
    std::vector<std::pair<float, float>> brickBlocks = {
        {640, 500},   // 初期エリアの障害物
        {1280, 440},  // 中間エリア
        {2560, 380},  // ギャップ2エリア
        {3840, 420},  // 後半エリア
        {5120, 340},  // 終盤エリア
        {6400, 400}   // 最終エリア
    };

    // ブロックを配置
    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }

    OutputDebugStringA("BlockSystem: Generated blocks for Grass Stage (Fixed positioning)\n");
}

void BlockSystem::GenerateBlocksForStoneStage()
{
    ClearAllBlocks();

    // **修正: 石ステージの地形を考慮した配置**
    std::vector<std::pair<float, float>> coinBlocks = {
        {480, 460},   // 石の隙間
        {1152, 380},  // 岩場の上空
        {2304, 420},  // 石の道の上
        {3456, 340},  // 高い岩場の上
        {4608, 380},  // 石の谷の上
        {5760, 300}   // 終盤の岩場
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {768, 480},   // 初期岩場
        {1920, 400},  // 中間岩場
        {3072, 360},  // 高岩場
        {4224, 400},  // 石の道
        {5376, 320},  // 終盤岩場
        {6528, 380}   // 最終岩場
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForSandStage()
{
    ClearAllBlocks();

    // **修正: 砂漠ステージのピラミッドと砂丘を避けた配置**
    std::vector<std::pair<float, float>> coinBlocks = {
        {448, 440},   // オアシス周辺
        {1344, 360},  // 砂丘の隙間
        {2688, 400},  // ピラミッド横
        {4032, 320},  // 砂の谷上空
        {5376, 380},  // 砂丘上空
        {6720, 300}   // 終盤砂漠
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {896, 460},   // 砂丘障害
        {2240, 380},  // ピラミッド周辺
        {3584, 340},  // 砂の道
        {4928, 400},  // 砂丘間
        {6272, 320},  // 終盤砂丘
        {7104, 380}   // 最終砂漠
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForSnowStage()
{
    ClearAllBlocks();

    // **修正: 雪山の地形を避けた配置**
    std::vector<std::pair<float, float>> coinBlocks = {
        {640, 420},   // 山麓エリア
        {1408, 340},  // 雪の足場上空
        {2816, 380},  // 山頂エリア
        {4224, 300},  // 氷河エリア
        {5632, 360},  // 雪原エリア
        {6976, 280}   // 最終山頂
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {1024, 440},  // 雪山障害
        {2432, 360},  // 氷の道
        {3840, 320},  // 雪だるま周辺
        {5248, 380},  // 雪原障害
        {6656, 300},  // 終盤雪山
        {7680, 340}   // 最終雪原
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForPurpleStage()
{
    ClearAllBlocks();

    // **修正: 魔法ステージの浮遊島を避けた魔法的配置**
    std::vector<std::pair<float,float>> coinBlocks = {
        {768, 380},   // 魔法の島周辺
        {1728, 320},  // 魔法の橋上空
        {3456, 360},  // 高い魔法島上空
        {5184, 280},  // 魔法の道上空
        {6144, 340},  // 最終魔法エリア
        {7296, 260}   // ゴール前魔法空間
    };


    std::vector<std::pair<float, float>> brickBlocks = {
        {1152, 400},  // 魔法障害
        {2880, 340},  // 魔法の試練
        {4608, 300},  // 高度魔法障害
        {5760, 360},  // 魔法の道障害
        {6912, 280},  // 終盤魔法障害
        {7680, 320}   // 最終魔法試練
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

float BlockSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

void BlockSystem::CheckAndResolvePlayerCollisions(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float velX = player->GetVelocityX();
    float velY = player->GetVelocityY();

    // **1. 横方向の衝突処理**
    if (fabsf(velX) > 0.1f) {
        float newX = playerX;
        for (const auto& block : blocks) {
            if (!IsBlockSolid(*block)) continue;
            if (CheckAABBCollision(newX - PLAYER_WIDTH / 2, playerY - PLAYER_HEIGHT / 2,
                PLAYER_WIDTH, PLAYER_HEIGHT,
                block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {

                if (velX > 0) {
                    player->SetPosition(block->x - PLAYER_WIDTH / 2 - 1, playerY);
                }
                else {
                    player->SetPosition(block->x + BLOCK_SIZE + PLAYER_WIDTH / 2 + 1, playerY);
                }
                player->SetVelocityX(0.0f);
                break;
            }
        }
    }

    // **2. 下方向の移動（着地処理）- 統合版**
    if (velY > 0.0f || (!player->IsOnGround() && velY >= 0.0f)) {
        bool landedOnBlock = false;
        float nearestBlockTop = -1.0f;
        float minDistance = 999.0f;

        float footY = playerY + PLAYER_HEIGHT / 2;

        for (const auto& block : blocks) {
            if (!IsBlockSolid(*block)) continue;

            float blockTop = block->y;
            float blockLeft = block->x;
            float blockRight = block->x + BLOCK_SIZE;

            // X方向の重なり判定（より寛容に）
            bool xOverlap = (playerX + PLAYER_WIDTH / 3 > blockLeft &&
                playerX - PLAYER_WIDTH / 3 < blockRight);

            if (!xOverlap) continue;

            // Y方向の着地判定
            float distanceToTop = footY - blockTop;
            if (distanceToTop >= -2.0f && distanceToTop <= 15.0f && distanceToTop < minDistance) {
                minDistance = distanceToTop;
                nearestBlockTop = blockTop;
                landedOnBlock = true;
            }
        }

        if (landedOnBlock && nearestBlockTop != -1.0f) {
            // **ブロック上に着地**
            player->SetPosition(playerX, nearestBlockTop - PLAYER_HEIGHT / 2);
            player->SetVelocityY(0.0f);
            player->SetOnGround(true);

            // 状態設定
            if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
                bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);
                if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
                    player->SetState(Player::DUCKING);
                }
                else if (hasInput) {
                    player->SetState(Player::WALKING);
                }
                else {
                    player->SetState(Player::IDLE);
                }
            }

            OutputDebugStringA("BlockSystem: Player landed on block - OnGround SET!\n");
        }
    }

    // **3. 上方向の移動（天井衝突）**
    else if (velY < 0.0f) {
        float headY = playerY - PLAYER_HEIGHT / 2;

        for (const auto& block : blocks) {
            if (!IsBlockSolid(*block)) continue;

            float blockBottom = block->y + BLOCK_SIZE;
            float blockLeft = block->x;
            float blockRight = block->x + BLOCK_SIZE;

            bool xOverlap = (playerX + PLAYER_WIDTH / 2 > blockLeft &&
                playerX - PLAYER_WIDTH / 2 < blockRight);

            if (xOverlap && headY <= blockBottom && headY >= block->y) {
                player->SetPosition(playerX, blockBottom + PLAYER_HEIGHT / 2 + 1);
                player->SetVelocityY(0.0f);

                if (player->GetState() == Player::JUMPING) {
                    player->SetState(Player::FALLING);
                }
                break;
            }
        }
    }

    // **4. 地面状態の統合確認**
    if (player->IsOnGround()) {
        bool stillOnSomething = false;
        float footY = playerY + PLAYER_HEIGHT / 2 + 4; // 少し下をチェック

        // ブロック上にいるかチェック
        for (const auto& block : blocks) {
            if (!IsBlockSolid(*block)) continue;

            float blockTop = block->y;
            float blockLeft = block->x;
            float blockRight = block->x + BLOCK_SIZE;

            bool xOverlap = (playerX + PLAYER_WIDTH / 2 - 8 > blockLeft &&
                playerX - PLAYER_WIDTH / 2 + 8 < blockRight);

            float distanceToTop = footY - blockTop;

            if (xOverlap && distanceToTop >= -2.0f && distanceToTop <= 10.0f) {
                stillOnSomething = true;
                // 位置を微調整
                player->SetPosition(playerX, blockTop - PLAYER_HEIGHT / 2);
                break;
            }
        }

        // **ブロック上にいない場合は、ステージ地面判定は Player::HandleCollisions() に任せる**
        if (!stillOnSomething) {
            // ここでは何もしない - ステージ地面の判定は既に完了済み
            // onGroundをfalseにするのは慎重に行う

            // 完全に何も触れていない場合のみ落下状態に
            // この判定は次回のフレームで自然に処理される
        }
    }
}
void BlockSystem::HandleBlockLandingStable(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float LANDING_TOLERANCE = 15.0f; // **より寛容な判定**

    float playerX = player->GetX();
    float playerY = player->GetY();
    float velY = player->GetVelocityY();

    // **下向きに移動中かつ地面にいない場合のみ処理**
    if (velY <= 0.0f || player->IsOnGround()) return;

    float footY = playerY + PLAYER_HEIGHT / 2;
    float nearestBlockTop = -1.0f;
    float minDistance = 999.0f;

    // **足元の5点でより確実にチェック**
    float checkPoints[] = {
        playerX - PLAYER_WIDTH / 2 + 8,    // 左端
        playerX - PLAYER_WIDTH / 4,        // 左寄り
        playerX,                         // 中央
        playerX + PLAYER_WIDTH / 4,        // 右寄り
        playerX + PLAYER_WIDTH / 2 - 8     // 右端
    };

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **より厳密なX方向の重なりチェック**
        bool xOverlap = false;
        for (float checkX : checkPoints) {
            if (checkX >= blockLeft - 2 && checkX <= blockRight + 2) { // 少し余裕を持たせる
                xOverlap = true;
                break;
            }
        }

        if (!xOverlap) continue;

        // **Y方向の着地判定**
        float distanceToTop = footY - blockTop;
        if (distanceToTop >= -2.0f && distanceToTop <= LANDING_TOLERANCE && distanceToTop < minDistance) {
            minDistance = distanceToTop;
            nearestBlockTop = blockTop;
        }
    }

    // **着地処理**
    if (nearestBlockTop != -1.0f) {
        // **正確にブロック上面に配置**
        float targetY = nearestBlockTop - PLAYER_HEIGHT / 2;

        player->SetPosition(playerX, targetY);
        player->SetVelocityY(0.0f);
        player->SetOnGround(true);


        // **状態を適切に設定**
        if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
            bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);
            if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
                player->SetState(Player::DUCKING);
            }
            else if (hasInput) {
                player->SetState(Player::WALKING);
            }
            else {
                player->SetState(Player::IDLE);
            }
        }

        // **デバッグ出力**
        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Player landed on block! Y: %.1f -> %.1f\n",
            playerY, targetY);
        OutputDebugStringA(debugMsg);
    }
}

// ブロック下面への衝突処理
void BlockSystem::HandleBlockCeilingCollision(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float headY = playerY - PLAYER_HEIGHT / 2;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockBottom = block->y + BLOCK_SIZE;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // 頭部の衝突チェック
        bool xOverlap = (playerX + PLAYER_WIDTH / 2 > blockLeft &&
            playerX - PLAYER_WIDTH / 2 < blockRight);

        if (xOverlap && headY <= blockBottom && headY >= block->y) {
            // ブロック下面にぶつかった
            player->SetPosition(playerX, blockBottom + PLAYER_HEIGHT / 2 + 1);
            player->SetVelocityY(0.0f);

            if (player->GetState() == Player::JUMPING) {
                player->SetState(Player::FALLING);
            }
            break;
        }
    }
}

// 地面状態の安定化
void BlockSystem::StabilizeGroundState(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float GROUND_CHECK_TOLERANCE = 8.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float footY = playerY + PLAYER_HEIGHT / 2;

    bool stillOnBlock = false;

    // 現在ブロック上にいるかチェック
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // より広い範囲でチェック（安定化のため）
        bool xOverlap = (playerX + PLAYER_WIDTH / 2 - 8 > blockLeft &&
            playerX - PLAYER_WIDTH / 2 + 8 < blockRight);

        float distanceToTop = footY - blockTop;

        if (xOverlap && distanceToTop >= -2.0f && distanceToTop <= GROUND_CHECK_TOLERANCE) {
            stillOnBlock = true;
            // 位置を微調整（ブロック上面に固定）
            player->SetPosition(playerX, blockTop - PLAYER_HEIGHT / 2);
            break;
        }
    }

    // ブロックから離れた場合
    if (!stillOnBlock) {
        player->SetOnGround(false);
        if (player->GetState() != Player::JUMPING) {
            player->SetState(Player::FALLING);
        }
    }
}

void BlockSystem::HandleBlockLandingOnly(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **厳密な条件: 下向きに移動中で、地面にいない場合のみ**
    if (playerVelY <= 1.0f || player->IsOnGround()) return;

    // **プレイヤーの足元の正確な位置**
    float footY = playerY + PLAYER_HEIGHT / 2;

    // **非常に狭い判定範囲（プレイヤーの中央部分のみ）**
    float checkLeft = playerX - 20.0f;    // 中央から左20px
    float checkRight = playerX + 20.0f;   // 中央から右20px

    bool validLanding = false;
    float targetBlockTop = -1.0f;
    float minDistance = 999.0f;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **X軸の重なりチェック（非常に狭く）**
        bool xOverlap = (checkRight > blockLeft && checkLeft < blockRight);
        if (!xOverlap) continue;

        // **Y軸の着地判定（非常に厳密）**
        float distanceToTop = footY - blockTop;

        if (distanceToTop >= 0 && distanceToTop <= 6.0f && distanceToTop < minDistance) {
            minDistance = distanceToTop;
            targetBlockTop = blockTop;
            validLanding = true;
        }
    }

    if (validLanding && targetBlockTop != -1.0f) {
        // **ブロック上に正確に着地**
        player->SetPosition(playerX, targetBlockTop - PLAYER_HEIGHT / 2);
        player->SetVelocityY(0.0f);
        player->SetOnGround(true);

        // **適切な状態に変更**
        if (player->GetState() == Player::FALLING) {
            bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);
            if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
                player->SetState(Player::DUCKING);
            }
            else if (hasInput) {
                player->SetState(Player::WALKING);
            }
            else {
                player->SetState(Player::IDLE);
            }
        }

        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Block landing! Distance: %.1f, BlockTop: %.1f\n",
            minDistance, targetBlockTop);
        OutputDebugStringA(debugMsg);
    }
}
bool BlockSystem::CheckPlayerHitFromBelowImproved(const Block& block, Player* player)
{
    if (!player) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **修正: 上向きに移動中かつ十分な速度があるかチェック**
    if (playerVelY > 1.0f) return false;  // より緩い条件

    // **修正: より寛容なX軸の重なり判定**
    const float PLAYER_WIDTH = 80.0f;
    const float COLLISION_MARGIN = 8.0f;   // 判定の余裕を増加

    float playerLeft = playerX - PLAYER_WIDTH / 2 + COLLISION_MARGIN;
    float playerRight = playerX + PLAYER_WIDTH / 2 - COLLISION_MARGIN;
    float blockLeft = block.x;
    float blockRight = block.x + BLOCK_SIZE;

    bool xOverlap = (playerRight > blockLeft && playerLeft < blockRight);
    if (!xOverlap) return false;

    // **修正: より正確なY軸の位置関係判定**
    const float PLAYER_HEIGHT = 100.0f;
    const float HIT_TOLERANCE = 32.0f;     // 許容範囲を拡大

    float playerHead = playerY - PLAYER_HEIGHT / 2;
    float blockBottom = block.y + BLOCK_SIZE;

    // **修正: 次フレームの位置も考慮**
    float nextPlayerHead = playerHead + playerVelY;

    // 現在または次フレームで衝突する可能性をチェック
    bool isHittingFromBelow = false;

    // 現在の位置での判定
    float currentDistance = abs(playerHead - blockBottom);
    if (currentDistance <= HIT_TOLERANCE && playerHead <= blockBottom + 5.0f) {
        isHittingFromBelow = true;
    }

    // 次フレームの位置での判定
    float nextDistance = abs(nextPlayerHead - blockBottom);
    if (nextDistance <= HIT_TOLERANCE && nextPlayerHead <= blockBottom + 5.0f) {
        isHittingFromBelow = true;
    }

    // **デバッグ出力を改善**
    if (isHittingFromBelow) {
        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Hit from below! Player(%.1f,%.1f) VelY:%.1f Block(%.1f,%.1f) Distance:%.1f\n",
            playerX, playerY, playerVelY, block.x, block.y, min(currentDistance, nextDistance));
        OutputDebugStringA(debugMsg);
    }

    return isHittingFromBelow;
}




// **新実装: CheckPlayerLandingOnBlocks - ブロック上への着地判定**
bool BlockSystem::CheckPlayerLandingOnBlocks(float playerX, float playerY, float playerWidth, float playerHeight)
{
    const float LANDING_TOLERANCE = 8.0f;

    // プレイヤーの足元位置
    float footY = playerY + playerHeight / 2;
    float leftFoot = playerX - playerWidth / 3;
    float rightFoot = playerX + playerWidth / 3;
    float centerFoot = playerX;

    // 各ブロックとの衝突をチェック
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        // ブロックの上面
        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // X軸の重なりチェック
        bool xOverlap = (rightFoot > blockLeft && leftFoot < blockRight);
        if (!xOverlap) continue;

        // Y軸の着地判定（プレイヤーの足がブロックの上面付近）
        float distanceToBlockTop = abs(footY - blockTop);
        if (distanceToBlockTop <= LANDING_TOLERANCE && footY >= blockTop) {

            char debugMsg[256];
            sprintf_s(debugMsg, "BlockSystem: Player landing on block! FootY: %.1f, BlockTop: %.1f, Distance: %.1f\n",
                footY, blockTop, distanceToBlockTop);
            OutputDebugStringA(debugMsg);

            return true;
        }
    }

    return false;
}

bool BlockSystem::CheckPlayerLandingOnBlocksImproved(float playerX, float playerY, float playerWidth, float playerHeight)
{
    const float LANDING_TOLERANCE = 8.0f;  // 寛容な判定

    // **修正: より広い範囲でのチェック**
    float footY = playerY + playerHeight / 2;
    float checkLeft = playerX - playerWidth / 3;    // 1/3幅でチェック
    float checkRight = playerX + playerWidth / 3;   // 1/3幅でチェック

    // **修正: 5点でのチェック**
    float checkPoints[] = {
        checkLeft,
        playerX - playerWidth / 6,
        playerX,
        playerX + playerWidth / 6,
        checkRight
    };

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **修正: より精密なX軸の重なりチェック**
        bool xOverlap = false;
        for (float checkX : checkPoints) {
            if (checkX >= blockLeft - 2.0f && checkX <= blockRight + 2.0f) {
                xOverlap = true;
                break;
            }
        }

        if (!xOverlap) continue;

        // **修正: より正確なY軸の着地判定**
        float distanceToBlockTop = footY - blockTop;

        if (distanceToBlockTop >= 0 && distanceToBlockTop <= LANDING_TOLERANCE) {
            char debugMsg[256];
            sprintf_s(debugMsg, "BlockSystem: Landing detected! FootY: %.1f, BlockTop: %.1f, Distance: %.1f\n",
                footY, blockTop, distanceToBlockTop);
            OutputDebugStringA(debugMsg);
            return true;
        }
    }

    return false;
}
// **新実装: HandleBlockLanding - ブロック上着地の処理**
void BlockSystem::HandleBlockLanding(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **下向きに移動中で、まだ地上にいない場合のみチェック**
    if (playerVelY <= 0.0f || player->IsOnGround()) return;

    // **ブロック上への着地をチェック**
    if (CheckPlayerLandingOnBlocks(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {

        // **最も近いブロックの上面に正確に配置**
        float targetY = FindNearestBlockTop(playerX, playerY, PLAYER_WIDTH);

        if (targetY != -1.0f) {
            // プレイヤーをブロックの上に配置
            player->SetPosition(playerX, targetY - PLAYER_HEIGHT / 2);
            player->SetVelocityY(0.0f);
            player->SetOnGround(true);  // **重要: 地面状態を確実に設定**

            // **適切な状態に変更**
            if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
                if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                    player->SetState(Player::WALKING);
                }
                else {
                    player->SetState(Player::IDLE);
                }
            }

            OutputDebugStringA("BlockSystem: Player successfully landed on block!\n");
        }
    }
}

float BlockSystem::FindNearestBlockTop(float playerX, float playerY, float playerWidth)
{
    float nearestBlockTop = -1.0f;
    float minDistance = 999.0f;

    const float LANDING_TOLERANCE = 4.0f; // **厳密な判定**
    float footY = playerY + 50.0f; // プレイヤーの足元付近

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **X軸の重なりチェック（より厳密に）**
        float playerLeft = playerX - playerWidth / 4;   // **範囲を狭める**
        float playerRight = playerX + playerWidth / 4;  // **範囲を狭める**

        bool xOverlap = (playerRight >= blockLeft && playerLeft <= blockRight);
        if (!xOverlap) continue;

        // **Y軸の距離チェック**
        float distance = footY - blockTop;
        if (distance >= 0 && distance <= LANDING_TOLERANCE && distance < minDistance) {
            minDistance = distance;
            nearestBlockTop = blockTop;
        }
    }

    return nearestBlockTop;
}