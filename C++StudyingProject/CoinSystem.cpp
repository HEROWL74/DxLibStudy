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

    std::vector<std::pair<float, float>> grassCoins = {
        // スタート地点周辺（地面レベル）
        {300, 650}, {500, 600}, {700, 650}, {900, 600},

        // 序盤プラットフォーム周辺
        {400, 450}, {800, 350}, {1200, 450}, {350, 500},

        // ギャップ1周辺（取りやすい位置）
        {1000, 600}, {1400, 550}, {1300, 500}, {1600, 450},

        // 中盤の高いプラットフォーム
        {2000, 400}, {2400, 350}, {2800, 400}, {2100, 500},

        // ギャップ2エリア
        {3200, 500}, {3600, 450}, {3100, 600}, {3800, 550},

        // 終盤エリア
        {4400, 600}, {4800, 550}, {5200, 600}, {5600, 500},
        {6000, 450}, {6400, 500}, {6800, 550},

        // チャレンジコイン（高い場所）
        {1500, 250}, {3000, 200}, {4500, 250}, {6000, 200}
    };

    for (const auto& pos : grassCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForStoneStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> stoneCoins = {
        // スタート地点周辺（安全な地面レベル）
        {300, 700}, {600, 700}, {900, 700}, {1200, 700},

        // 低い塔の周辺（取りやすい高さ）
        {500, 550}, {800, 500}, {1100, 550}, {1400, 500},

        // 安全な橋の上
        {700, 650}, {1000, 600}, {1600, 650}, {1900, 600},

        // 中間プラットフォーム上
        {1300, 550}, {1700, 500}, {2200, 550}, {2500, 500},

        // 石の通路（地面レベル）
        {2000, 700}, {2400, 700}, {2800, 700}, {3200, 700},

        // 安全な足場の上
        {2600, 600}, {3000, 550}, {3400, 600}, {3800, 550},

        // 終盤エリア（取りやすい位置）
        {4200, 700}, {4600, 650}, {5000, 700}, {5400, 650},

        // ゴール前エリア
        {5800, 700}, {6200, 650}, {6600, 700},

        // ボーナスコイン（少し高い場所だが安全）
        {1000, 400}, {2500, 350}, {4000, 400}, {5500, 350}
    };

    for (const auto& pos : stoneCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSandStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> sandCoins = {
        // オアシス周辺
        {350, 650}, {750, 600}, {1150, 650}, {550, 700},

        // 砂丘の頂上
        {500, 450}, {1300, 400}, {2100, 450}, {900, 500},

        // ピラミッドの周辺
        {2800, 500}, {3200, 450}, {3600, 400}, {2600, 550},

        // 砂漠の隠れた場所
        {1600, 550}, {2400, 500}, {4000, 550}, {1800, 600},

        // 高い砂丘
        {4400, 350}, {4800, 300}, {5200, 350}, {4600, 400},

        // 砂の橋
        {5600, 600}, {6000, 550}, {6400, 600}, {5800, 650},

        // ゴール前のオアシス
        {6800, 650}, {7200, 600}, {7000, 550},

        // 砂漠のお宝（チャレンジコイン）
        {900, 200}, {2700, 150}, {4600, 200}, {6600, 250}
    };

    for (const auto& pos : sandCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSnowStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> snowCoins = {
        // 山麓
        {400, 700}, {800, 650}, {1200, 700}, {600, 750},

        // 雪の足場
        {600, 550}, {1400, 500}, {2200, 550}, {1000, 600},

        // 氷の洞窟風
        {1000, 450}, {1800, 400}, {2600, 450}, {1300, 500},

        // 山の中腹
        {3000, 600}, {3400, 550}, {3800, 600}, {3200, 650},

        // 雪だるまの周辺
        {2400, 350}, {4200, 400}, {5000, 350}, {2600, 450},

        // 氷柱の近く
        {4600, 550}, {5400, 500}, {6200, 550}, {4800, 600},

        // 山頂付近
        {5800, 300}, {6600, 250}, {7000, 300}, {6200, 400},

        // 雪原の秘宝
        {1300, 200}, {3500, 150}, {5700, 200}, {6800, 350}
    };

    for (const auto& pos : snowCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForPurpleStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> purpleCoins = {
        // 魔法の島の周辺
        {400, 650}, {900, 600}, {1400, 550}, {650, 700},

        // 浮遊する魔法の輪
        {700, 450}, {1200, 400}, {1700, 350}, {950, 500},

        // 魔法の橋
        {2000, 500}, {2500, 450}, {3000, 500}, {2250, 550},

        // 魔法の階段周辺
        {2200, 350}, {3400, 300}, {4600, 350}, {2800, 400},

        // 高い魔法の島
        {3800, 400}, {4200, 350}, {4600, 400}, {4000, 450},

        // 魔法の通路
        {5000, 550}, {5400, 500}, {5800, 550}, {5200, 600},

        // 最終エリアの魔法
        {6200, 450}, {6600, 400}, {7000, 450}, {6400, 500},

        // 隠された魔法のコイン
        {1100, 200}, {2800, 150}, {4400, 200}, {6400, 250}
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