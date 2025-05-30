#include "EffectManager.h"

// 静的メンバの定義
const int EffectManager::PARTICLE_COLORS[] = {
    GetColor(255, 215, 0),  // ゴールド
    GetColor(255, 255, 255), // ホワイト
    GetColor(255, 192, 203), // ピンク
    GetColor(135, 206, 250), // ライトブルー
    GetColor(255, 165, 0),   // オレンジ
    GetColor(50, 205, 50),   // ライムグリーン
};

const int EffectManager::PARTICLE_COLOR_COUNT = sizeof(PARTICLE_COLORS) / sizeof(PARTICLE_COLORS[0]);

EffectManager::EffectManager()
{
    frameCounter = 0;
    particleSpawnTimer = 0;
}

EffectManager::~EffectManager()
{
    Finalize();
}

void EffectManager::Initialize()
{
    particles.clear();
    stars.clear();
    frameCounter = 0;
    particleSpawnTimer = 0;
}

void EffectManager::Update()
{
    frameCounter++;
    particleSpawnTimer++;

    // パーティクルの更新
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->x += it->vx;
        it->y += it->vy;
        it->life -= 1.0f;
        it->alpha = it->life / it->maxLife;

        // 重力効果
        it->vy += 0.05f;

        // 寿命が尽きたパーティクルを削除
        if (it->life <= 0)
        {
            it = particles.erase(it);
        }
        else
        {
            ++it;
        }
    }

    // 星エフェクトの更新
    for (auto& star : stars)
    {
        star.rotation += star.rotSpeed;
        star.pulsePhase += star.pulseSpeed;
        star.alpha = 0.5f + 0.5f * sin(star.pulsePhase);
    }

    // 定期的にパーティクルを生成
    if (particleSpawnTimer > 30)  // 0.5秒間隔
    {
        CreateRandomParticles();
        particleSpawnTimer = 0;
    }
}

void EffectManager::Draw()
{
    DrawTitleGlow();
    DrawStars();
    DrawParticles();
}

void EffectManager::StartTitleEffects()
{
    CreateStarEffects();
    CreateRandomParticles();
}

void EffectManager::CreateParticle(float x, float y, float vx, float vy, int color, float life)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.vx = vx;
    p.vy = vy;
    p.life = life;
    p.maxLife = life;
    p.color = color;
    p.size = 2.0f + (rand() % 4);
    p.alpha = 1.0f;

    particles.push_back(p);
}

void EffectManager::CreateRandomParticles()
{
    // 画面上部からランダムにパーティクルを生成
    for (int i = 0; i < 5; i++)
    {
        float x = rand() % 1280;
        float y = -10;
        float vx = -2 + (rand() % 5);
        float vy = 1 + (rand() % 3);
        int colorIndex = rand() % PARTICLE_COLOR_COUNT;
        float life = 120 + (rand() % 120);  // 2-4秒

        CreateParticle(x, y, vx, vy, PARTICLE_COLORS[colorIndex], life);
    }
}

void EffectManager::CreateStarEffects()
{
    stars.clear();

    // 画面にランダムに星を配置
    for (int i = 0; i < 20; i++)
    {
        StarEffect star;
        star.x = rand() % 1280;
        star.y = rand() % 720;
        star.scale = 0.5f + (rand() % 100) / 100.0f;
        star.rotSpeed = -0.02f + (rand() % 100) / 2500.0f;
        star.pulseSpeed = 0.05f + (rand() % 100) / 2000.0f;
        star.pulsePhase = (rand() % 628) / 100.0f;  // 0-2π

        stars.push_back(star);
    }
}

void EffectManager::DrawParticles()
{
    for (const auto& p : particles)
    {
        // アルファ値を適用した色で描画
        int alpha = (int)(p.alpha * 255);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        DrawCircle((int)p.x, (int)p.y, (int)p.size, p.color, TRUE);

        // 光る効果
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 2);
        DrawCircle((int)p.x, (int)p.y, (int)(p.size * 1.5f), p.color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::DrawStars()
{
    for (const auto& star : stars)
    {
        int alpha = (int)(star.alpha * 180);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // 星の描画（十字形）
        int size = (int)(10 * star.scale);
        int color = GetColor(255, 255, 255);

        // 回転を考慮した十字の描画
        float cos_r = cos(star.rotation);
        float sin_r = sin(star.rotation);

        int x1 = (int)(star.x + size * cos_r);
        int y1 = (int)(star.y + size * sin_r);
        int x2 = (int)(star.x - size * cos_r);
        int y2 = (int)(star.y - size * sin_r);

        int x3 = (int)(star.x + size * sin_r);
        int y3 = (int)(star.y - size * cos_r);
        int x4 = (int)(star.x - size * sin_r);
        int y4 = (int)(star.y + size * cos_r);

        DrawLine(x1, y1, x2, y2, color, 2);
        DrawLine(x3, y3, x4, y4, color, 2);

        // 中心に小さな円
        DrawCircle((int)star.x, (int)star.y, 2, color, TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::DrawTitleGlow()
{
    // タイトル領域に光る効果
    int alpha = (int)(100 + 50 * sin(frameCounter * 0.05f));
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // グラデーション円を描画
    for (int i = 0; i < 5; i++)
    {
        int radius = 150 + i * 30;
        int centerX = 640;
        int centerY = 200;

        DrawCircle(centerX, centerY, radius, GetColor(100, 150, 255), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void EffectManager::Finalize()
{
    particles.clear();
    stars.clear();
}