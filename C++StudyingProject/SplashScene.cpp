#include "SplashScene.h"
#include <math.h>
#include <random>

SplashScene::SplashScene()
    : currentPhase(PHASE_LOGO_FADEIN)
    , phaseTimer(0.0f)
    , logoHandle(-1)
    , bombHandle(-1)
    , bombActiveHandle(-1)
    , fontHandle(-1)
    , logoAlpha(0.0f)
    , logoScale(1.0f)
    , explosionTimer(0.0f)
    , explosionStarted(false)
    , bombAnimFrame(0)
    , slideOffset(0.0f)
    , transitionComplete(false)
{
}

SplashScene::~SplashScene()
{
    Finalize();
}

void SplashScene::Initialize()
{
    // リソース読み込み
    bombHandle = LoadGraph("Sprites/Tiles/bomb.png");
    bombActiveHandle = LoadGraph("Sprites/Tiles/bomb_active.png");

    // ロゴは適当な画像を使用（今ないので、テキストで代用中）
    // logoHandle = LoadGraph("Sprites/studio_logo.png");

    // フォント作成
    fontHandle = CreateFontToHandle(NULL, 48, 3);

    // 初期状態設定
    currentPhase = PHASE_LOGO_FADEIN;
    phaseTimer = 0.0f;
    logoAlpha = 0.0f;
    logoScale = 0.8f;
    explosionTimer = 0.0f;
    explosionStarted = false;
    bombAnimFrame = 0;
    slideOffset = 0.0f;
    transitionComplete = false;

    // 爆発パーティクル配列をクリア
    explosionParticles.clear();

    OutputDebugStringA("SplashScene: Initialized\n");
}

void SplashScene::Update()
{
    phaseTimer += 0.016f; // 60FPS想定

    switch (currentPhase) {
    case PHASE_LOGO_FADEIN:
        UpdateLogoFadeIn();
        break;
    case PHASE_LOGO_DISPLAY:
        UpdateLogoDisplay();
        break;
    case PHASE_EXPLOSION_PREP:
        UpdateExplosionPrep();
        break;
    case PHASE_EXPLOSION:
        UpdateExplosion();
        break;
    case PHASE_SLIDE_UP:
        UpdateSlideUp();
        break;
    case PHASE_COMPLETE:
        transitionComplete = true;
        break;
    }
}

void SplashScene::UpdateLogoFadeIn()
{
    // ロゴのフェードイン
    logoAlpha = phaseTimer / LOGO_FADEIN_DURATION;
    logoScale = 0.8f + (0.2f * logoAlpha); // 0.8 -> 1.0にスケール

    if (logoAlpha >= 1.0f) {
        logoAlpha = 1.0f;
        logoScale = 1.0f;
        currentPhase = PHASE_LOGO_DISPLAY;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Logo fade-in complete\n");
    }
}

void SplashScene::UpdateLogoDisplay()
{
    // ロゴ表示中（静止）
    if (phaseTimer >= LOGO_DISPLAY_DURATION) {
        currentPhase = PHASE_EXPLOSION_PREP;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Logo display complete, preparing explosion\n");
    }
}

void SplashScene::UpdateExplosionPrep()
{
    // 爆発準備（ボムの点滅など）
    bombAnimFrame = (int)(phaseTimer * 8.0f) % 2; // 8Hzで点滅

    if (phaseTimer >= EXPLOSION_PREP_DURATION) {
        currentPhase = PHASE_EXPLOSION;
        phaseTimer = 0.0f;
        InitializeExplosion();

       
        OutputDebugStringA("SplashScene: Explosion started!\n");
    }
}

void SplashScene::UpdateExplosion()
{
    explosionTimer = phaseTimer;

    // パーティクル更新
    UpdateExplosionParticles();

    if (phaseTimer >= EXPLOSION_DURATION) {
        currentPhase = PHASE_SLIDE_UP;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Explosion complete, starting slide transition\n");
    }
}

void SplashScene::UpdateSlideUp()
{
    // スライドアップトランジション
    slideOffset = (phaseTimer / SLIDE_UP_DURATION) * SCREEN_H;

    if (slideOffset >= SCREEN_H) {
        slideOffset = SCREEN_H;
        currentPhase = PHASE_COMPLETE;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Slide-up complete\n");
    }
}

void SplashScene::Draw()
{
    // 背景を黒で塗りつぶし
    DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);

    switch (currentPhase) {
    case PHASE_LOGO_FADEIN:
    case PHASE_LOGO_DISPLAY:
        DrawLogo();
        break;
    case PHASE_EXPLOSION_PREP:
        DrawLogo();
        // ボムを描画（点滅）
        if (bombAnimFrame == 0 && bombHandle != -1) {
            DrawExtendGraph(SCREEN_W / 2 - 32, SCREEN_H / 2 + 100,
                SCREEN_W / 2 + 32, SCREEN_H / 2 + 164,
                bombHandle, TRUE);
        }
        else if (bombActiveHandle != -1) {
            DrawExtendGraph(SCREEN_W / 2 - 32, SCREEN_H / 2 + 100,
                SCREEN_W / 2 + 32, SCREEN_H / 2 + 164,
                bombActiveHandle, TRUE);
        }
        break;
    case PHASE_EXPLOSION:
        DrawExplosion();
        break;
    case PHASE_SLIDE_UP:
        DrawSlideTransition();
        break;
    case PHASE_COMPLETE:
        // 完全に隠れた状態
        break;
    }
}

void SplashScene::DrawLogo()
{
    // ロゴ描画（画像がない場合はテキストで代用）
    if (logoHandle != -1) {
        // 画像でのロゴ描画
        int logoW, logoH;
        GetGraphSize(logoHandle, &logoW, &logoH);

        int centerX = SCREEN_W / 2;
        int centerY = SCREEN_H / 2 - 50;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * logoAlpha));
        DrawExtendGraph(
            centerX - (int)(logoW * logoScale / 2),
            centerY - (int)(logoH * logoScale / 2),
            centerX + (int)(logoW * logoScale / 2),
            centerY + (int)(logoH * logoScale / 2),
            logoHandle, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else {
        // テキストでのロゴ描画
        std::string logoText = "HEROWL";
        std::string subText = "Game Studio";

        // 文字幅を正確に計算
        int logoWidth = GetDrawStringWidthToHandle(logoText.c_str(), (int)logoText.length(), fontHandle);
        int subWidth = GetDrawStringWidthToHandle(subText.c_str(), (int)subText.length(), fontHandle);

        // 画面中央に正確に配置
        int centerX = SCREEN_W / 2;
        int centerY = SCREEN_H / 2 - 50;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * logoAlpha));

        // メインロゴ（中央揃え）
        int logoX = centerX - logoWidth;
        int logoY = centerY - 40;
        DrawStringToHandle(logoX, logoY, logoText.c_str(), GetColor(255, 255, 255), fontHandle);

        // サブテキスト（中央揃え）
        int subX = centerX - subWidth  + 47;
        int subY = centerY + 30;
        DrawStringToHandle(subX, subY, subText.c_str(), GetColor(180, 180, 180), fontHandle);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void SplashScene::DrawExplosion()
{
    // より強力な白いフラッシュ効果
    float flashAlpha = 0.0f;
    if (explosionTimer < 0.3f) {
        flashAlpha = 1.0f - (explosionTimer / 0.3f);
    }

    if (flashAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * flashAlpha));
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 背景に放射状のグラデーション効果
    if (explosionTimer < 1.0f) {
        float bgAlpha = (1.0f - explosionTimer) * 0.3f;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * bgAlpha));

        // 中心から放射状に色を変化
        for (int r = 0; r < 400; r += 20) {
            int alpha = (int)(255 * bgAlpha * (1.0f - r / 400.0f));
            if (alpha > 0) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                DrawCircle(SCREEN_W / 2, SCREEN_H / 2, r, GetColor(255, 100 + r / 4, 0), FALSE);
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // パーティクル描画（より豪華に）
    for (const auto& particle : explosionParticles) {
        if (!particle.active) continue;

        float alpha = particle.life / particle.maxLife;
        int size = (int)(particle.size * alpha);

        if (size > 0) {
            // メインパーティクル
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
            DrawCircle((int)particle.x, (int)particle.y, size, particle.color, TRUE);

            // 光る効果（加算合成）
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(128 * alpha));
            DrawCircle((int)particle.x, (int)particle.y, size / 2, GetColor(255, 255, 255), TRUE);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // 中心部の強い光（爆発の核）
    if (explosionTimer < 0.8f) {
        float coreAlpha = (0.8f - explosionTimer) / 0.8f;
        float coreSize = 100.0f * (1.0f - coreAlpha) + 20.0f;

        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(255 * coreAlpha));
        DrawCircle(SCREEN_W / 2, SCREEN_H / 2, (int)coreSize, GetColor(255, 255, 200), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void SplashScene::DrawSlideTransition()
{
    // 上から下にスライドするマスク
    DrawBox(0, 0, SCREEN_W, (int)slideOffset, GetColor(0, 0, 0), TRUE);
}

void SplashScene::InitializeExplosion()
{
    explosionStarted = true;
    explosionTimer = 0.0f;

    // 画面中央から爆発パーティクルを生成
    CreateExplosionParticles(SCREEN_W / 2.0f, SCREEN_H / 2.0f);
}

void SplashScene::CreateExplosionParticles(float centerX, float centerY)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> speedDist(100.0f, 600.0f);  // 速度を大幅に増加
    std::uniform_real_distribution<float> sizeDist(15.0f, 50.0f);     // サイズを大幅に増加
    std::uniform_real_distribution<float> lifeDist(1.0f, 2.5f);       // 寿命を延長

    explosionParticles.clear();

    // パーティクル数を大幅に増加（300個）
    for (int i = 0; i < 300; i++) {
        ExplosionParticle particle;

        float angle = angleDist(gen);
        float speed = speedDist(gen);

        particle.x = centerX;
        particle.y = centerY;
        particle.vx = cos(angle) * speed;
        particle.vy = sin(angle) * speed;
        particle.size = sizeDist(gen);
        particle.life = lifeDist(gen);
        particle.maxLife = particle.life;
        particle.active = true;

        // より豪華な色を設定（より多様な色）
        int colorType = i % 6;
        switch (colorType) {
        case 0: particle.color = GetColor(255, 50, 50);   // 赤
            break;
        case 1: particle.color = GetColor(255, 150, 50);  // オレンジ
            break;
        case 2: particle.color = GetColor(255, 255, 50);  // 黄色
            break;
        case 3: particle.color = GetColor(255, 255, 255); // 白
            break;
        case 4: particle.color = GetColor(255, 100, 255); // マゼンタ
            break;
        case 5: particle.color = GetColor(100, 255, 255); // シアン
            break;
        }

        explosionParticles.push_back(particle);
    }

    // 追加の火花エフェクト（中心から放射状）
    for (int i = 0; i < 50; i++) {
        ExplosionParticle spark;

        float angle = (2.0f * 3.14159f * i) / 50.0f; // 均等に配置
        float speed = 400.0f + (i % 3) * 100.0f;     // 3段階の速度

        spark.x = centerX;
        spark.y = centerY;
        spark.vx = cos(angle) * speed;
        spark.vy = sin(angle) * speed;
        spark.size = 8.0f + (i % 4) * 3.0f;
        spark.life = 1.5f;
        spark.maxLife = spark.life;
        spark.active = true;
        spark.color = GetColor(255, 255, 200); // 明るい黄色

        explosionParticles.push_back(spark);
    }
}

void SplashScene::UpdateExplosionParticles()
{
    for (auto& particle : explosionParticles) {
        if (!particle.active) continue;

        // 位置更新
        particle.x += particle.vx * 0.016f;
        particle.y += particle.vy * 0.016f;

        // より強い重力効果
        particle.vy += 400.0f * 0.016f;

        // 空気抵抗効果（速度を徐々に減衰）
        particle.vx *= 0.995f;
        particle.vy *= 0.995f;

        // 寿命減少
        particle.life -= 0.016f;

        if (particle.life <= 0.0f) {
            particle.active = false;
        }
    }
}

void SplashScene::Finalize()
{
    if (logoHandle != -1) {
        DeleteGraph(logoHandle);
        logoHandle = -1;
    }

    if (bombHandle != -1) {
        DeleteGraph(bombHandle);
        bombHandle = -1;
    }

    if (bombActiveHandle != -1) {
        DeleteGraph(bombActiveHandle);
        bombActiveHandle = -1;
    }

    if (fontHandle != -1) {
        DeleteFontToHandle(fontHandle);
        fontHandle = -1;
    }

    explosionParticles.clear();
}