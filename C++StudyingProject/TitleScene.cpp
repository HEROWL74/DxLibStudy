#include "TitleScene.h"
using namespace std;

TitleScene::TitleScene()
    : startRequested(false)
    , exitRequested(false)
    , optionState(Hidden)
    , transitionState(None)
    , optionAnimProgress(0.0f)
    , optionFadeProgress(0.0f)
    , optionSlideProgress(0.0f)
    , backgroundFadeProgress(0.0f)
    , transitionFadeProgress(0.0f)
    , titlePulsePhase(0.0f)
    , mousePressed(false)
    , mousePressedPrev(false)
    , particleSpawnTimer(0.0f)
    , randomEngine(std::random_device{}())
{
    // �p�[�e�B�N���z���������
    particles.resize(50);
    for (auto& p : particles) {
        p.active = false;
    }
}

TitleScene::~TitleScene()
{
    DeleteGraph(backgroundHandle);
    DeleteGraph(buttonHandle);
    DeleteGraph(slideHandle);
    DeleteGraph(slideBarHandle);
    DeleteGraph(homeHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);
}

void TitleScene::Initialize()
{
    // �e�N�X�`���ǂݍ���
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    buttonHandle = LoadGraph("UI/PNG/Yellow/button_rectangle_depth_gradient.png");
    slideHandle = LoadGraph("UI/PNG/Yellow/slide_hangle.png");
    slideBarHandle = LoadGraph("UI/PNG/Yellow/slide_horizontal_color.png");
    homeHandle = LoadGraph("UI/PNG/Yellow/home.png");
    fontHandle = CreateFontToHandle(NULL, 28, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);

    // �{�^���z�u�i�����ɒ����j
    int centerX = SCREEN_W / 2;
    int buttonX = centerX - BUTTON_W / 2 - 100; // 200�s�N�Z�����Ɉړ�

    buttons = {
        { buttonX, 550, BUTTON_W, BUTTON_H, "START",   NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
        { buttonX, 690, BUTTON_W, BUTTON_H, "OPTIONS", NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
        { buttonX, 830, BUTTON_W, BUTTON_H, "EXIT",    NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f }
    };

    // ���ǂ��ꂽ�X���C�_�[�ݒ�
    slider = {
        500, 0, 800, 24,  // x, y, w, h (y�͌�œ��I�ɐݒ�)
        0.5f,             // value
        false,            // dragging
        0.0f,             // hoverProgress
        0.5f,             // valueDisplay
        0.0f              // glowEffect
    };
}

void TitleScene::Update()
{
    // �}�E�X���W�擾
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // �L�[�{�[�h���͎擾
    backspacePressedPrev = backspacePressed;
    backspacePressed = CheckHitKey(KEY_INPUT_BACK) != 0;

    // �^�C�g����������
    titlePulsePhase += 0.02f;

    // �{�^���X�V
    UpdateButtons();

    // �I�v�V�����J�A�j���[�V�����i�������ꂽ�t�F�[�h�ƃX���C�h�j
    switch (optionState) {
    case Showing:
        optionAnimProgress += ANIM_SPEED;
        optionFadeProgress += FADE_SPEED;
        optionSlideProgress += ANIM_SPEED * 0.8f; // �X���C�h�͏����x��

        if (optionAnimProgress >= 1.0f) {
            optionAnimProgress = 1.0f;
            optionFadeProgress = 1.0f;
            optionSlideProgress = 1.0f;
            optionState = Visible;
        }
        break;
    case Hiding:
        optionAnimProgress -= ANIM_SPEED;
        optionFadeProgress -= FADE_SPEED * 1.5f; // �t�F�[�h�A�E�g�͑���
        optionSlideProgress -= ANIM_SPEED;

        if (optionAnimProgress <= 0.0f) {
            optionAnimProgress = 0.0f;
            optionFadeProgress = 0.0f;
            optionSlideProgress = 0.0f;
            optionState = Hidden;
        }
        break;
    case Visible:
        optionAnimProgress = 1.0f;
        optionFadeProgress = 1.0f;
        optionSlideProgress = 1.0f;
        break;
    default:
        optionAnimProgress = 0.0f;
        optionFadeProgress = 0.0f;
        optionSlideProgress = 0.0f;
        break;
    }

    // ��ʑJ�ڃt�F�[�h����
    switch (transitionState) {
    case FadingOut:
        transitionFadeProgress += TRANSITION_SPEED;
        if (transitionFadeProgress >= 1.0f) {
            transitionFadeProgress = 1.0f;
            transitionState = FadingIn;
        }
        break;
    case FadingIn:
        transitionFadeProgress -= TRANSITION_SPEED;
        if (transitionFadeProgress <= 0.0f) {
            transitionFadeProgress = 0.0f;
            transitionState = Complete;
        }
        break;
    default:
        break;
    }

    // �w�i�t�F�[�h����
    float targetFade = (optionState != Hidden) ? 1.0f : 0.0f;
    backgroundFadeProgress = Lerp(backgroundFadeProgress, targetFade, FADE_SPEED * 0.7f);

    // Backspace�L�[�ŃI�v�V������ʂ����
    if ((optionState == Visible || optionState == Showing) &&
        backspacePressed && !backspacePressedPrev && transitionState == None) {
        optionState = Hiding;
    }

    // �X���C�_�[�X�V�i��ʑJ�ڒ��łȂ��ꍇ�̂݁j
    if (optionState != Hidden && transitionState == None) {
        UpdateSlider();
    }

    // �p�[�e�B�N���X�V
    UpdateParticles();

    // �p�[�e�B�N������
    particleSpawnTimer += 1.0f;
    if (particleSpawnTimer >= 10.0f) {
        SpawnParticle(
            (float)(rand() % SCREEN_W),
            (float)SCREEN_H + 10
        );
        particleSpawnTimer = 0.0f;
    }
}

void TitleScene::Draw()
{
    // �w�i�`��
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // �p�[�e�B�N���`��i�w�i�̏�j
    DrawParticles();

    // �^�C�g�����S�i���ǂ��ꂽ�������ʕt���j
    float titleScale = 1.0f + sinf(titlePulsePhase) * 0.05f;
    int titleAlpha = (int)(255 * (0.9f + sinf(titlePulsePhase * 2) * 0.1f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);

    string titleText = "GAME TITLE";
    int titleWidth = (int)(titleText.length() * 24 * titleScale);
    int titleX = SCREEN_W / 2 - titleWidth / 2;
    int titleY = 200;

    // �^�C�g���̃O���[����
    DrawGlowEffect(titleX - 20, titleY - 10, titleWidth + 40, 60, 0.3f, GetColor(255, 215, 0));
    DrawStringToHandle(titleX, titleY, titleText.c_str(), GetColor(255, 255, 255), largeFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �{�^���`��i���ǔŁj
    for (const auto& btn : buttons) {
        DrawEnhancedButton(btn);
    }

    // �I�v�V������ʕ`��
    if (optionState != Hidden) {
        // �X���C�h�ƃt�F�[�h�𕪗����ēK�p
        float easedSlide = ApplyEasing(optionSlideProgress, EASE_OUT_ELASTIC);
        float easedFade = ApplyEasing(optionFadeProgress, EASE_IN_OUT_CUBIC);

        // �w�i�I�[�o�[���C�i�������Ƃ����t�F�[�h�C���j
        int overlayAlpha = (int)(180 * backgroundFadeProgress);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, overlayAlpha);

        // �O���f�[�V�����w�i�I�[�o�[���C
        for (int i = 0; i < SCREEN_H; i++) {
            float gradientRatio = (float)i / SCREEN_H;
            int lineAlpha = (int)(overlayAlpha * (0.3f + gradientRatio * 0.7f));
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, lineAlpha);
            DrawLine(0, i, SCREEN_W, i, GetColor(0, 0, 30), TRUE);
        }

        // �I�v�V�����E�B���h�E�i������X���C�h�C�� + �t�F�[�h�C���j
        int windowHeight = SCREEN_H - 300;
        int targetY = 150;
        int slideY = SCREEN_H - (int)((SCREEN_H - targetY) * easedSlide);

        // �E�B���h�E�̃t�F�[�h����
        int windowAlpha = (int)(255 * easedFade);

        // �E�B���h�E�̉e���ʁi�t�F�[�h�ƘA���j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(120 * easedFade));
        DrawBox(350, slideY + 15, SCREEN_W - 350, slideY + windowHeight + 15, GetColor(0, 0, 0), TRUE);

        // ���C���E�B���h�E�w�i
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(230 * easedFade));
        DrawBox(400, slideY, SCREEN_W - 400, slideY + windowHeight, GetColor(45, 45, 65), TRUE);

        // �E�B���h�E�g
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
        DrawBox(400, slideY, SCREEN_W - 400, slideY + windowHeight, GetColor(120, 160, 220), FALSE);

        // �I�v�V�����^�C�g���i�t�F�[�h�C���j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
        DrawStringToHandle(SCREEN_W / 2 - 80, slideY + 30, "OPTIONS", GetColor(255, 255, 255), largeFontHandle);

        // ���ʐݒ胉�x���i�t�F�[�h�C���j
        DrawStringToHandle(450, slideY + 120, "Master Volume", GetColor(200, 200, 200), fontHandle);

        // Backspace�L�[�̐����e�L�X�g
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * easedFade));
        DrawStringToHandle(450, slideY + windowHeight - 60, "Press [Backspace] to return", GetColor(150, 150, 150), fontHandle);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);

        // ���ǂ��ꂽ�X���C�_�[�`��i�ʒu���X���C�h�ɍ��킹�čX�V�j
        slider.y = slideY + 160;
        DrawEnhancedSlider();

        // ���ʐ��l�\���i�t�F�[�h�C���j
        int volumePercent = (int)(slider.valueDisplay * 100);
        string volumeText = to_string(volumePercent) + "%";
        DrawStringToHandle(slider.x + slider.w + 20, slideY + 155, volumeText.c_str(), GetColor(255, 215, 0), fontHandle);

        // ����{�^���i���ǂ��ꂽ�z�[���A�C�R���A�t�F�[�h�C���j
        int closeX = 450;
        int closeY = slideY + 30;
        bool closeHovered = IsMouseOver(closeX, closeY, 60, 60);

        if (closeHovered && easedFade > 0.8f) { // �t�F�[�h���قڊ������Ă���O���[�\��
            DrawGlowEffect(closeX - 5, closeY - 5, 70, 70, 0.4f * easedFade, GetColor(255, 100, 100));
        }

        float closeScale = closeHovered ? 1.1f : 1.0f;
        int closeSize = (int)(50 * closeScale);
        int closeOffsetX = (60 - closeSize) / 2;
        int closeOffsetY = (60 - closeSize) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
        DrawExtendGraph(
            closeX + closeOffsetX, closeY + closeOffsetY,
            closeX + closeOffsetX + closeSize, closeY + closeOffsetY + closeSize,
            homeHandle, TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // ��ʑJ�ڃt�F�[�h�I�[�o�[���C�i�őO�ʂɕ`��j
    if (transitionState != None) {
        int fadeAlpha = (int)(255 * transitionFadeProgress);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void TitleScene::UpdateButtons()
{
    for (auto& btn : buttons) {
        bool over = IsMouseOver(btn.x, btn.y, btn.w, btn.h);
        btn.hovered = over;

        // �X�P�[���A�j���[�V�����i�C�[�W���O�K�p�j
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        btn.scale = Lerp(btn.scale, targetScale, SCALE_LERP);

        // �t���[�g����
        btn.floatPhase += FLOAT_SPEED;

        // �O���[����
        float targetGlow = over ? 1.0f : 0.0f;
        btn.glowIntensity = Lerp(btn.glowIntensity, targetGlow, 0.15f);

        // �v���X����
        if (over && mousePressed) {
            btn.pressProgress = min(1.0f, btn.pressProgress + 0.2f);
        }
        else {
            btn.pressProgress = max(0.0f, btn.pressProgress - 0.15f);
        }
    }

    // �{�^���N���b�N�����i��ʑJ�ڒ��łȂ��ꍇ�̂݁j
    if (optionState == Hidden && transitionState == None) {
        // START
        if (IsClicked(buttons[0].x, buttons[0].y, buttons[0].w, buttons[0].h)) {
            transitionState = FadingOut;
            transitionFadeProgress = 0.0f;
        }
        // OPTIONS
        if (IsClicked(buttons[1].x, buttons[1].y, buttons[1].w, buttons[1].h)) {
            optionState = Showing;
            optionAnimProgress = 0.0f;
        }
        // EXIT
        if (IsClicked(buttons[2].x, buttons[2].y, buttons[2].w, buttons[2].h)) {
            exitRequested = true;
        }
    }

    // �I�v�V��������i��ʑJ�ڒ��łȂ��ꍇ�̂݁j
    if (optionState == Visible && transitionState == None) {
        int slideY = 150; // Visible��Ԃł͌Œ�ʒu
        if (IsClicked(450, slideY + 30, 60, 60)) {
            optionState = Hiding;
        }
    }
}

void TitleScene::UpdateSlider()
{
    int knobSize = 32;
    float knobX = slider.x + slider.value * slider.w - knobSize / 2;
    float knobY = slider.y + slider.h / 2 - knobSize / 2;

    bool knobHovered = IsMouseOver((int)knobX, (int)knobY, knobSize, knobSize);
    slider.hoverProgress = Lerp(slider.hoverProgress, knobHovered ? 1.0f : 0.0f, 0.2f);

    // �h���b�O�J�n
    if (!slider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        slider.dragging = true;
    }

    // �h���b�O���̒l�X�V
    if (slider.dragging) {
        float newValue = (float)(mouseX - slider.x) / slider.w;
        slider.value = clamp(newValue, 0.0f, 1.0f);
        slider.glowEffect = min(1.0f, slider.glowEffect + 0.1f);
    }
    else {
        slider.glowEffect = max(0.0f, slider.glowEffect - 0.05f);
    }

    // �h���b�O�I��
    if (!mousePressed) {
        slider.dragging = false;
    }

    // �\���l�̊��炩�ȕ��
    slider.valueDisplay = Lerp(slider.valueDisplay, slider.value, 0.25f);
}

void TitleScene::UpdateParticles()
{
    for (auto& p : particles) {
        if (!p.active) continue;

        p.x += p.vx;
        p.y += p.vy;
        p.life -= 0.016f; // 60FPS�z��

        if (p.life <= 0 || p.y < -50) {
            p.active = false;
        }
    }
}

void TitleScene::SpawnParticle(float x, float y)
{
    for (auto& p : particles) {
        if (p.active) continue;

        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

        p.x = x;
        p.y = y;
        p.vx = dist(randomEngine) * 0.5f;
        p.vy = -1.0f - abs(dist(randomEngine)) * 2.0f;
        p.life = 3.0f + dist(randomEngine) * 2.0f;
        p.maxLife = p.life;
        p.size = 2.0f + abs(dist(randomEngine)) * 3.0f;

        // �S�[���f���J���[�̃o���G�[�V����
        int r = 200 + (rand() % 55);
        int g = 150 + (rand() % 105);
        int b = 50 + (rand() % 100);
        p.color = GetColor(r, g, b);
        p.active = true;
        break;
    }
}

void TitleScene::DrawEnhancedButton(const UIButton& btn)
{
    float yOffset = btn.hovered ? sinf(btn.floatPhase) * FLOAT_AMP : 0.0f;
    float pressOffset = btn.pressProgress * 3.0f;

    float finalScale = btn.scale * (1.0f - btn.pressProgress * 0.05f);
    float sw = btn.w * finalScale;
    float sh = btn.h * finalScale;
    float dx = btn.x - (sw - btn.w) * 0.5f;
    float dy = btn.y + yOffset + pressOffset - (sh - btn.h) * 0.5f;

    // �O���[����
    if (btn.glowIntensity > 0.01f) {
        DrawGlowEffect((int)dx - 10, (int)dy - 10, (int)sw + 20, (int)sh + 20,
            btn.glowIntensity * 0.6f, GetColor(255, 215, 0));
    }

    // �{�^���{��
    DrawExtendGraph((int)dx, (int)dy, (int)(dx + sw), (int)(dy + sh), buttonHandle, TRUE);

    // �e�L�X�g�`��i���ǂ��ꂽ�Z���^�����O�j
    int textWidth = GetDrawStringWidthToHandle(btn.label.c_str(), (int)btn.label.length(), fontHandle);
    int textX = btn.x + btn.w / 2 - textWidth / 2;
    int textY = (int)(btn.y + yOffset + pressOffset + btn.h / 2 - 14);

    // �e�L�X�g�̉e����
    DrawStringToHandle(textX + 2, textY + 2, btn.label.c_str(), GetColor(20, 20, 20), fontHandle);
    DrawStringToHandle(textX, textY, btn.label.c_str(), GetColor(50, 50, 50), fontHandle);
}

void TitleScene::DrawEnhancedSlider()
{
    // �X���C�_�[�o�[�摜���g�p�islide_horizontal_color.png�j
    int barWidth = slider.w;
    int barHeight = slider.h;

    // �X���C�_�[�o�[�S�̂�`��
    DrawExtendGraph(slider.x, slider.y, slider.x + barWidth, slider.y + barHeight,
        slideBarHandle, TRUE);

    // �l�̕������Â����ăR���g���X�g��t����i�I�v�V�����j
    int valueWidth = (int)(slider.valueDisplay * slider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(slider.x, slider.y, slider.x + valueWidth, slider.y + slider.h,
        GetColor(255, 215, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �m�u�i���ǔŁj
    int knobSize = 32;
    float knobX = slider.x + slider.valueDisplay * slider.w - knobSize / 2;
    float knobY = slider.y + slider.h / 2 - knobSize / 2;

    // �z�o�[���̊g�����
    float hoverScale = 1.0f + slider.hoverProgress * 0.2f;
    int scaledKnobSize = (int)(knobSize * hoverScale);
    int scaleOffset = (scaledKnobSize - knobSize) / 2;

    // �m�u�̃O���[����
    if (slider.glowEffect > 0.01f || slider.hoverProgress > 0.01f) {
        float totalGlow = max(slider.glowEffect, slider.hoverProgress * 0.5f);
        DrawGlowEffect((int)knobX - scaleOffset - 5, (int)knobY - scaleOffset - 5,
            scaledKnobSize + 10, scaledKnobSize + 10, totalGlow, GetColor(255, 215, 0));
    }

    // �m�u�{��
    DrawExtendGraph(
        (int)knobX - scaleOffset, (int)knobY - scaleOffset,
        (int)knobX - scaleOffset + scaledKnobSize, (int)knobY - scaleOffset + scaledKnobSize,
        slideHandle, TRUE
    );
}

void TitleScene::DrawParticles()
{
    for (const auto& p : particles) {
        if (!p.active) continue;

        float alpha = p.life / p.maxLife;
        int drawAlpha = (int)(255 * alpha * 0.7f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, drawAlpha);

        DrawCircle((int)p.x, (int)p.y, (int)p.size, p.color, TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void TitleScene::DrawGlowEffect(int x, int y, int w, int h, float intensity, int color)
{
    if (intensity <= 0.01f) return;

    int alpha = (int)(intensity * 100);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // �����̃��C���[�ŃO���[��`��
    for (int i = 0; i < 3; i++) {
        int offset = (i + 1) * 3;
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

// ���[�e�B���e�B�֐��̎���
float TitleScene::EaseInOutCubic(float t)
{
    return t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
}

float TitleScene::EaseOutElastic(float t)
{
    const float c4 = (2 * 3.14159265359f) / 3;
    return t == 0 ? 0 : t == 1 ? 1 : powf(2, -10 * t) * sinf((t * 10 - 0.75f) * c4) + 1;
}

float TitleScene::EaseInOutQuart(float t)
{
    return t < 0.5f ? 8 * t * t * t * t : 1 - powf(-2 * t + 2, 4) / 2;
}

float TitleScene::ApplyEasing(float t, EaseType type)
{
    switch (type) {
    case EASE_IN_OUT_CUBIC: return EaseInOutCubic(t);
    case EASE_OUT_ELASTIC: return EaseOutElastic(t);
    case EASE_IN_OUT_QUART: return EaseInOutQuart(t);
    default: return t;
    }
}

float TitleScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float TitleScene::SmoothStep(float edge0, float edge1, float x)
{
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

int TitleScene::BlendColor(int color1, int color2, float ratio)
{
    int r1, g1, b1, r2, g2, b2;
    GetColor2(color1, &r1, &g1, &b1);
    GetColor2(color2, &r2, &g2, &b2);

    int r = (int)(r1 + (r2 - r1) * ratio);
    int g = (int)(g1 + (g2 - g1) * ratio);
    int b = (int)(b1 + (b2 - b1) * ratio);

    return GetColor(r, g, b);
}

bool TitleScene::IsMouseOver(int x, int y, int w, int h) const
{
    return mouseX >= x && mouseX <= x + w &&
        mouseY >= y && mouseY <= y + h;
}

bool TitleScene::IsClicked(int x, int y, int w, int h) const
{
    return IsMouseOver(x, y, w, h) && mousePressed && !mousePressedPrev;
}