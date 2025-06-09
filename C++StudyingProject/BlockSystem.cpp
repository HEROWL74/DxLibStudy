#include "BlockSystem.h"
#include "SoundManager.h"
#include <math.h>
#include <algorithm>

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
    LoadTextures();
    CreateBrickFragmentTextures();
    coinsFromBlocks = 0;
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

    // 全ブロックの更新
    for (auto& block : blocks) {
        if (block->state != DESTROYED) {
            UpdateBlock(*block, player);
        }
    }

    // 破片の更新
    UpdateFragments();
}

void BlockSystem::UpdateBlock(Block& block, Player* player)
{
    // プレイヤーが下からブロックに当たったかチェック
    if (!block.wasHit && CheckPlayerHitFromBelow(block, player)) {
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
        block.bounceTimer -= 0.016f; // 60FPS想定

        if (block.bounceTimer <= 0.0f) {
            block.bounceTimer = 0.0f;
            block.bounceAnimation = 0.0f;
        }
        else {
            // サイン波でバウンス効果
            float progress = 1.0f - (block.bounceTimer / BOUNCE_DURATION);
            block.bounceAnimation = sinf(progress * 3.14159265f) * BOUNCE_HEIGHT;
        }
    }

    // ヒットフラグのリセット（プレイヤーが離れた場合）
    if (block.wasHit && !CheckPlayerHitFromBelow(block, player)) {
        float distance = GetDistance(block.x, block.y, player->GetX(), player->GetY());
        if (distance > HIT_DETECTION_SIZE * 1.5f) { // 十分離れたらリセット
            block.wasHit = false;
        }
    }
}

void BlockSystem::UpdateFragments()
{
    for (auto it = fragments.begin(); it != fragments.end();) {
        auto& fragment = *it;

        // 物理更新
        fragment->x += fragment->velocityX;
        fragment->y += fragment->velocityY;
        fragment->velocityY += FRAGMENT_GRAVITY; // 重力適用

        // **地面との簡易バウンス処理**
        const float GROUND_LEVEL = 800.0f; // 地面レベル（適宜調整）
        if (fragment->y >= GROUND_LEVEL && fragment->velocityY > 0 && !fragment->bounced) {
            fragment->y = GROUND_LEVEL;
            fragment->velocityY *= -0.4f; // 40%の反発係数
            fragment->velocityX *= 0.8f;  // 地面摩擦で横速度減少
            fragment->bounced = true;

            // バウンス後は回転速度も減少
            fragment->rotationSpeed *= 0.6f;
        }

        // **空気抵抗の適用**
        fragment->velocityX *= 0.995f; // 徐々に横速度が減少

        // 回転更新
        fragment->rotation += fragment->rotationSpeed;

        // 生存時間の減少
        fragment->life -= 0.016f; // 60FPS想定

        // 生存時間が切れたら削除
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

        // コイン獲得
        coinsFromBlocks++;

        // サウンド再生
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

        // デバッグ出力
        OutputDebugStringA("BlockSystem: Coin block hit! Coins from blocks: ");
        char coinMsg[32];
        sprintf_s(coinMsg, "%d\n", coinsFromBlocks);
        OutputDebugStringA(coinMsg);
    }
}

void BlockSystem::HandleBrickBlockHit(Block& block)
{
    // レンガブロックを破壊
    block.state = DESTROYED;

    // 破片を生成
    CreateBrickFragments(block.x, block.y);

    // サウンド再生（ブロック破壊音）
    // SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK); // 必要に応じて追加

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
    // SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK);

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
        // 画面座標に変換
        int screenX = (int)(fragment->x - cameraX);
        int screenY = (int)fragment->y;

        // 画面外なら描画しない
        if (screenX < -64 || screenX > 1920 + 64) continue;

        // 透明度を生存時間に基づいて設定
        float lifeRatio = fragment->life / FRAGMENT_LIFE;
        int alpha = (int)(255 * lifeRatio);

        if (alpha > 0 && fragment->textureHandle != -1) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            // 破片サイズ（元のブロックの1/2）
            int fragmentSize = 32;

            // **回転描画（DerivationGraphで作成した破片テクスチャを回転）**
            if (fragment->rotation != 0.0f) {
                // 回転の中心点
                int centerX = screenX + fragmentSize / 2;
                int centerY = screenY + fragmentSize / 2;

                // 回転描画（DxLibのDrawRotaGraph相当の処理）
                DrawRotaGraph(
                    centerX, centerY,                    // 描画中心
                    1.0,                                 // 拡大率
                    fragment->rotation,                  // 回転角度（ラジアン）
                    fragment->textureHandle,            // DerivationGraphで作成したテクスチャ
                    TRUE                                // 透明色処理
                );
            }
            else {
                // 通常描画（回転なし）
                DrawExtendGraph(
                    screenX, screenY,
                    screenX + fragmentSize, screenY + fragmentSize,
                    fragment->textureHandle, TRUE
                );
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // **デバッグ：破片の軌道を表示**
#ifdef _DEBUG
// 破片の速度ベクトルを線で表示
            int velocityEndX = screenX + (int)(fragment->velocityX * 10);
            int velocityEndY = screenY + (int)(fragment->velocityY * 10);
            DrawLine(screenX + fragmentSize / 2, screenY + fragmentSize / 2,
                velocityEndX, velocityEndY, GetColor(255, 0, 0));

            // 生存時間を数値で表示
            string lifeText = to_string(fragment->life).substr(0, 3);
            DrawString(screenX, screenY - 20, lifeText.c_str(), GetColor(255, 255, 0));
#endif
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

// **新追加：衝突解決**
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

    // 最小の重なりを見つけて、その方向に押し戻す
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
        if ((velX > 0 && overlapLeft < overlapRight) || (velX < 0 && overlapLeft >= overlapRight)) {
            // プレイヤーの速度を0にする（必要に応じて）
        }
    }
    else {
        // 縦方向の衝突
        if (overlapTop < overlapBottom) {
            // 下から上へ衝突（プレイヤーがブロックの上に乗る）
            player->SetPosition(currentX, blockTop - PLAYER_HEIGHT / 2 - 1);

            // 下向きの速度をリセット（着地）
            if (velY > 0) {
                // プレイヤーを地面に着地させる
            }
        }
        else {
            // 上から下へ衝突（プレイヤーがブロックの下にぶつかる）
            player->SetPosition(currentX, blockBottom + PLAYER_HEIGHT / 2 + 1);

            // 上向きの速度をリセット
            if (velY < 0) {
                // プレイヤーの上向き速度を止める
            }
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
    auto block = std::make_unique<Block>(x, y, COIN_BLOCK);
    block->textureHandle = coinBlockActiveTexture;
    blocks.push_back(std::move(block));
}

void BlockSystem::AddBrickBlock(float x, float y)
{
    auto block = std::make_unique<Block>(x, y, BRICK_BLOCK);
    block->textureHandle = brickBlockTexture;
    blocks.push_back(std::move(block));
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
    std::vector<std::pair<float, float>> coinBlocks = {
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

// **新追加：プレイヤーとブロックの個別衝突チェック（GameScene用）**
void BlockSystem::CheckAndResolvePlayerCollisions(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();

    // 各ブロックとの衝突をチェック
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        if (CheckAABBCollision(
            playerX - PLAYER_WIDTH / 2, playerY - PLAYER_HEIGHT / 2,
            PLAYER_WIDTH, PLAYER_HEIGHT,
            block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {

            ResolveCollision(player, *block, playerX, playerY);

            // 位置が変更された可能性があるので更新
            playerX = player->GetX();
            playerY = player->GetY();
        }
    }
}