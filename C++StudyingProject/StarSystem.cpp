#include "StarSystem.h"
#include <math.h>
#include <algorithm>

using namespace std;

StarSystem::StarSystem()
    : starTexture(-1)
    , sparkleTexture(-1)
    , collectedStarsCount(0)
{
}

StarSystem::~StarSystem()
{
    if (starTexture != -1) DeleteGraph(starTexture);
    if (sparkleTexture != -1) DeleteGraph(sparkleTexture);
}

void StarSystem::Initialize()
{
    LoadTextures();
    collectedStarsCount = 0;
}

void StarSystem::LoadTextures()
{
    // 星テクスチャを読み込み
    starTexture = LoadGraph("UI/PNG/Yellow/star.png");
    // きらめき効果用（オプション）
    // sparkleTexture = LoadGraph("Sprites/Effects/star_sparkle.png");
}

void StarSystem::Update(Player* player)
{
    if (!player) return;

    // プレイヤーの位置を取得
    float playerX = player->GetX();
    float playerY = player->GetY();

    // 全ての星を更新（逆順でイテレート、削除対応）
    for (auto it = stars.begin(); it != stars.end();) {
        if ((*it)->active) {
            UpdateStar(**it, playerX, playerY);

            // **非アクティブになった星を完全に削除**
            if (!(*it)->active) {
                it = stars.erase(it); // イテレータを更新
            }
            else {
                ++it; // 次の要素へ
            }
        }
        else {
            // **既に非アクティブの星も削除**
            it = stars.erase(it);
        }
    }
}

void StarSystem::UpdateStar(Star& star, float playerX, float playerY)
{
    // プレイヤーと星の距離を計算
    float playerDistance = GetDistance(star.x, star.y, playerX, playerY);

    // 状態に応じた処理
    switch (star.state) {
    case STAR_IDLE:
        // 引き寄せ範囲内かチェック
        if (playerDistance <= ATTRACT_DISTANCE) {
            star.state = STAR_ATTRACTING;
            star.attractTimer = 0.0f;
        }
        break;

    case STAR_ATTRACTING:
        // 収集範囲内かチェック
        if (playerDistance <= COLLECTION_DISTANCE) {
            star.state = STAR_COLLECTED;
            star.attractTimer = 0.0f;
            star.collected = true;
            collectedStarsCount++;

            // **収集時の初期エフェクト設定**
            star.collectionPhase = 0.0f;
            star.burstIntensity = 1.0f;
            star.sparkleCount = 12; // 放射する光の数
        }
        break;

    case STAR_COLLECTED:
        // **収集アニメーション**
        star.attractTimer += 0.016f;
        star.collectionPhase += 0.016f;

        // **アニメーション完了後に確実に非アクティブ化**
        if (star.attractTimer >= SPARKLE_DURATION) {
            star.active = false; // 非アクティブ化
            star.alpha = 0;      // 完全透明
            star.scale = 0.0f;   // サイズ0
            return; // 以降の処理をスキップ
        }
        break;
    }

    // **アクティブな星のみ物理・アニメーション更新**
    if (star.active) {
        UpdateStarPhysics(star);
        UpdateStarAnimation(star);
    }
}

void StarSystem::UpdateStarPhysics(Star& star)
{
    switch (star.state) {
    case STAR_IDLE:
    case STAR_ATTRACTING:
        // 浮遊効果
        star.y = star.originalY + sinf(star.bobPhase) * BOB_AMPLITUDE;
        break;

    case STAR_COLLECTED:
        // **3回点滅アニメーション**
        float progress = star.attractTimer / SPARKLE_DURATION;

        // **最終段階での完全消失チェック**
        if (progress >= 1.0f) {
            star.active = false;
            star.alpha = 0;
            star.scale = 0.0f;
            star.burstIntensity = 0.0f;
            return;
        }

        // **点滅計算（3回）**
        float blinkPhase = progress * (DX_PI * 3.0f); // 3π ≈ 9.42 (3回の点滅)
        int blinkCount = (int)(blinkPhase / DX_PI); // 完了した点滅回数

        if (blinkCount < 3) {
            // **点滅中（0～3回）**
            star.alpha = (int)(255 * (0.3f + 0.7f * fabsf(sinf(blinkPhase))));
        }
        else {
            // **3回点滅完了後は徐々にフェード**
            float fadeProgress = (progress - 0.67f) / 0.33f; // 残り33%でフェード
            fadeProgress = max(0.0f, min(1.0f, fadeProgress)); // 0-1に制限
            star.alpha = (int)(255 * (1.0f - fadeProgress));

            // **フェード完了で即座に非アクティブ**
            if (fadeProgress >= 1.0f) {
                star.active = false;
                star.alpha = 0;
                return;
            }
        }

        // **サイズは一定（点滅のみ）**
        star.scale = 1.0f;

        // **パーティクル用の強度計算**
        star.burstIntensity = max(0.0f, 1.0f - progress);

        break;
    }
}

void StarSystem::UpdateStarAnimation(Star& star)
{
    // 浮遊アニメーション
    star.bobPhase += BOB_SPEED;
    if (star.bobPhase >= 2.0f * 3.14159265f) {
        star.bobPhase -= 2.0f * 3.14159265f;
    }

    // 回転アニメーション
    star.rotation += ROTATE_SPEED;
    if (star.rotation >= 2.0f * 3.14159265f) {
        star.rotation -= 2.0f * 3.14159265f;
    }

    // 引き寄せ時のパルス効果
    if (star.state == STAR_ATTRACTING) {
        float pulseScale = 1.0f + sinf(star.animationTimer * 6.0f) * 0.15f;
        star.scale = pulseScale;
    }

    star.animationTimer += 0.016f;
}

void StarSystem::Draw(float cameraX)
{
    if (starTexture == -1) return;

    for (const auto& star : stars) {
        // **アクティブかつ透明度がある星のみ描画**
        if (star->active && star->alpha > 0) {
            DrawStar(*star, cameraX);

            // **収集エフェクト（バースト強度がある場合のみ）**
            if (star->state == STAR_COLLECTED && star->burstIntensity > 0.0f) {
                DrawCollectionEffect(*star, cameraX);
            }
        }
    }
}

void StarSystem::DrawStar(const Star& star, float cameraX)
{
    // 画面座標に変換
    int screenX = (int)(star.x - cameraX);
    int screenY = (int)star.y;

    // 画面外なら描画しない
    if (screenX < -STAR_SIZE || screenX > 1920 + STAR_SIZE) return;

    // スケールと透明度を適用
    int starSize = (int)(STAR_SIZE * star.scale);
    int offsetX = starSize / 2;
    int offsetY = starSize / 2;

    // 透明度設定
    if (star.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, star.alpha);
    }

    // 回転を考慮した描画（簡易版）
    float rotationScale = fabsf(sinf(star.rotation)) * 0.2f + 0.8f;
    int rotatedSize = (int)(starSize * rotationScale);
    int rotateOffsetX = (starSize - rotatedSize) / 2;
    int rotateOffsetY = (starSize - rotatedSize) / 2;

    // **星描画**
    DrawExtendGraph(
        screenX - offsetX + rotateOffsetX,
        screenY - offsetY + rotateOffsetY,
        screenX - offsetX + rotateOffsetX + rotatedSize,
        screenY - offsetY + rotateOffsetY + rotatedSize,
        starTexture, TRUE
    );

    // 透明度をリセット
    if (star.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void StarSystem::DrawCollectionEffect(const Star& star, float cameraX)
{
    int screenX = (int)(star.x - cameraX);
    int screenY = (int)star.y;

    DrawCollectionBurst(star, screenX, screenY, cameraX);
}

// **新機能：3回点滅とDrawCircleパーティクル**
void StarSystem::DrawCollectionBurst(const Star& star, int screenX, int screenY, float cameraX)
{
    if (star.burstIntensity <= 0.0f) return;

    float progress = star.collectionPhase / SPARKLE_DURATION;

    // **1. 中央の光る円（脈動効果）**
    float pulseScale = 1.0f + sinf(progress * 20.0f) * 0.3f;
    int centralSize = (int)(25.0f * pulseScale * star.burstIntensity);

    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(150 * star.burstIntensity));
    DrawCircle(screenX, screenY, centralSize, GetColor(255, 255, 100), TRUE);

    // **2. 外側に広がる円（リング効果）**
    for (int ring = 0; ring < 4; ring++) {
        float ringProgress = progress - ring * 0.1f; // 時間差で広がる
        if (ringProgress > 0.0f) {
            float ringRadius = ringProgress * 80.0f + ring * 15.0f;
            int ringAlpha = (int)(80 * star.burstIntensity / (ring + 1));

            SetDrawBlendMode(DX_BLENDMODE_ADD, ringAlpha);
            DrawCircle(screenX, screenY, (int)ringRadius,
                GetColor(255, 220 - ring * 30, 50 + ring * 40), FALSE);
        }
    }

    // **3. 放射状に飛び散るパーティクル（円）**
    int particleCount = 8;
    for (int i = 0; i < particleCount; i++) {
        float angle = (float)i * (6.28318f / particleCount) + progress * 3.0f;
        float distance = progress * 70.0f + 15.0f;

        int particleX = screenX + (int)(cosf(angle) * distance);
        int particleY = screenY + (int)(sinf(angle) * distance);

        // パーティクルサイズ（時間経過で小さくなる）
        int particleSize = (int)(8 * star.burstIntensity);

        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(120 * star.burstIntensity));
        DrawCircle(particleX, particleY, particleSize, GetColor(255, 255, 200), TRUE);
    }

    // **4. ランダムな小さなキラキラ（円）**
    if (progress < 0.7f) {
        for (int s = 0; s < 12; s++) {
            // 疑似ランダム計算（sin/cosを使用）
            float randomAngle = sinf((float)s * 2.3f + progress * 5.0f) * 6.28318f;
            float randomDistance = (cosf((float)s * 1.7f + progress * 3.0f) * 0.5f + 0.5f) * 50.0f + 20.0f;

            int sparkleX = screenX + (int)(cosf(randomAngle) * randomDistance);
            int sparkleY = screenY + (int)(sinf(randomAngle) * randomDistance);

            // 小さなキラキラサイズ
            int sparkleSize = (int)(3 + sinf((float)s + progress * 8.0f) * 2.0f);

            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * star.burstIntensity));
            DrawCircle(sparkleX, sparkleY, sparkleSize, GetColor(255, 255, 150), TRUE);
        }
    }

    // **5. 収束する光の粒子**
    if (progress > 0.3f) {
        int convergenceCount = 6;
        for (int c = 0; c < convergenceCount; c++) {
            float convergenceProgress = (progress - 0.3f) / 0.7f;
            float startDistance = 60.0f;
            float currentDistance = startDistance * (1.0f - convergenceProgress);

            float angle = (float)c * (6.28318f / convergenceCount) + progress * 2.0f;
            int convX = screenX + (int)(cosf(angle) * currentDistance);
            int convY = screenY + (int)(sinf(angle) * currentDistance);

            int convSize = (int)(5 * (1.0f - convergenceProgress));

            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(80 * star.burstIntensity));
            DrawCircle(convX, convY, convSize, GetColor(255, 200, 100), TRUE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void StarSystem::AddStar(float x, float y)
{
    stars.push_back(std::make_unique<Star>(x, y));
}

void StarSystem::ClearAllStars()
{
    stars.clear();
}

void StarSystem::GenerateStarsForStageIndex(int stageIndex)
{
    switch (stageIndex) {
    case 0: GenerateStarsForGrassStage(); break;
    case 1: GenerateStarsForStoneStage(); break;
    case 2: GenerateStarsForSandStage(); break;
    case 3: GenerateStarsForSnowStage(); break;
    case 4: GenerateStarsForPurpleStage(); break;
    default: GenerateStarsForGrassStage(); break;
    }
}

void StarSystem::GenerateStarsForGrassStage()
{
    ClearAllStars();

    // 草原ステージ：3つの星を配置（難易度順）
    AddStar(1500, 300);  // 星1：中程度の高さ
    AddStar(3500, 200);  // 星2：高い場所
    AddStar(5500, 250);  // 星3：最終エリア
}

void StarSystem::GenerateStarsForStoneStage()
{
    ClearAllStars();

    // 石ステージ：岩の隙間に配置
    AddStar(1000, 250);  // 星1：左の隙間
    AddStar(2500, 200);  // 星2：中央の高い場所
    AddStar(4500, 300);  // 星3：右の隙間
}

void StarSystem::GenerateStarsForSandStage()
{
    ClearAllStars();

    // 砂漠ステージ：ピラミッドとオアシス
    AddStar(1200, 250);  // 星1：最初の砂丘
    AddStar(2700, 150);  // 星2：ピラミッドの頂上
    AddStar(4800, 200);  // 星3：高い砂丘
}

void StarSystem::GenerateStarsForSnowStage()
{
    ClearAllStars();

    // 雪山ステージ：山の頂上付近
    AddStar(1500, 200);  // 星1：山の中腹
    AddStar(3000, 150);  // 星2：山頂
    AddStar(5200, 180);  // 星3：雪原の高台
}

void StarSystem::GenerateStarsForPurpleStage()
{
    ClearAllStars();

    // 魔法ステージ：浮遊する高い場所
    AddStar(1300, 150);  // 星1：魔法の島
    AddStar(3200, 100);  // 星2：最高の浮遊島
    AddStar(5800, 120);  // 星3：魔法の隙間
}

float StarSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}