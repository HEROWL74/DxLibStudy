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
    // ���\�[�X�ǂݍ���
    bombHandle = LoadGraph("Sprites/Tiles/bomb.png");
    bombActiveHandle = LoadGraph("Sprites/Tiles/bomb_active.png");

    // ���S�͓K���ȉ摜���g�p�i���Ȃ��̂ŁA�e�L�X�g�ő�p���j
    // logoHandle = LoadGraph("Sprites/studio_logo.png");

    // �t�H���g�쐬
    fontHandle = CreateFontToHandle(NULL, 48, 3);

    // ������Ԑݒ�
    currentPhase = PHASE_LOGO_FADEIN;
    phaseTimer = 0.0f;
    logoAlpha = 0.0f;
    logoScale = 0.8f;
    explosionTimer = 0.0f;
    explosionStarted = false;
    bombAnimFrame = 0;
    slideOffset = 0.0f;
    transitionComplete = false;

    // �����p�[�e�B�N���z����N���A
    explosionParticles.clear();

    OutputDebugStringA("SplashScene: Initialized\n");
}

void SplashScene::Update()
{
    phaseTimer += 0.016f; // 60FPS�z��

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
    // ���S�̃t�F�[�h�C��
    logoAlpha = phaseTimer / LOGO_FADEIN_DURATION;
    logoScale = 0.8f + (0.2f * logoAlpha); // 0.8 -> 1.0�ɃX�P�[��

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
    // ���S�\�����i�Î~�j
    if (phaseTimer >= LOGO_DISPLAY_DURATION) {
        currentPhase = PHASE_EXPLOSION_PREP;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Logo display complete, preparing explosion\n");
    }
}

void SplashScene::UpdateExplosionPrep()
{
    // ���������i�{���̓_�łȂǁj
    bombAnimFrame = (int)(phaseTimer * 8.0f) % 2; // 8Hz�œ_��

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

    // �p�[�e�B�N���X�V
    UpdateExplosionParticles();

    if (phaseTimer >= EXPLOSION_DURATION) {
        currentPhase = PHASE_SLIDE_UP;
        phaseTimer = 0.0f;
        OutputDebugStringA("SplashScene: Explosion complete, starting slide transition\n");
    }
}

void SplashScene::UpdateSlideUp()
{
    // �X���C�h�A�b�v�g�����W�V����
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
    // �w�i�����œh��Ԃ�
    DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);

    switch (currentPhase) {
    case PHASE_LOGO_FADEIN:
    case PHASE_LOGO_DISPLAY:
        DrawLogo();
        break;
    case PHASE_EXPLOSION_PREP:
        DrawLogo();
        // �{����`��i�_�Łj
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
        // ���S�ɉB�ꂽ���
        break;
    }
}

void SplashScene::DrawLogo()
{
    // ���S�`��i�摜���Ȃ��ꍇ�̓e�L�X�g�ő�p�j
    if (logoHandle != -1) {
        // �摜�ł̃��S�`��
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
        // �e�L�X�g�ł̃��S�`��
        std::string logoText = "HEROWL";
        std::string subText = "Game Studio";

        // �������𐳊m�Ɍv�Z
        int logoWidth = GetDrawStringWidthToHandle(logoText.c_str(), (int)logoText.length(), fontHandle);
        int subWidth = GetDrawStringWidthToHandle(subText.c_str(), (int)subText.length(), fontHandle);

        // ��ʒ����ɐ��m�ɔz�u
        int centerX = SCREEN_W / 2;
        int centerY = SCREEN_H / 2 - 50;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * logoAlpha));

        // ���C�����S�i���������j
        int logoX = centerX - logoWidth;
        int logoY = centerY - 40;
        DrawStringToHandle(logoX, logoY, logoText.c_str(), GetColor(255, 255, 255), fontHandle);

        // �T�u�e�L�X�g�i���������j
        int subX = centerX - subWidth  + 47;
        int subY = centerY + 30;
        DrawStringToHandle(subX, subY, subText.c_str(), GetColor(180, 180, 180), fontHandle);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void SplashScene::DrawExplosion()
{
    // ��苭�͂Ȕ����t���b�V������
    float flashAlpha = 0.0f;
    if (explosionTimer < 0.3f) {
        flashAlpha = 1.0f - (explosionTimer / 0.3f);
    }

    if (flashAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * flashAlpha));
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �w�i�ɕ��ˏ�̃O���f�[�V��������
    if (explosionTimer < 1.0f) {
        float bgAlpha = (1.0f - explosionTimer) * 0.3f;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * bgAlpha));

        // ���S������ˏ�ɐF��ω�
        for (int r = 0; r < 400; r += 20) {
            int alpha = (int)(255 * bgAlpha * (1.0f - r / 400.0f));
            if (alpha > 0) {
                SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
                DrawCircle(SCREEN_W / 2, SCREEN_H / 2, r, GetColor(255, 100 + r / 4, 0), FALSE);
            }
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �p�[�e�B�N���`��i��荋�؂Ɂj
    for (const auto& particle : explosionParticles) {
        if (!particle.active) continue;

        float alpha = particle.life / particle.maxLife;
        int size = (int)(particle.size * alpha);

        if (size > 0) {
            // ���C���p�[�e�B�N��
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
            DrawCircle((int)particle.x, (int)particle.y, size, particle.color, TRUE);

            // ������ʁi���Z�����j
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(128 * alpha));
            DrawCircle((int)particle.x, (int)particle.y, size / 2, GetColor(255, 255, 255), TRUE);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // ���S���̋������i�����̊j�j
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
    // �ォ�牺�ɃX���C�h����}�X�N
    DrawBox(0, 0, SCREEN_W, (int)slideOffset, GetColor(0, 0, 0), TRUE);
}

void SplashScene::InitializeExplosion()
{
    explosionStarted = true;
    explosionTimer = 0.0f;

    // ��ʒ������甚���p�[�e�B�N���𐶐�
    CreateExplosionParticles(SCREEN_W / 2.0f, SCREEN_H / 2.0f);
}

void SplashScene::CreateExplosionParticles(float centerX, float centerY)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159f);
    std::uniform_real_distribution<float> speedDist(100.0f, 600.0f);  // ���x��啝�ɑ���
    std::uniform_real_distribution<float> sizeDist(15.0f, 50.0f);     // �T�C�Y��啝�ɑ���
    std::uniform_real_distribution<float> lifeDist(1.0f, 2.5f);       // ����������

    explosionParticles.clear();

    // �p�[�e�B�N������啝�ɑ����i300�j
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

        // ��荋�؂ȐF��ݒ�i��葽�l�ȐF�j
        int colorType = i % 6;
        switch (colorType) {
        case 0: particle.color = GetColor(255, 50, 50);   // ��
            break;
        case 1: particle.color = GetColor(255, 150, 50);  // �I�����W
            break;
        case 2: particle.color = GetColor(255, 255, 50);  // ���F
            break;
        case 3: particle.color = GetColor(255, 255, 255); // ��
            break;
        case 4: particle.color = GetColor(255, 100, 255); // �}�[���^
            break;
        case 5: particle.color = GetColor(100, 255, 255); // �V�A��
            break;
        }

        explosionParticles.push_back(particle);
    }

    // �ǉ��̉ΉԃG�t�F�N�g�i���S������ˏ�j
    for (int i = 0; i < 50; i++) {
        ExplosionParticle spark;

        float angle = (2.0f * 3.14159f * i) / 50.0f; // �ϓ��ɔz�u
        float speed = 400.0f + (i % 3) * 100.0f;     // 3�i�K�̑��x

        spark.x = centerX;
        spark.y = centerY;
        spark.vx = cos(angle) * speed;
        spark.vy = sin(angle) * speed;
        spark.size = 8.0f + (i % 4) * 3.0f;
        spark.life = 1.5f;
        spark.maxLife = spark.life;
        spark.active = true;
        spark.color = GetColor(255, 255, 200); // ���邢���F

        explosionParticles.push_back(spark);
    }
}

void SplashScene::UpdateExplosionParticles()
{
    for (auto& particle : explosionParticles) {
        if (!particle.active) continue;

        // �ʒu�X�V
        particle.x += particle.vx * 0.016f;
        particle.y += particle.vy * 0.016f;

        // ��苭���d�͌���
        particle.vy += 400.0f * 0.016f;

        // ��C��R���ʁi���x�����X�Ɍ����j
        particle.vx *= 0.995f;
        particle.vy *= 0.995f;

        // ��������
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