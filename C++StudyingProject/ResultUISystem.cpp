#include "ResultUISystem.h"
#include <math.h>
#include <algorithm>

using namespace std;

ResultUISystem::ResultUISystem()
    : starOutlineTexture(-1)
    , starFilledTexture(-1)
    , buttonTexture(-1)
    , fontHandle(-1)
    , largeFontHandle(-1)
    , resultState(RESULT_HIDDEN)
    , animationProgress(0.0f)
    , starsAnimProgress(0.0f)
    , buttonsAlpha(0.0f)
    , buttonsVisible(false)
    , displayedStars(0)
    , totalDisplayStars(0)
    , currentStageNumber(1)
    , mouseX(0), mouseY(0)
    , mousePressed(false), mousePressedPrev(false)
    // **�V�ǉ�: �N���G�C�e�B�u�G�t�F�N�g�p�ϐ�**
    , titleExplosionProgress(0.0f)
    , confettiTimer(0.0f)
    , sparkleWavePhase(0.0f)
    , backgroundRippleProgress(0.0f)
    , panelRotation(0.0f)
    , titleBouncePhase(0.0f)
    , starRainTimer(0.0f)
    , rainbowCyclePhase(0.0f)
    , scoreCounterProgress(0.0f)
    , finalScoreDisplay(0)
    , targetScore(0)
{
    // **�R���t�F�b�e�B�ƃp�[�e�B�N��������**
    confettiParticles.resize(CONFETTI_COUNT);
    sparkleParticles.resize(SPARKLE_COUNT);

    for (auto& particle : confettiParticles) {
        particle.active = false;
    }

    for (auto& sparkle : sparkleParticles) {
        sparkle.active = false;
    }
}

ResultUISystem::~ResultUISystem()
{
    if (starOutlineTexture != -1) DeleteGraph(starOutlineTexture);
    if (starFilledTexture != -1) DeleteGraph(starFilledTexture);
    if (buttonTexture != -1) DeleteGraph(buttonTexture);
    if (fontHandle != -1) DeleteFontToHandle(fontHandle);
    if (largeFontHandle != -1) DeleteFontToHandle(largeFontHandle);
}

void ResultUISystem::Initialize()
{
    LoadTextures();

    // **�{�^���������i���S�����z�u�j**
    int screenWidth = 1920;
    int screenHeight = 1080;
    int buttonWidth = 384;
    int buttonHeight = 128;
    int buttonSpacing = 50; // �{�^���Ԃ̊Ԋu

    // 2�̃{�^���̑������v�Z
    int totalButtonWidth = buttonWidth * 2 + buttonSpacing;

    // ��ʒ�������̊J�n�ʒu���v�Z
    int startX = (screenWidth - totalButtonWidth) / 2;
    int buttonY = 750; // Y���W�ʒu

    retryButton = {
        startX, buttonY, buttonWidth, buttonHeight,
        "RETRY", BUTTON_SCALE_NORMAL, 0.0f, false, true, 0.0f
    };

    nextStageButton = {
        startX + buttonWidth + buttonSpacing, buttonY, buttonWidth, buttonHeight,
        "NEXT", BUTTON_SCALE_NORMAL, 0.0f, false, true, 0.0f
    };
}

void ResultUISystem::LoadTextures()
{
    starOutlineTexture = LoadGraph("UI/PNG/Yellow/star_outline_depth.png");
    starFilledTexture = LoadGraph("UI/PNG/Yellow/star.png");
    buttonTexture = LoadGraph("UI/PNG/Yellow/button_rectangle_depth_gradient.png");
    fontHandle = CreateFontToHandle(NULL, 28, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);
}

void ResultUISystem::Update()
{
    // �}�E�X���͎擾
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // **�N���G�C�e�B�u�A�j���[�V�����X�V**
    UpdateCreativeAnimations();

    // **�]���̃A�j���[�V�����X�V**
    UpdateAnimation();

    // �{�^���X�V
    if (resultState == RESULT_VISIBLE) {
        UpdateButtons();
    }
}

// **�V�@�\: �N���G�C�e�B�u�A�j���[�V���������X�V**
void ResultUISystem::UpdateCreativeAnimations()
{
    // **�ėp�t�F�[�Y�X�V**
    sparkleWavePhase += SPARKLE_WAVE_SPEED;
    panelRotation += PANEL_ROTATION_SPEED;
    titleBouncePhase += TITLE_BOUNCE_SPEED;
    rainbowCyclePhase += RAINBOW_CYCLE_SPEED;

    // **2�΂Ń��Z�b�g�i�I�[�o�[�t���[�h�~�j**
    if (sparkleWavePhase >= 6.28318f) sparkleWavePhase -= 6.28318f;
    if (panelRotation >= 6.28318f) panelRotation -= 6.28318f;
    if (titleBouncePhase >= 6.28318f) titleBouncePhase -= 6.28318f;
    if (rainbowCyclePhase >= 6.28318f) rainbowCyclePhase -= 6.28318f;

    // **��ԕʂ̓���G�t�F�N�g**
    if (resultState == RESULT_SHOWING) {
        // **1. �^�C�g�������G�t�F�N�g**
        if (animationProgress > 0.3f) {
            titleExplosionProgress += EXPLOSION_SPEED;
            titleExplosionProgress = min(1.0f, titleExplosionProgress);
        }

        // **2. �w�i�g��G�t�F�N�g**
        if (animationProgress > 0.1f) {
            backgroundRippleProgress += RIPPLE_SPEED;
            backgroundRippleProgress = min(1.0f, backgroundRippleProgress);
        }

        // **3. �X�R�A�J�E���^�[�A�j���[�V����**
        if (animationProgress > 0.6f) {
            scoreCounterProgress += SCORE_COUNT_SPEED;
            scoreCounterProgress = min(1.0f, scoreCounterProgress);

            // **�X�R�A�̃J�E���g�A�b�v���o**
            finalScoreDisplay = (int)(targetScore * EaseOutBounce(scoreCounterProgress));
        }
    }

    // **�p�[�e�B�N���V�X�e���X�V**
    UpdateConfettiParticles();
    UpdateSparkleParticles();

    // **�R���t�F�b�e�B�����^�C�}�[**
    if (resultState == RESULT_SHOWING && animationProgress > 0.5f) {
        confettiTimer += 0.016f;
        if (confettiTimer >= CONFETTI_SPAWN_RATE) {
            SpawnConfetti();
            confettiTimer = 0.0f;
        }
    }

    // **�X�^�[���C�������^�C�}�[**
    if (resultState == RESULT_VISIBLE ||
        (resultState == RESULT_SHOWING && starsAnimProgress > 0.5f)) {
        starRainTimer += 0.016f;
        if (starRainTimer >= STAR_RAIN_RATE) {
            SpawnStarRain();
            starRainTimer = 0.0f;
        }
    }
}

void ResultUISystem::UpdateAnimation()
{
    switch (resultState) {
    case RESULT_SHOWING:
        // **�i�K1: ���C���p�l���̃A�j���[�V�����i�e���G�t�F�N�g�t���j**
        animationProgress += ANIM_SPEED;
        if (animationProgress >= 1.0f) {
            animationProgress = 1.0f;
        }

        // **�i�K2: ���̃A�j���[�V�����i�g��o��j**
        if (animationProgress > 0.4f) {
            starsAnimProgress += STAR_ANIM_SPEED * 0.6f;
        }

        // **�i�K3: �{�^���t�F�[�h�C���i�G���K���g�j**
        if (starsAnimProgress >= 1.2f && !buttonsVisible) {
            buttonsVisible = true;
        }

        if (buttonsVisible) {
            buttonsAlpha += BUTTON_FADE_SPEED;
            if (buttonsAlpha >= 1.0f) {
                buttonsAlpha = 1.0f;
                resultState = RESULT_VISIBLE;
            }
        }
        break;

    case RESULT_VISIBLE:
        // **�p���I�ȃA�j���[�V�����ێ�**
        starsAnimProgress += STAR_ANIM_SPEED * 0.1f;
        break;

    case RESULT_HIDING:
        animationProgress -= ANIM_SPEED * 2.0f;
        buttonsAlpha -= BUTTON_FADE_SPEED * 2.0f;
        starsAnimProgress -= STAR_ANIM_SPEED * 2.0f;

        if (animationProgress <= 0.0f) {
            animationProgress = 0.0f;
            starsAnimProgress = 0.0f;
            buttonsAlpha = 0.0f;
            buttonsVisible = false;
            resultState = RESULT_HIDDEN;

            // **�G�t�F�N�g���Z�b�g**
            ResetCreativeEffects();
        }
        break;

    default:
        break;
    }
}

void ResultUISystem::UpdateButtons()
{
    if (!buttonsVisible || buttonsAlpha < 0.1f) {
        return;
    }

    // **���I�ȃ{�^���G�t�F�N�g**
    float breatheEffect = sinf(titleBouncePhase * 0.5f) * 0.02f + 1.0f;

    // ���g���C�{�^��
    bool retryHovered = IsMouseOver(retryButton.x, retryButton.y, retryButton.w, retryButton.h);
    retryButton.hovered = retryHovered;
    retryButton.scale = Lerp(retryButton.scale,
        (retryHovered ? BUTTON_SCALE_HOVER : BUTTON_SCALE_NORMAL) * breatheEffect, 0.2f);
    retryButton.glowIntensity = Lerp(retryButton.glowIntensity,
        retryHovered ? 1.0f : 0.0f, 0.15f);

    // �l�N�X�g�{�^��
    bool nextHovered = IsMouseOver(nextStageButton.x, nextStageButton.y, nextStageButton.w, nextStageButton.h);
    nextStageButton.hovered = nextHovered;
    nextStageButton.scale = Lerp(nextStageButton.scale,
        (nextHovered ? BUTTON_SCALE_HOVER : BUTTON_SCALE_NORMAL) * breatheEffect, 0.2f);
    nextStageButton.glowIntensity = Lerp(nextStageButton.glowIntensity,
        nextHovered ? 1.0f : 0.0f, 0.15f);

    // �{�^���A�j���[�V�����i�s
    retryButton.animProgress = min(1.0f, retryButton.animProgress + 0.1f);
    nextStageButton.animProgress = min(1.0f, nextStageButton.animProgress + 0.1f);
}

void ResultUISystem::Draw()
{
    if (resultState == RESULT_HIDDEN) return;

    // **���C���[���`��**
    DrawEnhancedBackground();      // 1. �������ꂽ�w�i�i�w�i�G�t�F�N�g�܂ށj
    DrawMainPanel();              // 2. ���C���p�l��
    DrawEnhancedTitle();          // 3. �������ꂽ�^�C�g��
    DrawCreativeStars();          // 4. �N���G�C�e�B�u�Ȑ��\��
    DrawScoreDisplay();           // 5. �X�R�A�\��
    DrawEnhancedButtons();        // 6. �������ꂽ�{�^��
    DrawParticleEffects();        // 7. �p�[�e�B�N���G�t�F�N�g�i�őO�ʁj
}

// **�V�@�\: �������ꂽ�w�i�`��**
void ResultUISystem::DrawEnhancedBackground()
{
    float easedProgress = EaseInOutCubic(animationProgress);

    // **1. ���I�O���f�[�V�����w�i**
    DrawDynamicGradientBackground(easedProgress);

    // **2. �g��G�t�F�N�g**
    if (backgroundRippleProgress > 0.0f) {
        DrawRippleEffect();
    }

    // **3. �􉽊w�p�^�[��**
    DrawGeometricPattern(easedProgress);
}

void ResultUISystem::DrawDynamicGradientBackground(float progress)
{
    int baseAlpha = (int)(200 * progress);

    // **���C���{�[���ʂ��܂ޓ��I�O���f�[�V����**
    for (int i = 0; i < 1080; i++) {
        float gradientRatio = (float)i / 1080;

        // **���F�T�C�N���v�Z**
        float hue = fmodf(rainbowCyclePhase + gradientRatio * 2.0f, 6.28318f);
        int r, g, b;
        HSVtoRGB(hue, 0.3f, 0.4f + gradientRatio * 0.6f, r, g, b);

        int lineAlpha = (int)(baseAlpha * (0.2f + gradientRatio * 0.8f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, lineAlpha);
        DrawLine(0, i, 1920, i, GetColor(r, g, b));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawRippleEffect()
{
    int centerX = 1920 / 2;
    int centerY = 540;

    // **�����̔g��𓯐S�~��ɕ`��**
    for (int ring = 0; ring < 5; ring++) {
        float ringProgress = backgroundRippleProgress - ring * 0.15f;
        if (ringProgress > 0.0f && ringProgress < 1.0f) {
            float radius = ringProgress * 800.0f + ring * 100.0f;
            int alpha = (int)(80 * (1.0f - ringProgress) / (ring + 1));

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawCircle(centerX, centerY, (int)radius,
                GetColor(100 + ring * 40, 150 + ring * 20, 255 - ring * 30), FALSE);
        }
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawGeometricPattern(float progress)
{
    // **��]����􉽊w�p�^�[��**
    int centerX = 1920 / 2;
    int centerY = 540;

    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(30 * progress));

    for (int i = 0; i < 12; i++) {
        float angle = panelRotation + i * (6.28318f / 12);
        float radius = 300.0f + sinf(sparkleWavePhase + i * 0.5f) * 50.0f;

        int x1 = centerX + (int)(cosf(angle) * radius);
        int y1 = centerY + (int)(sinf(angle) * radius);
        int x2 = centerX + (int)(cosf(angle + 0.2f) * (radius + 100));
        int y2 = centerY + (int)(sinf(angle + 0.2f) * (radius + 100));

        DrawLine(x1, y1, x2, y2, GetColor(255, 200, 100));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: ���C���p�l���`��**
void ResultUISystem::DrawMainPanel()
{
    float easedProgress = EaseOutBack(animationProgress);

    // **�p�l���T�C�Y�ƈʒu�v�Z�i���S�����z�u�j**
    int screenWidth = 1920;
    int screenHeight = 1080;
    int panelWidth = 800;  // �p�l�����������傫��
    int panelHeight = 500; // �p�l�������������傫��
    int panelX = (screenWidth - panelWidth) / 2;   // ���S����
    int panelY = (screenHeight - panelHeight) / 2 - 50; // ������ɒ���

    // �A�j���[�V�����p��Y���W�I�t�Z�b�g
    panelY += (int)((1.0f - easedProgress) * 150);

    // **�y���ȉ�]�G�t�F�N�g**
    float rotation = sinf(panelRotation) * 0.02f;

    // **3D���̉e����**
    int shadowOffset = (int)(15 * easedProgress);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(100 * easedProgress));
    DrawBox(panelX + shadowOffset, panelY + shadowOffset,
        panelX + panelWidth + shadowOffset, panelY + panelHeight + shadowOffset,
        GetColor(0, 0, 0), TRUE);

    // **���C���p�l���i�O���X�G�t�F�N�g�j**
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(240 * easedProgress));
    DrawBox(panelX, panelY, panelX + panelWidth, panelY + panelHeight,
        GetColor(60, 60, 100), TRUE);

    // **�p�l�����E���i�O���[���ʁj**
    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * easedProgress));
    for (int i = 0; i < 3; i++) {
        DrawBox(panelX - i, panelY - i, panelX + panelWidth + i, panelY + panelHeight + i,
            GetColor(120, 160, 255), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: �������ꂽ�^�C�g���`��**
void ResultUISystem::DrawEnhancedTitle()
{
    float easedProgress = EaseOutBack(animationProgress);

    string titleText = "STAGE " + to_string(currentStageNumber) + " CLEAR!";
    int titleWidth = GetDrawStringWidthToHandle(titleText.c_str(), titleText.length(), largeFontHandle);
    int titleX = (1920 - titleWidth) / 2; // ���S�����z�u
    int titleY = 280; // �p�l�����̓K�؂Ȉʒu

    // **�^�C�g�������G�t�F�N�g**
    if (titleExplosionProgress > 0.0f) {
        DrawTitleExplosion(titleX, titleY, titleWidth);
    }

    // **���I�o�E���X����**
    float bounceOffset = sinf(titleBouncePhase) * 3.0f * easedProgress;
    titleY += (int)bounceOffset;

    // **�^�C�g���X�P�[���A�j���[�V����**
    float titleScale = easedProgress * (1.0f + sinf(titleBouncePhase * 2.0f) * 0.05f);
    int alpha = (int)(255 * easedProgress);

    // **���w�O���[����**
    if (easedProgress > 0.3f) {
        DrawMultiLayerGlow(titleX - 30, titleY - 15, titleWidth + 60, 80,
            (easedProgress - 0.3f) * 1.43f);
    }

    // **���C���^�C�g���`��**
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawStringToHandle(titleX, titleY, titleText.c_str(),
        GetColor(255, 255, 255), largeFontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawTitleExplosion(int centerX, int centerY, int titleWidth)
{
    // **���������p�[�e�B�N������**
    int particleCount = 20;
    for (int i = 0; i < particleCount; i++) {
        float angle = (float)i * (6.28318f / particleCount);
        float distance = titleExplosionProgress * 150.0f;
        float particleLife = max(0.0f, 1.0f - titleExplosionProgress * 1.5f);

        int particleX = centerX + titleWidth / 2 + (int)(cosf(angle) * distance);
        int particleY = centerY + 40 + (int)(sinf(angle) * distance * 0.5f);

        int size = (int)(8 * particleLife);
        int alpha = (int)(200 * particleLife);

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(particleX, particleY, size, GetColor(255, 255, 150), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: �N���G�C�e�B�u�X�^�[�`��**
void ResultUISystem::DrawCreativeStars()
{
    if (animationProgress < 0.3f) return;

    int starSize = 90;
    int starSpacing = 140;
    int startX = (1920 - (3 * starSpacing)) / 2 + starSpacing / 2; // ���S�����z�u
    int starY = 420; // �p�l�����̓K�؂Ȉʒu

    for (int i = 0; i < 3; i++) {
        int starX = startX + i * starSpacing;

        // **�g��o�����**
        float starDelay = i * 0.4f;
        float starProgress = max(0.0f, (starsAnimProgress - starDelay) * 1.5f);

        if (starProgress <= 0.0f) continue;

        // **���̎��͂ɃG�l���M�[�����O**
        DrawStarEnergyRing(starX, starY, starSize, starProgress, i);

        // **���C�����`��**
        if (i < displayedStars) {
            DrawAdvancedCollectedStar(starX, starY, starSize, starProgress, i);
        }
        else {
            DrawAdvancedUnCollectedStar(starX, starY, starSize, starProgress);
        }
    }

    // **���J�E���g�\���i���@�I�G�t�F�N�g�t���j**
    if (starsAnimProgress > 1.5f) {
        DrawMagicalStarCount(startX + starSpacing, starY + starSize + 50);
    }
}

void ResultUISystem::DrawStarEnergyRing(int centerX, int centerY, int baseSize, float progress, int starIndex)
{
    float ringProgress = min(1.0f, progress * 2.0f);
    float ringRadius = baseSize * 0.8f + sinf(sparkleWavePhase + starIndex * 0.7f) * 15.0f;

    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(80 * ringProgress));

    // **�����̃G�l���M�[�����O**
    for (int ring = 0; ring < 3; ring++) {
        float currentRadius = ringRadius + ring * 20.0f;
        int alpha = (int)(80 * ringProgress / (ring + 1));

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(centerX, centerY, (int)currentRadius,
            GetColor(255, 200 - ring * 50, 100 + ring * 50), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: �X�R�A�\��**
void ResultUISystem::DrawScoreDisplay()
{
    if (scoreCounterProgress <= 0.0f) return;

    int scoreY = 580; // �p�l�����̓K�؂Ȉʒu
    int scoreX = 1920 / 2; // ���S����

    // **�X�R�A�w�i�p�l��**
    DrawScorePanel(scoreX - 150, scoreY - 30, 300, 60);

    // **�J�E���g�A�b�v���l**
    string scoreText = "SCORE: " + to_string(finalScoreDisplay);
    int textWidth = GetDrawStringWidthToHandle(scoreText.c_str(), scoreText.length(), fontHandle);

    // **���l�J�E���^�[����**
    float counterShake = (targetScore > finalScoreDisplay) ?
        sinf(scoreCounterProgress * 50.0f) * 2.0f : 0.0f;

    int textX = scoreX - textWidth / 2 + (int)counterShake; // ���S�����z�u

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * scoreCounterProgress));
    DrawStringToHandle(textX, scoreY, scoreText.c_str(),
        GetColor(255, 215, 0), fontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawScorePanel(int x, int y, int w, int h)
{
    float glowIntensity = sinf(titleBouncePhase) * 0.3f + 0.7f;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(200 * scoreCounterProgress));
    DrawBox(x, y, x + w, y + h, GetColor(30, 30, 60), TRUE);

    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * glowIntensity * scoreCounterProgress));
    DrawBox(x - 2, y - 2, x + w + 2, y + h + 2, GetColor(255, 215, 0), FALSE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�p�[�e�B�N���V�X�e������**
void ResultUISystem::UpdateConfettiParticles()
{
    for (auto& particle : confettiParticles) {
        if (!particle.active) continue;

        particle.x += particle.vx;
        particle.y += particle.vy;
        particle.vy += 0.2f; // �d��
        particle.rotation += particle.rotationSpeed;
        particle.life -= 0.016f;

        if (particle.life <= 0.0f || particle.y > 1100) {
            particle.active = false;
        }
    }
}

void ResultUISystem::UpdateSparkleParticles()
{
    for (auto& sparkle : sparkleParticles) {
        if (!sparkle.active) continue;

        sparkle.x += sparkle.vx;
        sparkle.y += sparkle.vy;
        sparkle.life -= 0.016f;
        sparkle.scale = sparkle.life / sparkle.maxLife;

        if (sparkle.life <= 0.0f) {
            sparkle.active = false;
        }
    }
}

void ResultUISystem::SpawnConfetti()
{
    for (int i = 0; i < 5; i++) {
        for (auto& particle : confettiParticles) {
            if (particle.active) continue;

            particle.x = (float)(rand() % 1920);
            particle.y = -20.0f;
            particle.vx = (rand() % 200 - 100) / 50.0f;
            particle.vy = (rand() % 100 + 50) / 50.0f;
            particle.rotation = 0.0f;
            particle.rotationSpeed = (rand() % 200 - 100) / 1000.0f;
            particle.life = 3.0f + (rand() % 200) / 100.0f;
            particle.maxLife = particle.life;
            particle.size = 4.0f + (rand() % 6);

            // �����_���ȐF
            int colorChoice = rand() % 5;
            switch (colorChoice) {
            case 0: particle.color = GetColor(255, 100, 100); break;
            case 1: particle.color = GetColor(100, 255, 100); break;
            case 2: particle.color = GetColor(100, 100, 255); break;
            case 3: particle.color = GetColor(255, 255, 100); break;
            case 4: particle.color = GetColor(255, 100, 255); break;
            }

            particle.active = true;
            break;
        }
    }
}

void ResultUISystem::SpawnStarRain()
{
    for (auto& sparkle : sparkleParticles) {
        if (sparkle.active) continue;

        sparkle.x = (float)(rand() % 1920);
        sparkle.y = -10.0f;
        sparkle.vx = (rand() % 100 - 50) / 100.0f;
        sparkle.vy = (rand() % 200 + 100) / 100.0f;
        sparkle.life = 2.0f + (rand() % 150) / 100.0f;
        sparkle.maxLife = sparkle.life;
        sparkle.scale = 1.0f;
        sparkle.color = GetColor(255, 255, 150);
        sparkle.active = true;
        break;
    }
}

// **�p�[�e�B�N���`��**
void ResultUISystem::DrawParticleEffects()
{
    // �R���t�F�b�e�B�`��
    for (const auto& particle : confettiParticles) {
        if (!particle.active) continue;

        float alpha = particle.life / particle.maxLife;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));

        // ��]���鏬���Ȏl�p�`
        int size = (int)particle.size;
        DrawBox((int)particle.x - size / 2, (int)particle.y - size / 2,
            (int)particle.x + size / 2, (int)particle.y + size / 2,
            particle.color, TRUE);
    }

    // �X�^�[���C���`��
    for (const auto& sparkle : sparkleParticles) {
        if (!sparkle.active) continue;

        float alpha = sparkle.life / sparkle.maxLife;
        int size = (int)(8 * sparkle.scale);

        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(200 * alpha));
        DrawCircle((int)sparkle.x, (int)sparkle.y, size, sparkle.color, TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: �������ꂽ�{�^���`��**
void ResultUISystem::DrawEnhancedButtons()
{
    if (!buttonsVisible || buttonsAlpha < 0.01f) return;

    // **���I�X���C�h�C������**
    float slideProgress = EaseOutElastic(buttonsAlpha);
    float slideOffset = (1.0f - slideProgress) * 100.0f;

    // **�z���O���t�B�b�N����**
    DrawButtonHologram(retryButton, slideOffset, 0.0f);
    DrawButtonHologram(nextStageButton, slideOffset, 0.2f);

    // **���C���{�^���`��**
    UIButton tempRetryButton = retryButton;
    tempRetryButton.y -= (int)slideOffset;
    DrawAdvancedButton(tempRetryButton, buttonsAlpha);

    UIButton tempNextButton = nextStageButton;
    tempNextButton.y -= (int)(slideOffset * 0.8f); // ��������
    DrawAdvancedButton(tempNextButton, buttonsAlpha * 0.9f);
}

void ResultUISystem::DrawButtonHologram(const UIButton& button, float slideOffset, float delay)
{
    float hologramProgress = max(0.0f, buttonsAlpha - delay);
    if (hologramProgress <= 0.0f) return;

    int buttonY = button.y - (int)slideOffset;

    // **�z���O��������������**
    for (int i = 0; i < 5; i++) {
        float scanProgress = fmodf(titleBouncePhase * 0.3f + i * 0.2f, 1.0f);
        int scanY = buttonY + (int)(scanProgress * button.h);

        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(50 * hologramProgress));
        DrawLine(button.x - 10, scanY, button.x + button.w + 10, scanY,
            GetColor(100, 200, 255));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawAdvancedButton(const UIButton& button, float alpha)
{
    if (alpha <= 0) return;

    float scale = button.scale;
    int buttonWidth = (int)(button.w * scale);
    int buttonHeight = (int)(button.h * scale);
    int buttonX = button.x - (buttonWidth - button.w) / 2;
    int buttonY = button.y - (buttonHeight - button.h) / 2;

    // **���w�O���[����**
    if (button.glowIntensity > 0.01f) {
        DrawMultiLayerGlow(buttonX - 15, buttonY - 15,
            buttonWidth + 30, buttonHeight + 30,
            button.glowIntensity);
    }

    // **�{�^���w�i�i�����x�K�p�j**
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
    DrawExtendGraph(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight,
        buttonTexture, TRUE);

    // **�G�l���M�[���E��**
    if (button.hovered) {
        DrawEnergyBorder(buttonX, buttonY, buttonWidth, buttonHeight, alpha);
    }

    // **�{�^���e�L�X�g�i�e�t���j**
    int textWidth = GetDrawStringWidthToHandle(button.label.c_str(), button.label.length(), fontHandle);
    int textX = button.x + button.w / 2 - textWidth / 2;
    int textY = button.y + button.h / 2 - 14;

    // �e�L�X�g�̉e����
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(alpha * 100));
    DrawStringToHandle(textX + 3, textY + 3, button.label.c_str(), GetColor(0, 0, 0), fontHandle);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
    DrawStringToHandle(textX, textY, button.label.c_str(), GetColor(255, 255, 255), fontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawEnergyBorder(int x, int y, int w, int h, float alpha)
{
    // **�d�C�I���E���G�t�F�N�g**
    for (int i = 0; i < 3; i++) {
        float energyPhase = sparkleWavePhase + i * 2.0f;
        int energyAlpha = (int)(80 * alpha * (sinf(energyPhase) * 0.5f + 0.5f));

        SetDrawBlendMode(DX_BLENDMODE_ADD, energyAlpha);
        DrawBox(x - i - 1, y - i - 1, x + w + i + 1, y + h + i + 1,
            GetColor(100 + i * 50, 150 + i * 30, 255), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: ���x�Ȑ��`��**
void ResultUISystem::DrawAdvancedCollectedStar(int x, int y, int baseSize, float progress, int starIndex)
{
    // **1. �o��A�j���[�V����**
    float appearScale = EaseOutBounce(min(1.0f, progress));

    // **2. �p���I�Ȗ���**
    float pulseScale = 1.0f;
    if (progress > 1.0f) {
        float pulsePhase = sparkleWavePhase + starIndex * 0.8f;
        pulseScale = 1.0f + sinf(pulsePhase) * 0.15f;
    }

    float finalScale = appearScale * pulseScale;
    int finalSize = (int)(baseSize * finalScale);

    // **3. �����x�v�Z**
    int alpha = 255;
    if (progress <= 1.0f) {
        alpha = (int)(255 * progress);
    }

    // **4. ���̎��͂ɃG�l���M�[�g**
    DrawStarEnergyWaves(x, y, finalSize, progress, starIndex);

    // **5. ���C�����`��**
    int starX = x - finalSize / 2;
    int starY = y - finalSize / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawExtendGraph(starX, starY, starX + finalSize, starY + finalSize,
        starFilledTexture, TRUE);

    // **6. �����O���[����**
    if (progress > 0.5f) {
        DrawInnerStarGlow(x, y, finalSize, alpha);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawAdvancedUnCollectedStar(int x, int y, int baseSize, float progress)
{
    // **�o��A�j���[�V�����i�T���߁j**
    float appearScale = EaseOutCubic(min(1.0f, progress));

    // **�킸���Ȗ����i�擾�ς݂��T���߁j**
    float pulseScale = 1.0f;
    if (progress > 1.0f) {
        pulseScale = 1.0f + sinf(sparkleWavePhase * 0.5f) * 0.05f;
    }

    float finalScale = appearScale * pulseScale;
    int finalSize = (int)(baseSize * finalScale);

    int alpha = 255;
    if (progress <= 1.0f) {
        alpha = (int)(255 * progress);
    }

    // **�A�E�g���C�����`��**
    int starX = x - finalSize / 2;
    int starY = y - finalSize / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawExtendGraph(starX, starY, starX + finalSize, starY + finalSize,
        starOutlineTexture, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawStarEnergyWaves(int centerX, int centerY, int size, float progress, int starIndex)
{
    if (progress <= 0.5f) return;

    float waveProgress = (progress - 0.5f) * 2.0f;

    // **�����̔g�������O**
    for (int wave = 0; wave < 4; wave++) {
        float wavePhase = sparkleWavePhase + starIndex * 0.5f + wave * 1.5f;
        float waveRadius = size * 0.8f + sinf(wavePhase) * 20.0f + wave * 15.0f;
        int waveAlpha = (int)(60 * waveProgress / (wave + 1));

        SetDrawBlendMode(DX_BLENDMODE_ADD, waveAlpha);
        DrawCircle(centerX, centerY, (int)waveRadius,
            GetColor(255 - wave * 40, 255 - wave * 20, 150 + wave * 25), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawInnerStarGlow(int centerX, int centerY, int size, int alpha)
{
    // **�����������߂�**
    float glowIntensity = sinf(sparkleWavePhase * 2.0f) * 0.5f + 0.5f;
    int glowSize = size / 3;
    int glowAlpha = (int)(alpha * 0.6f * glowIntensity);

    SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);
    DrawCircle(centerX, centerY, glowSize, GetColor(255, 255, 200), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: ���@�I�Ȑ��J�E���g�\��**
void ResultUISystem::DrawMagicalStarCount(int centerX, int y)
{
    string starsText = to_string(displayedStars) + " / 3 STARS COLLECTED";
    int textWidth = GetDrawStringWidthToHandle(starsText.c_str(), starsText.length(), fontHandle);
    int textX = centerX - textWidth / 2;

    float textProgress = min(1.0f, (starsAnimProgress - 1.5f) * 3.0f);

    // **���@�~�̔w�i**
    DrawMagicCircleBackground(centerX, y + 20, textProgress);

    // **�e�L�X�g�̌��鉏���**
    int alpha = (int)(255 * textProgress);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 2);

    // �����̐F�ŉ����
    int colors[] = { GetColor(255, 215, 0), GetColor(255, 255, 150), GetColor(255, 180, 50) };
    for (int i = 0; i < 3; i++) {
        DrawStringToHandle(textX + i - 1, y + i - 1, starsText.c_str(), colors[i], fontHandle);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawStringToHandle(textX, y, starsText.c_str(), GetColor(255, 255, 255), fontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawMagicCircleBackground(int centerX, int centerY, float progress)
{
    if (progress <= 0.0f) return;

    // **��]���閂�@�~**
    float circleRadius = 120.0f * progress;
    int segments = 24;

    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 * progress));

    for (int i = 0; i < segments; i++) {
        float angle1 = panelRotation + i * (6.28318f / segments);
        float angle2 = panelRotation + (i + 1) * (6.28318f / segments);

        int x1 = centerX + (int)(cosf(angle1) * circleRadius);
        int y1 = centerY + (int)(sinf(angle1) * circleRadius);
        int x2 = centerX + (int)(cosf(angle2) * circleRadius);
        int y2 = centerY + (int)(sinf(angle2) * circleRadius);

        DrawLine(x1, y1, x2, y2, GetColor(100, 150, 255));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **�V�@�\: ���w�O���[����**
void ResultUISystem::DrawMultiLayerGlow(int x, int y, int w, int h, float intensity)
{
    if (intensity <= 0.01f) return;

    // **���C���{�[�O���[**
    for (int layer = 0; layer < 5; layer++) {
        float layerIntensity = intensity / (layer + 1);
        int alpha = (int)(80 * layerIntensity);
        int offset = (layer + 1) * 4;

        // HSV���g�p���ă��C���{�[����
        float hue = fmodf(rainbowCyclePhase + layer * 1.2f, 6.28318f);
        int r, g, b;
        HSVtoRGB(hue, 0.8f, 1.0f, r, g, b);

        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset,
            GetColor(r, g, b), FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// **���[�e�B���e�B�֐�**
void ResultUISystem::HSVtoRGB(float h, float s, float v, int& r, int& g, int& b)
{
    float c = v * s;
    float x = c * (1 - fabsf(fmodf(h / 1.047f, 2.0f) - 1));
    float m = v - c;

    float r_prime, g_prime, b_prime;

    if (h >= 0 && h < 1.047f) {
        r_prime = c; g_prime = x; b_prime = 0;
    }
    else if (h >= 1.047f && h < 2.094f) {
        r_prime = x; g_prime = c; b_prime = 0;
    }
    else if (h >= 2.094f && h < 3.141f) {
        r_prime = 0; g_prime = c; b_prime = x;
    }
    else if (h >= 3.141f && h < 4.188f) {
        r_prime = 0; g_prime = x; b_prime = c;
    }
    else if (h >= 4.188f && h < 5.235f) {
        r_prime = x; g_prime = 0; b_prime = c;
    }
    else {
        r_prime = c; g_prime = 0; b_prime = x;
    }

    r = (int)((r_prime + m) * 255);
    g = (int)((g_prime + m) * 255);
    b = (int)((b_prime + m) * 255);
}

// **�G�t�F�N�g���Z�b�g**
void ResultUISystem::ResetCreativeEffects()
{
    titleExplosionProgress = 0.0f;
    confettiTimer = 0.0f;
    backgroundRippleProgress = 0.0f;
    starRainTimer = 0.0f;
    scoreCounterProgress = 0.0f;
    finalScoreDisplay = 0;

    // �p�[�e�B�N���N���A
    for (auto& particle : confettiParticles) {
        particle.active = false;
    }
    for (auto& sparkle : sparkleParticles) {
        sparkle.active = false;
    }
}

// **���ʕ\���J�n**
void ResultUISystem::ShowResult(int starsCollected, int totalStars, int currentStage)
{
    displayedStars = starsCollected;
    totalDisplayStars = totalStars;
    currentStageNumber = currentStage;
    resultState = RESULT_SHOWING;
    animationProgress = 0.0f;
    starsAnimProgress = 0.0f;
    buttonsAlpha = 0.0f;
    buttonsVisible = false;

    // **�X�R�A�v�Z**
    targetScore = starsCollected * 1000 + currentStage * 500;
    finalScoreDisplay = 0;

    // �G�t�F�N�g���Z�b�g
    ResetCreativeEffects();

    // �{�^���A�j���[�V�������Z�b�g
    retryButton.animProgress = 0.0f;
    nextStageButton.animProgress = 0.0f;
}

void ResultUISystem::HideResult()
{
    resultState = RESULT_HIDING;
}

void ResultUISystem::ResetState()
{
    resultState = RESULT_HIDDEN;
    animationProgress = 0.0f;
    starsAnimProgress = 0.0f;
    buttonsAlpha = 0.0f;
    buttonsVisible = false;
    displayedStars = 0;
    totalDisplayStars = 0;
    currentStageNumber = 1;

    ResetCreativeEffects();
    ResetButtonStates();

    retryButton.animProgress = 0.0f;
    nextStageButton.animProgress = 0.0f;
}

ResultUISystem::ButtonAction ResultUISystem::GetClickedButton()
{
    if (resultState != RESULT_VISIBLE) return BUTTON_NONE;
    if (!mousePressed || mousePressedPrev) return BUTTON_NONE;

    if (IsMouseOver(retryButton.x, retryButton.y, retryButton.w, retryButton.h)) {
        return BUTTON_RETRY;
    }

    if (IsMouseOver(nextStageButton.x, nextStageButton.y, nextStageButton.w, nextStageButton.h)) {
        return BUTTON_NEXT_STAGE;
    }

    return BUTTON_NONE;
}

void ResultUISystem::ResetButtonStates()
{
    retryButton.hovered = false;
    retryButton.scale = BUTTON_SCALE_NORMAL;
    retryButton.glowIntensity = 0.0f;

    nextStageButton.hovered = false;
    nextStageButton.scale = BUTTON_SCALE_NORMAL;
    nextStageButton.glowIntensity = 0.0f;
}

bool ResultUISystem::IsMouseOver(int x, int y, int w, int h) const
{
    return mouseX >= x && mouseX <= x + w && mouseY >= y && mouseY <= y + h;
}

// **�C�[�W���O�֐�**
float ResultUISystem::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}

float ResultUISystem::EaseOutBounce(float t)
{
    const float n1 = 7.5625f;
    const float d1 = 2.75f;

    if (t < 1 / d1) {
        return n1 * t * t;
    }
    else if (t < 2 / d1) {
        return n1 * (t -= 1.5f / d1) * t + 0.75f;
    }
    else if (t < 2.5 / d1) {
        return n1 * (t -= 2.25f / d1) * t + 0.9375f;
    }
    else {
        return n1 * (t -= 2.625f / d1) * t + 0.984375f;
    }
}

float ResultUISystem::EaseOutElastic(float t)
{
    const float c4 = (2 * 3.14159265f) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : powf(2, -10 * t) * sinf((t * 10 - 0.75f) * c4) + 1;
}

float ResultUISystem::EaseOutCubic(float t)
{
    return 1 - powf(1 - t, 3);
}

float ResultUISystem::EaseInOutCubic(float t)
{
    return t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
}

float ResultUISystem::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}