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
{
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

    // �{�^��������
    retryButton = {
        1920 / 2 - 180, 700, 160, 60,
        "RETRY", BUTTON_SCALE_NORMAL, 0.0f, false, true, 0.0f
    };

    nextStageButton = {
        1920 / 2 + 20, 700, 160, 60,
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

    // �A�j���[�V�����X�V
    UpdateAnimation();

    // �{�^���X�V
    if (resultState == RESULT_VISIBLE) {
        UpdateButtons();
    }
}

void ResultUISystem::UpdateAnimation()
{
    switch (resultState) {
    case RESULT_SHOWING:
        // **�i�K1: ���C���p�l���̃A�j���[�V����**
        animationProgress += ANIM_SPEED;
        if (animationProgress >= 1.0f) {
            animationProgress = 1.0f;
        }

        // **�i�K2: ���̃A�j���[�V����**
        if (animationProgress > 0.6f) {
            starsAnimProgress += STAR_ANIM_SPEED * 0.4f;
            // **�d�v: 1.0f�Ŏ~�߂Ȃ��I�p��������**
        }

        // **�i�K3: �{�^���t�F�[�h�C��**
        if (starsAnimProgress >= 1.0f && !buttonsVisible) {
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
        // **�d�v: �S�Ă̏�ԂŌp���I�ɃA�j���[�V�����X�V**
        starsAnimProgress += STAR_ANIM_SPEED * 0.2f; // �p���I�ɑ���
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
        }
        break;

    default:
        break;
    }
}

void ResultUISystem::UpdateButtons()
{
    // **�{�^�����\������Ă��Ȃ��ꍇ�͍X�V���Ȃ�**
    if (!buttonsVisible || buttonsAlpha < 0.1f) {
        return;
    }

    // ���g���C�{�^��
    bool retryHovered = IsMouseOver(retryButton.x, retryButton.y, retryButton.w, retryButton.h);
    retryButton.hovered = retryHovered;
    retryButton.scale = Lerp(retryButton.scale,
        retryHovered ? BUTTON_SCALE_HOVER : BUTTON_SCALE_NORMAL, 0.2f);
    retryButton.glowIntensity = Lerp(retryButton.glowIntensity,
        retryHovered ? 1.0f : 0.0f, 0.15f);

    // �l�N�X�g�X�e�[�W�{�^��
    bool nextHovered = IsMouseOver(nextStageButton.x, nextStageButton.y, nextStageButton.w, nextStageButton.h);
    nextStageButton.hovered = nextHovered;
    nextStageButton.scale = Lerp(nextStageButton.scale,
        nextHovered ? BUTTON_SCALE_HOVER : BUTTON_SCALE_NORMAL, 0.2f);
    nextStageButton.glowIntensity = Lerp(nextStageButton.glowIntensity,
        nextHovered ? 1.0f : 0.0f, 0.15f);

    // �{�^���̃A�j���[�V�����i�s
    retryButton.animProgress = min(1.0f, retryButton.animProgress + 0.1f);
    nextStageButton.animProgress = min(1.0f, nextStageButton.animProgress + 0.1f);
}

void ResultUISystem::Draw()
{
    if (resultState == RESULT_HIDDEN) return;

    // �w�i
    DrawBackground();

    // �^�C�g��
    DrawTitle();

    // ��
    DrawStars();

    // �{�^��
    DrawButtons();
}

void ResultUISystem::DrawBackground()
{
    float easedProgress = EaseInOutCubic(animationProgress);
    int bgAlpha = (int)(200 * easedProgress);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, bgAlpha);

    // �O���f�[�V�����w�i
    for (int i = 0; i < 1080; i++) {
        float gradientRatio = (float)i / 1080;
        int lineAlpha = (int)(bgAlpha * (0.4f + gradientRatio * 0.6f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, lineAlpha);
        DrawLine(0, i, 1920, i, GetColor(0, 0, 40), TRUE);
    }

    // ���C���p�l��
    int panelWidth = 600;
    int panelHeight = 400;
    int panelX = 1920 / 2 - panelWidth / 2;
    int panelY = 200 + (int)((1.0f - easedProgress) * 100); // �ォ�牺�ɃX���C�h

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(240 * easedProgress));
    DrawBox(panelX, panelY, panelX + panelWidth, panelY + panelHeight,
        GetColor(50, 50, 80), TRUE);

    // �p�l���̘g
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * easedProgress));
    DrawBox(panelX, panelY, panelX + panelWidth, panelY + panelHeight,
        GetColor(120, 160, 220), FALSE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawTitle()
{
    float easedProgress = EaseOutBack(animationProgress);

    string titleText = "STAGE " + to_string(currentStageNumber) + " CLEAR!";
    int titleWidth = GetDrawStringWidthToHandle(titleText.c_str(), titleText.length(), largeFontHandle);
    int titleX = 1920 / 2 - titleWidth / 2;
    int titleY = 250;

    // �^�C�g���̃X�P�[���A�j���[�V����
    float titleScale = easedProgress;
    int alpha = (int)(255 * easedProgress);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // �O���[����
    if (easedProgress > 0.5f) {
        DrawGlowEffect(titleX - 20, titleY - 10, titleWidth + 40, 60,
            (easedProgress - 0.5f) * 2.0f, GetColor(255, 215, 0));
    }

    // �^�C�g���e�L�X�g
    DrawStringToHandle(titleX, titleY, titleText.c_str(),
        GetColor(255, 255, 255), largeFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawStars()
{
    if (animationProgress < 0.4f) return;

    int starSize = 80;
    int starSpacing = 120;
    int startX = 1920 / 2 - (3 * starSpacing) / 2 + starSpacing / 2;
    int starY = 380;

    for (int i = 0; i < 3; i++) {
        int starX = startX + i * starSpacing;

        // **���̓o��i�s�x�v�Z**
        float starDelay = i * 0.5f;
        float starProgress = max(0.0f, (starsAnimProgress - starDelay) * 1.2f);
        // **�d�v: max�l��ݒ肵�Ȃ��i�p���I�ȑ��������j**

        if (starProgress <= 0.0f) continue;

        // **���A�j���[�V�����`��**
        if (i < displayedStars) {
            DrawCollectedStarAnimation(starX, starY, starSize, starProgress, i);
        }
        else {
            DrawUnCollectedStar(starX, starY, starSize, starProgress);
        }
    }

    // ���̐��\��
    if (starsAnimProgress > 1.0f) {
        string starsText = to_string(displayedStars) + " / 3 STARS";
        int starsWidth = GetDrawStringWidthToHandle(starsText.c_str(), starsText.length(), fontHandle);
        int starsX = 1920 / 2 - starsWidth / 2;
        int starsY = starY + starSize + 30;

        float textAlpha = min(1.0f, (starsAnimProgress - 1.0f) * 2.0f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * textAlpha));
        DrawStringToHandle(starsX, starsY, starsText.c_str(),
            GetColor(255, 215, 0), fontHandle);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void ResultUISystem::DrawCollectedStarAnimation(int x, int y, int baseSize, float progress, int starIndex)
{
    // **1. �o��A�j���[�V����**
    float appearScale = 1.0f;
    if (progress <= 1.0f) {
        appearScale = EaseOutBack(min(1.0f, progress));
    }

    // **2. �o�E���h�A�j���[�V�����i3��̂݁A���S��~�j**
    float bounceScale = 1.0f;

    if (progress <= 1.0f) {
        // **�o�ꎞ�̏����o�E���X**
        if (progress < 0.6f) {
            float expandProgress = progress / 0.6f;
            bounceScale = 1.0f + EaseOutBack(expandProgress) * 0.5f; // 1.0 �� 1.5�{
        }
        else {
            float shrinkProgress = (progress - 0.6f) / 0.4f;
            bounceScale = 1.5f - EaseInOutCubic(shrinkProgress) * 0.5f; // 1.5 �� 1.0�{
        }
    }
    else if (progress > 1.0f && progress <= 7.0f) {
        // **3��̃o�E���X�iprogress 1.0�`7.0�̊ԂŎ��s�j**
        float bounceTime = progress - 1.0f; // 0�`6�͈̔�
        float bouncePhase = bounceTime * 1.047f; // 6�b��3��o�E���X

        if (bouncePhase <= 6.28f) { // 3��̃T�C���g
            float bounceValue = sinf(bouncePhase);
            if (bounceValue > 0) {
                // �o�E���X�̋��x���񐔂ɉ����Č���
                float bounceNumber = floor(bouncePhase / 2.094f); // ���݉���ڂ̃o�E���X�� (0,1,2)
                float decayFactor = 1.0f - (bounceNumber * 0.25f); // 1.0, 0.75, 0.5�Ɍ���
                bounceScale = 1.0f + bounceValue * 0.25f * decayFactor;
            }
        }
        // **3�񊮗���� bounceScale = 1.0f (���̃T�C�Y�Œ�~)**
    }
    // **progress > 7.0f �̏ꍇ�� bounceScale = 1.0f �̂܂܁i���S��~�j**

    // **3. �ŏI�X�P�[���v�Z**
    float finalScale = appearScale * bounceScale;
    int finalSize = (int)(baseSize * finalScale);

    // **4. �����x�v�Z�i��]�Ȃ��j**
    int alpha = 255;
    if (progress <= 1.0f) {
        alpha = (int)(255 * progress);
    }

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

    // **5. ���̕`��i��]�Ȃ��A�V���v���j**
    int starX = x - finalSize / 2;
    int starY = y - finalSize / 2;

    DrawExtendGraph(starX, starY,
        starX + finalSize, starY + finalSize,
        starFilledTexture, TRUE);

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **6. �o�E���X���̌y���O���[�i3��̃o�E���X���̂݁j**
    if (bounceScale > 1.03f && progress > 1.0f && progress <= 7.0f) {
        float glowIntensity = (bounceScale - 1.0f) * 4.0f;
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(25 * glowIntensity));

        int glowSize = finalSize + 4;
        int glowX = x - glowSize / 2;
        int glowY = y - glowSize / 2;

        DrawBox(glowX, glowY, glowX + glowSize, glowY + glowSize,
            GetColor(255, 230, 120), FALSE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void ResultUISystem::DrawUnCollectedStar(int x, int y, int baseSize, float progress)
{
    // **�o��A�j���[�V����**
    float appearScale = 1.0f;
    if (progress <= 1.0f) {
        appearScale = EaseOutBack(min(1.0f, progress));
    }

    // **1�񂾂��̌y���o�E���X�A���̌㊮�S��~**
    float bounceScale = 1.0f;
    if (progress > 1.0f && progress <= 3.0f) {
        // **1�񂾂��̌y���o�E���X**
        float bounceTime = progress - 1.0f; // 0�`2�͈̔�
        float bouncePhase = bounceTime * 3.14f; // 2�b��1��̃T�C���g

        if (bouncePhase <= 3.14f) {
            float bounceValue = sinf(bouncePhase);
            if (bounceValue > 0) {
                bounceScale = 1.0f + bounceValue * 0.12f; // �T���߂�12%�o�E���X
            }
        }
    }
    // **progress > 3.0f �̏ꍇ�� bounceScale = 1.0f �̂܂܁i���S��~�j**

    float finalScale = appearScale * bounceScale;
    int finalSize = (int)(baseSize * finalScale);

    // **�����x�v�Z**
    int alpha = 255;
    if (progress <= 1.0f) {
        alpha = (int)(255 * progress);
    }

    // **�A�E�g���C�����̕`��i��]�Ȃ��A�V���v���j**
    int starX = x - finalSize / 2;
    int starY = y - finalSize / 2;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawExtendGraph(starX, starY,
        starX + finalSize, starY + finalSize,
        starOutlineTexture, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawButtons()
{
    // **�{�^�����\������Ă��Ȃ��A�܂��͓����x���Ⴂ�ꍇ�̓X�L�b�v**
    if (!buttonsVisible || buttonsAlpha < 0.01f) return;

    // **�t�F�[�h���ʂ�K�p**
    int buttonAlpha = (int)(255 * buttonsAlpha);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, buttonAlpha);

    // **�{�^���̃t�F�[�h�C�����̏ォ��X���C�h����**
    float slideProgress = buttonsAlpha; // 0.0�`1.0
    float slideOffset = (1.0f - EaseOutBack(slideProgress)) * 50.0f; // 50�s�N�Z���ォ��

    // ���g���C�{�^���i�t�F�[�h�{�X���C�h�j
    UIButton tempRetryButton = retryButton;
    tempRetryButton.y -= (int)slideOffset;
    DrawButtonWithAlpha(tempRetryButton, buttonAlpha);

    // �l�N�X�g�{�^���i�t�F�[�h�{�X���C�h�A�����x���j
    float nextSlideProgress = max(0.0f, buttonsAlpha - 0.2f) / 0.8f; // �����x��ĊJ�n
    float nextSlideOffset = (1.0f - EaseOutBack(nextSlideProgress)) * 50.0f;

    UIButton tempNextButton = nextStageButton;
    tempNextButton.y -= (int)nextSlideOffset;
    DrawButtonWithAlpha(tempNextButton, (int)(255 * nextSlideProgress));

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::DrawButtonWithAlpha(const UIButton& button, int alpha)
{
    if (alpha <= 0) return;

    float scale = button.scale;
    int buttonWidth = (int)(button.w * scale);
    int buttonHeight = (int)(button.h * scale);
    int buttonX = button.x - (buttonWidth - button.w) / 2;
    int buttonY = button.y - (buttonHeight - button.h) / 2;

    // �O���[���ʁi�����x�K�p�j
    if (button.glowIntensity > 0.01f) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(button.glowIntensity * 100 * alpha / 255));
        for (int i = 0; i < 3; i++) {
            int offset = (i + 1) * 4;
            DrawBox(buttonX - offset, buttonY - offset,
                buttonX + buttonWidth + offset, buttonY + buttonHeight + offset,
                GetColor(255, 215, 0), FALSE);
        }
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    }

    // �{�^���w�i�i�����x�K�p�j
    DrawExtendGraph(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight,
        buttonTexture, TRUE);

    // �{�^���e�L�X�g�i�����x�K�p�j
    int textWidth = GetDrawStringWidthToHandle(button.label.c_str(), button.label.length(), fontHandle);
    int textX = button.x + button.w / 2 - textWidth / 2;
    int textY = button.y + button.h / 2 - 14;

    // �e�L�X�g�̉e���ʁi�����x�K�p�j
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha / 3); // �e�͈Â�
    DrawStringToHandle(textX + 2, textY + 2, button.label.c_str(), GetColor(20, 20, 20), fontHandle);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawStringToHandle(textX, textY, button.label.c_str(), GetColor(50, 50, 50), fontHandle);
}

void ResultUISystem::DrawButton(const UIButton& button)
{
    float scale = button.scale;
    int buttonWidth = (int)(button.w * scale);
    int buttonHeight = (int)(button.h * scale);
    int buttonX = button.x - (buttonWidth - button.w) / 2;
    int buttonY = button.y - (buttonHeight - button.h) / 2;

    // �O���[����
    if (button.glowIntensity > 0.01f) {
        DrawGlowEffect(buttonX - 10, buttonY - 10, buttonWidth + 20, buttonHeight + 20,
            button.glowIntensity * 0.6f, GetColor(255, 215, 0));
    }

    // �{�^���w�i
    DrawExtendGraph(buttonX, buttonY, buttonX + buttonWidth, buttonY + buttonHeight,
        buttonTexture, TRUE);

    // �{�^���e�L�X�g
    int textWidth = GetDrawStringWidthToHandle(button.label.c_str(), button.label.length(), fontHandle);
    int textX = button.x + button.w / 2 - textWidth / 2;
    int textY = button.y + button.h / 2 - 14;

    // �e�L�X�g�̉e����
    DrawStringToHandle(textX + 2, textY + 2, button.label.c_str(), GetColor(20, 20, 20), fontHandle);
    DrawStringToHandle(textX, textY, button.label.c_str(), GetColor(50, 50, 50), fontHandle);
}

void ResultUISystem::DrawGlowEffect(int x, int y, int w, int h, float intensity, int color)
{
    if (intensity <= 0.01f) return;

    int alpha = (int)(intensity * 100);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    for (int i = 0; i < 3; i++) {
        int offset = (i + 1) * 4;
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void ResultUISystem::ShowResult(int starsCollected, int totalStars, int currentStage)
{
    displayedStars = starsCollected;
    totalDisplayStars = totalStars;
    currentStageNumber = currentStage;
    resultState = RESULT_SHOWING;
    animationProgress = 0.0f;
    starsAnimProgress = 0.0f;

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
    // **���S�ȏ�ԃ��Z�b�g**
    resultState = RESULT_HIDDEN;
    animationProgress = 0.0f;
    starsAnimProgress = 0.0f;
    buttonsAlpha = 0.0f;
    buttonsVisible = false;
    displayedStars = 0;
    totalDisplayStars = 0;
    currentStageNumber = 1;

    // �{�^����Ԃ����Z�b�g
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

float ResultUISystem::EaseOutBack(float t)
{
    const float c1 = 1.70158f;
    const float c3 = c1 + 1;
    return 1 + c3 * powf(t - 1, 3) + c1 * powf(t - 1, 2);
}

float ResultUISystem::EaseInOutCubic(float t)
{
    return t < 0.5f ? 4 * t * t * t : 1 - powf(-2 * t + 2, 3) / 2;
}

float ResultUISystem::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}