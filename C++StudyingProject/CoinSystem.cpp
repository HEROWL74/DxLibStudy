#include "CoinSystem.h"
#include <math.h>
#include <algorithm>
#include "SoundManager.h"

using namespace std;

CoinSystem::CoinSystem()
    : coinTexture(-1)
    , sparkleTexture(-1)
    , collectedCoinsCount(0)
{
}

CoinSystem::~CoinSystem()
{
    if (coinTexture != -1) DeleteGraph(coinTexture);
    if (sparkleTexture != -1) DeleteGraph(sparkleTexture);
}

void CoinSystem::Initialize()
{
    LoadTextures();
    collectedCoinsCount = 0;
}

void CoinSystem::LoadTextures()
{
    // コインテクスチャを読み込み
    coinTexture = LoadGraph("Sprites/Tiles/hud_coin.png");

    // きらめき効果用（存在する場合）
    // sparkleTexture = LoadGraph("Sprites/Effects/sparkle.png");
}

void CoinSystem::Update(Player* player, float hudCoinX, float hudCoinY)
{
    if (!player) return;

    // プレイヤーの位置を取得
    float playerX = player->GetX();
    float playerY = player->GetY();

    // 全てのコインを更新
    for (auto it = coins.begin(); it != coins.end();) {
        if ((*it)->active) {
            UpdateCoin(**it, playerX, playerY, hudCoinX, hudCoinY);
            ++it;
        }
        else {
            // 非アクティブなコインを削除
            it = coins.erase(it);
        }
    }
}

void CoinSystem::UpdateCoin(Coin& coin, float playerX, float playerY, float hudCoinX, float hudCoinY)
{
    // プレイヤーとコインの距離を計算（判定用）
    float playerDistance = GetDistance(coin.x, coin.y, playerX, playerY);

    // 状態に応じた処理
    switch (coin.state) {
    case COIN_IDLE:
        // 引き寄せ範囲内かチェック（プレイヤーとの距離で判定）
        if (playerDistance <= ATTRACT_DISTANCE) {
            coin.state = COIN_ATTRACTING;
            coin.attractTimer = 0.0f;
        }
        break;

    case COIN_ATTRACTING:
        // 収集範囲内かチェック（プレイヤーとの距離で判定）
        if (playerDistance <= COLLECTION_DISTANCE) {
            coin.state = COIN_COLLECTED;
            coin.attractTimer = 0.0f;
          
            SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);;
            collectedCoinsCount++;
        }
        break;

    case COIN_COLLECTED:
        // 収集アニメーション（ゆっくりと消え去りをかけて）
        coin.attractTimer += 0.016f; // 60FPS想定

        // HUDに到達したかチェック（距離が十分近くなったら削除）
        float hudDistance = GetDistance(coin.x, coin.y, hudCoinX, hudCoinY);

        if (coin.attractTimer >= 1.2f || hudDistance <= 20.0f) { // 1.2秒経過または20px以内に到達
            coin.active = false;
        }
        break;
    }

    // 物理更新
    UpdateCoinPhysics(coin, hudCoinX, hudCoinY);

    // アニメーション更新
    UpdateCoinAnimation(coin);
}

void CoinSystem::UpdateCoinPhysics(Coin& coin, float hudCoinX, float hudCoinY)
{
    switch (coin.state) {
    case COIN_IDLE:
        // 通常の浮遊動作
        coin.y = coin.originalY + sinf(coin.bobPhase) * BOB_AMPLITUDE;
        break;

    case COIN_COLLECTED:
    {
        // 収集後の演出（HUDのコインアイコンに向かって加速移動）
        float progress = coin.attractTimer / 1.2f; // 1.2秒でゆっくり完了
        float dirX = hudCoinX - coin.x;
        float dirY = hudCoinY - coin.y;
        float distance = sqrtf(dirX * dirX + dirY * dirY);

        if (distance > 0.1f) {
            // 正規化されたベクトル
            dirX /= distance;
            dirY /= distance;

            // 加速的に移動（HUDに近づくほど速く）
            float speed = 15.0f * (1.0f + progress * 3.0f);

            coin.x += dirX * speed;
            coin.y += dirY * speed;
        }

        // フェードアウトと拡大効果
        coin.alpha = (int)(255 * (1.0f - progress * 0.9f)); // よりキビキビとフェードアウト
        coin.scale = 1.0f + progress * 0.3f; // ゆっくりと拡大
    }
    break;
    }
}

void CoinSystem::UpdateCoinAnimation(Coin& coin)
{
    // 浮遊アニメーション
    coin.bobPhase += BOB_SPEED;
    if (coin.bobPhase >= 2.0f * 3.14159265f) {
        coin.bobPhase -= 2.0f * 3.14159265f;
    }

    // 回転アニメーション
    coin.animationTimer += ROTATE_SPEED;
    if (coin.animationTimer >= 2.0f * 3.14159265f) {
        coin.animationTimer -= 2.0f * 3.14159265f;
    }

    // 引き寄せ時のスケールアニメーション
    if (coin.state == COIN_ATTRACTING) {
        float pulseScale = 1.0f + sinf(coin.animationTimer * 4.0f) * 0.1f;
        coin.scale = pulseScale;
    }
}

void CoinSystem::Draw(float cameraX)
{
    if (coinTexture == -1) return;

    for (const auto& coin : coins) {
        if (coin->active) {
            DrawCoin(*coin, cameraX);

            // 収集エフェクト
            if (coin->state == COIN_COLLECTED) {
                DrawCollectionEffect(*coin, cameraX);
            }
        }
    }
}

void CoinSystem::DrawCoin(const Coin& coin, float cameraX)
{
    // 画面座標に変換
    int screenX = (int)(coin.x - cameraX);
    int screenY = (int)coin.y;

    // 画面外なら描画しない（最適化）
    if (screenX < -COIN_SIZE || screenX > 1920 + COIN_SIZE) return;

    // スケールと透明度を適用
    int coinSize = (int)(COIN_SIZE * coin.scale);
    int offsetX = coinSize / 2;
    int offsetY = coinSize / 2;

    // 透明度設定
    if (coin.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, coin.alpha);
    }

    // 回転効果（簡易版：横方向のスケール変更で回転を表現）
    float rotationScale = fabsf(cosf(coin.animationTimer));
    int rotatedWidth = (int)(coinSize * (0.3f + rotationScale * 0.7f));

    // **コイン描画（64pxに拡大）**
    DrawExtendGraph(
        screenX - offsetX, screenY - offsetY,
        screenX - offsetX + rotatedWidth, screenY - offsetY + coinSize,
        coinTexture, TRUE
    );

    // **デバッグ用：コインの当たり判定範囲とHUDターゲットを表示**
#ifdef _DEBUG
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    DrawCircle(screenX, screenY, (int)COLLECTION_DISTANCE, GetColor(0, 255, 0), FALSE); // 収集範囲（緑）
    DrawCircle(screenX, screenY, (int)ATTRACT_DISTANCE, GetColor(255, 255, 0), FALSE);  // 引き寄せ範囲（黄）

    // HUDターゲット位置を表示（赤い×印） - 固定されたスクリーン座標
    int hudTargetScreenX = 30 + 80 + 20 + 48 / 2;
    int hudTargetScreenY = 30 + 64 + 20 + 48 / 2;
    DrawLine(hudTargetScreenX - 10, hudTargetScreenY - 10, hudTargetScreenX + 10, hudTargetScreenY + 10, GetColor(255, 0, 0));
    DrawLine(hudTargetScreenX - 10, hudTargetScreenY + 10, hudTargetScreenX + 10, hudTargetScreenY - 10, GetColor(255, 0, 0));

    // HUDまでの距離を表示
    if (coin.state == COIN_COLLECTED) {
        float hudWorldX = hudTargetScreenX + cameraX;
        float hudWorldY = hudTargetScreenY;
        float hudDistance = GetDistance(coin.x, coin.y, hudWorldX, hudWorldY);

        string distanceText = "Dist: " + to_string((int)hudDistance);
        DrawString(screenX - 30, screenY - 60, distanceText.c_str(), GetColor(255, 255, 0));
    }

    // ワールド座標での方向ベクトルを表示
    if (coin.state == COIN_ATTRACTING || coin.state == COIN_COLLECTED) {
        float hudWorldX = hudTargetScreenX + cameraX;
        float hudWorldY = hudTargetScreenY;
        float dirX = hudWorldX - coin.x;
        float dirY = hudWorldY - coin.y;

        // 方向ベクトルを矢印で表示
        int endX = screenX + (int)(dirX * 0.1f);
        int endY = screenY + (int)(dirY * 0.1f);
        DrawLine(screenX, screenY, endX, endY, GetColor(255, 0, 255));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif

    // 透明度をリセット
    if (coin.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void CoinSystem::DrawCollectionEffect(const Coin& coin, float cameraX)
{
    int screenX = (int)(coin.x - cameraX);
    int screenY = (int)coin.y;

    // きらめき効果（ゆっくりとした演出）
    float progress = coin.attractTimer / 1.2f; // 1.2秒でゆっくり
    int sparkleSize = (int)(25 + progress * 35); // ゆっくりと大きくなる
    int sparkleAlpha = (int)(160 * (1.0f - progress));

    if (sparkleAlpha > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, sparkleAlpha);

        // 複数の円できらめき効果（ゆっくりと回転）
        for (int i = 0; i < 4; i++) {
            float angle = progress * 4.0f + i * 1.57f; // ゆっくり回転
            int sparkleX = screenX + (int)(cosf(angle) * 18);
            int sparkleY = screenY + (int)(sinf(angle) * 18);

            DrawCircle(sparkleX, sparkleY, sparkleSize / (i + 1),
                GetColor(255, 255, 150), TRUE);
        }

        // 中央に追加のきらめき
        DrawCircle(screenX, screenY, sparkleSize / 2,
            GetColor(255, 215, 0), TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void CoinSystem::AddCoin(float x, float y)
{
    coins.push_back(std::make_unique<Coin>(x, y));
}

void CoinSystem::ClearAllCoins()
{
    coins.clear();
}

void CoinSystem::GenerateCoinsForStageIndex(int stageIndex)
{
    switch (stageIndex) {
    case 0: // Grass Stage
        GenerateCoinsForGrassStage();
        break;
    case 1: // Stone Stage
        GenerateCoinsForStoneStage();
        break;
    case 2: // Sand Stage
        GenerateCoinsForSandStage();
        break;
    case 3: // Snow Stage
        GenerateCoinsForSnowStage();
        break;
    case 4: // Purple Stage
        GenerateCoinsForPurpleStage();
        break;
    default:
        GenerateCoinsForGrassStage(); // デフォルト
        break;
    }
}

void CoinSystem::GenerateCoinsForStage()
{
    // デフォルト配置（後方互換性のため）
    GenerateCoinsForGrassStage();
}

void CoinSystem::GenerateCoinsForGrassStage()
{
    ClearAllCoins();

    // **修正: ステージの地形やブロックと被らない位置に配置**
    std::vector<std::pair<float, float>> grassCoins = {
        // 空中の浮遊コイン（地形ブロックを避けた配置）
        {400, 400}, {600, 350}, {900, 300},      // 初期エリア上空
        {1300, 250}, {1700, 200}, {2100, 300},  // 中間エリア上空
        {2800, 250}, {3200, 180}, {3600, 220},  // 高難度エリア
        {4200, 300}, {4800, 200}, {5200, 250},  // 終盤エリア
        {5800, 180}, {6200, 220}, {6600, 160},  // ゴール前エリア

        // チャレンジコイン（非常に高い位置）
        {1500, 150}, {3000, 120}, {4500, 140}, {6000, 100}
    };

    for (const auto& pos : grassCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForStoneStage()
{
    ClearAllCoins();

    // **修正: 石ステージの地形を避けた配置**
    std::vector<std::pair<float, float>> stoneCoins = {
        // 岩の隙間の安全な場所
        {500, 380}, {800, 320}, {1100, 280},    // 初期エリア
        {1600, 250}, {2000, 200}, {2400, 300},  // 中間の岩場
        {3000, 220}, {3500, 160}, {3900, 240},  // 高い岩場
        {4400, 280}, {4900, 200}, {5300, 180},  // 石の道
        {5800, 300}, {6200, 150}, {6600, 200},  // 終盤エリア

        // ボーナスコイン（高所）
        {1200, 150}, {2800, 100}, {4200, 120}, {5600, 90}
    };

    for (const auto& pos : stoneCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSandStage()
{
    ClearAllCoins();

    // **修正: 砂漠ステージのピラミッドや砂丘を避けた配置**
    std::vector<std::pair<float, float>> sandCoins = {
        // オアシス周辺とピラミッド隙間
        {400, 350}, {700, 300}, {1000, 250},    // オアシス周辺
        {1400, 280}, {1800, 200}, {2200, 320},  // 砂丘の間
        {2700, 180}, {3100, 150}, {3500, 220},  // ピラミッド周辺
        {4000, 260}, {4400, 180}, {4800, 300},  // 砂の谷
        {5300, 200}, {5700, 150}, {6100, 240},  // 終盤の砂丘

        // 砂漠の宝（隠されたコイン）
        {1100, 120}, {2900, 80}, {4600, 100}, {6400, 60}
    };

    for (const auto& pos : sandCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSnowStage()
{
    ClearAllCoins();

    // **修正: 雪山の地形を避けた安全な配置**
    std::vector<std::pair<float, float>> snowCoins = {
        // 雪原と氷の足場の間
        {500, 380}, {900, 320}, {1300, 260},    // 山麓
        {1800, 280}, {2200, 200}, {2600, 240},  // 雪の足場周辺
        {3200, 180}, {3600, 140}, {4000, 220},  // 山頂付近
        {4500, 260}, {4900, 180}, {5300, 300},  // 雪だるま周辺
        {5800, 160}, {6200, 200}, {6600, 120},  // 雪原の秘密

        // 氷河の宝
        {1400, 100}, {3400, 60}, {5100, 80}, {6800, 40}
    };

    for (const auto& pos : snowCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForPurpleStage()
{
    ClearAllCoins();

    // **修正: 魔法ステージの浮遊島を避けた魔法的配置**
    std::vector<std::pair<float, float>> purpleCoins = {
        // 浮遊する魔法のコイン
        {500, 320}, {900, 280}, {1300, 220},    // 魔法の島周辺
        {1800, 200}, {2200, 160}, {2600, 260},  // 魔法の橋付近
        {3200, 180}, {3600, 120}, {4000, 240},  // 高い魔法の島
        {4500, 200}, {4900, 140}, {5300, 280},  // 魔法の道
        {5800, 160}, {6200, 100}, {6600, 180},  // 最終エリア

        // 隠された魔法のコイン
        {1200, 80}, {3000, 40}, {4800, 60}, {6400, 20}
    };

    for (const auto& pos : purpleCoins) {
        AddCoin(pos.first, pos.second);
    }
}

float CoinSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

float CoinSystem::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}