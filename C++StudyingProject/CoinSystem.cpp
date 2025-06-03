#include "CoinSystem.h"
#include <math.h>
#include <algorithm>

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
        // 吸い寄せ範囲内かチェック（プレイヤーとの距離で判定）
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
            collectedCoinsCount++;
        }
        break;

    case COIN_COLLECTED:
        // 収集アニメーション（ゆっくり時間をかけて）
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
        // 収集時の演出（HUDのコインアイコンに向かって加速移動）
        float progress = coin.attractTimer / 1.2f; // 1.2秒でゆっくり完了
        float dirX = hudCoinX - coin.x;
        float dirY = hudCoinY - coin.y;
        float distance = sqrtf(dirX * dirX + dirY * dirY);

        if (distance > 0.1f) {
            // 正規化された方向ベクトル
            dirX /= distance;
            dirY /= distance;

            // 加速的に移動（HUDに近づくほど速く）
            float speed = 15.0f * (1.0f + progress * 3.0f);

            coin.x += dirX * speed;
            coin.y += dirY * speed;
        }

        // フェードアウトと拡大効果
        coin.alpha = (int)(255 * (1.0f - progress * 0.9f)); // より強くフェードアウト
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

    // 吸い寄せ時のスケールアニメーション
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
    DrawCircle(screenX, screenY, (int)ATTRACT_DISTANCE, GetColor(255, 255, 0), FALSE);  // 吸い寄せ範囲（黄）

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

        // 方向ベクトルを線で表示
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

void CoinSystem::GenerateCoinsForStage()
{
    // ステージに適当にコインを配置（拡張されたステージ用）
    ClearAllCoins();

    // 基本的な配置パターン（プレイヤーが通りやすい位置に配置）
    std::vector<std::pair<float, float>> coinPositions = {
        // 序盤エリア（地面近くで取りやすく）
        {400, 700}, {600, 650}, {800, 700}, {1000, 650}, {1200, 700},

        // 中盤エリア1
        {1400, 650}, {1600, 600}, {1800, 650}, {2000, 600}, {2200, 650},

        // 中盤エリア2
        {2400, 700}, {2600, 650}, {2800, 700}, {3000, 650}, {3200, 700},

        // 中盤エリア3
        {3400, 650}, {3600, 600}, {3800, 650}, {4000, 600}, {4200, 650},

        // 終盤エリア1
        {4400, 700}, {4600, 650}, {4800, 700}, {5000, 650}, {5200, 700},

        // 終盤エリア2
        {5400, 650}, {5600, 600}, {5800, 650}, {6000, 600}, {6200, 650},

        // 最終エリア
        {6400, 700}, {6600, 650}, {6800, 700}, {7000, 650}, {7200, 700},

        // プラットフォーム上（少し高め）
        {1000, 500}, {1800, 450}, {2600, 500}, {3400, 450}, {4200, 500},
        {5000, 450}, {5800, 500}, {6600, 450},

        // ボーナスコイン（高い場所、チャレンジ用）
        {900, 400}, {1500, 300}, {2100, 350}, {2700, 300}, {3300, 350},
        {3900, 300}, {4500, 350}, {5100, 300}, {5700, 350}, {6300, 300},

        // 発見しやすい位置
        {500, 600}, {1100, 550}, {1700, 600}, {2300, 550}, {2900, 600},
        {3500, 550}, {4100, 600}, {4700, 550}, {5300, 600}, {5900, 550},
        {6500, 600}
    };

    // コインを配置
    for (const auto& pos : coinPositions) {
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