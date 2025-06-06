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
    , alienGlowPhase(0.0f)          // **�V�ǉ�**
    , starFieldPhase(0.0f)          // **�V�ǉ�**
    , titleColorPhase(0.0f)         // **�V�ǉ�**
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


    // **BGM�J�n�i�^�C�g����ʂƃI�v�V������ʂŌp���Đ��j**
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);


    buttons = {
        { buttonX, 550, BUTTON_W, BUTTON_H, "START",   NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
        { buttonX, 690, BUTTON_W, BUTTON_H, "OPTIONS", NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
        { buttonX, 830, BUTTON_W, BUTTON_H, "EXIT",    NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f }
    };

    // BGM�X���C�_�[�ݒ�i������slider�̐ݒ�̌�ɒǉ��j
    bgmSlider = {
        500, 0, 800, 24,  // x, y, w, h (y�͌�œ��I�ɐݒ�)
        0.7f,             // value (BGM�����l)
        false,            // dragging
        0.0f,             // hoverProgress
        0.7f,             // valueDisplay
        0.0f              // glowEffect
    };

    seSlider = {
        500, 0, 800, 24,  // x, y, w, h (y�͌�œ��I�ɐݒ�)
        0.8f,             // value (SE�����l)
        false,            // dragging
        0.0f,             // hoverProgress
        0.8f,             // valueDisplay
        0.0f              // glowEffect
    };

    // **SoundManager�̌��݂̉��ʐݒ���擾���ăX���C�_�[�ɔ��f**
    bgmSlider.value = SoundManager::GetInstance().GetBGMVolume();
    bgmSlider.valueDisplay = bgmSlider.value;

    seSlider.value = SoundManager::GetInstance().GetSEVolume();
    seSlider.valueDisplay = seSlider.value;
}

// Update�֐��̉��ǔŁi�R�����g�t���j
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

    // **�d�v�F�^�C�g��BGM�̌p���Đ���ۏ�**
    // �^�C�g����ʂƃI�v�V������ʂ̗�����BGM�����ꑱ����悤�ɊǗ�
    EnsureTitleBGMPlaying();

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

            // **�I�v�V������ʂ����S�ɕ\�����ꂽ**
            OutputDebugStringA("TitleScene: Options fully visible - BGM playing\n");
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

            // **�I�v�V������ʂ����S�ɔ�\���ɂȂ���**
            OutputDebugStringA("TitleScene: Options hidden - BGM continues\n");
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

        // **Backspace�ŃI�v�V���������FBGM�͌p��**
        OutputDebugStringA("TitleScene: Options closed by Backspace - BGM continues\n");
    }

    // �X���C�_�[�X�V�i�I�v�V������ʂ��\������Ă��鎞�̂݁j
    if (optionState != Hidden && transitionState == None) {
        UpdateBGMSlider();
        UpdateSESlider();
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
    // **1. �w�i�`��i�Ŕw�ʁj**
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // **2. �p�[�e�B�N���`��i�w�i�̏�j**
    DrawParticles();

    // **3. �^�C�g�����S�`��i���ӂ�X�^�C���j**
    // ���₩�ȃA�j���[�V��������
    float titleScale = 1.0f + sinf(titlePulsePhase) * 0.03f; // ���T���߂ȃX�P�[���ϓ�
    int titleAlpha = (int)(255 * (0.95f + sinf(titlePulsePhase * 0.8f) * 0.05f)); // ���₩�ȓ����x�ω�

    // **�^�C�g��: "Alien's Days"**
    string titleText = "Alien's Days";

    // ���m�ȕ��v�Z�ŃZ���^�����O
    int baseWidth = GetDrawStringWidthToHandle(titleText.c_str(), (int)titleText.length(), largeFontHandle);
    int titleWidth = (int)(baseWidth * titleScale);
    int titleX = SCREEN_W / 2.23 - baseWidth / 2; // �X�P�[���O�̕��ŃZ���^�����O
    int titleY = 200 + (int)(sinf(titlePulsePhase * 0.6f) * 3.0f); // ���₩�ȕ��V����

    // **���ӂ�Ȕw�i�{�b�N�X�i�p�ە��j**
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawBox(titleX - 50, titleY - 25, titleX + baseWidth + 50, titleY + 85, GetColor(255, 255, 255), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **�\�t�g�ȃO���[���ʁi�����s���N�n�j**
    DrawGlowEffect(titleX - 20, titleY - 10, baseWidth + 40, 70, 0.2f + sinf(titlePulsePhase) * 0.1f, GetColor(255, 200, 220));

    // **���C���^�C�g���`��i�T���߂ȃX�P�[���j**
    int scaledFontSize = (int)(52 * titleScale); // �K�x�ȃT�C�Y
    int tempLargeFontHandle = CreateFontToHandle(NULL, scaledFontSize, 5);

    // �D�����e����
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawStringToHandle(titleX + 3, titleY + 3, titleText.c_str(), GetColor(150, 150, 150), tempLargeFontHandle);

    // ���C���e�L�X�g�i�����������F�����j
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);
    DrawStringToHandle(titleX, titleY, titleText.c_str(), GetColor(80, 120, 160), tempLargeFontHandle); // �D�����F

    // **�T�u�^�C�g���i���ӂ�e�[�}�j**
    string subTitle = "~ A Gentle Adventure ~";
    int subTitleWidth = GetDrawStringWidthToHandle(subTitle.c_str(), (int)subTitle.length(), fontHandle);
    int subTitleX = SCREEN_W / 2.23 - subTitleWidth / 2;
    int subTitleY = titleY + 70;

    // �T�u�^�C�g���`��i�D�����A�j���[�V�����j
    int subAlpha = (int)(180 * (0.8f + sinf(titlePulsePhase * 0.4f) * 0.2f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, subAlpha);
    DrawStringToHandle(subTitleX, subTitleY, subTitle.c_str(), GetColor(120, 150, 100), fontHandle); // �D�����ΐF

    DeleteFontToHandle(tempLargeFontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **4. �{�^���`��i���ǔŁj**
    for (const auto& btn : buttons) {
        DrawEnhancedButton(btn);
    }

    // **5. �I�v�V������ʕ`��i���ǂ��ꂽ�A�j���[�V�����j**
    if (optionState != Hidden) {
        // ���������ꂽ�C�[�W���O�K�p
        float easedSlide = ApplyEasing(optionSlideProgress, EASE_OUT_ELASTIC);
        float easedFade = ApplyEasing(optionFadeProgress, EASE_IN_OUT_CUBIC);

        // **���ǂ��ꂽ�w�i�I�[�o�[���C�i�i�K�I�t�F�[�h�j**
        float overlayProgress = backgroundFadeProgress;

        // �w�i�u���[���ʂ̃V�~�����[�V�����i�������C���[�j
        for (int layer = 0; layer < 3; layer++) {
            int layerAlpha = (int)(60 * overlayProgress * (1.0f - layer * 0.2f));
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, layerAlpha);

            // �����ɂ��炵����`�Ő[�x�������o
            int offset = layer * 2;
            DrawBox(offset, offset, SCREEN_W - offset, SCREEN_H - offset,
                GetColor(20 + layer * 10, 20 + layer * 10, 40 + layer * 15), TRUE);
        }

        // **�r�l�b�g���ʁi��ʎ��ӂ��Â��j**
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(80 * overlayProgress));
        for (int i = 0; i < 100; i++) {
            float ratio = (float)i / 100.0f;
            int alpha = (int)(80 * overlayProgress * ratio);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            int thickness = (int)(100 - i);
            DrawBox(0, 0, thickness, SCREEN_H, GetColor(0, 0, 0), TRUE);                    // ��
            DrawBox(SCREEN_W - thickness, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE); // �E
            DrawBox(0, 0, SCREEN_W, thickness, GetColor(0, 0, 0), TRUE);                   // ��
            DrawBox(0, SCREEN_H - thickness, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE); // ��
        }

        // **���ǂ��ꂽ�E�B���h�E�A�j���[�V����**
        int windowHeight = SCREEN_H - 300;
        int targetY = 150;

        // ��莩�R�Ȉړ��O���i�C�[�Y�A�E�g�o�b�N�j
        float slideT = easedSlide;
        float backAmount = 0.1f;
        if (slideT > 0.8f) {
            float overT = (slideT - 0.8f) / 0.2f;
            slideT = 0.8f + 0.2f * (1.0f - powf(1.0f - overT, 3.0f));
        }

        int slideY = SCREEN_H - (int)((SCREEN_H - targetY) * slideT);

        // **���i���ȉe���ʁi���d�e�j**
        for (int shadowLayer = 0; shadowLayer < 5; shadowLayer++) {
            int shadowOffset = (shadowLayer + 1) * 3;
            int shadowAlpha = (int)((80 - shadowLayer * 15) * easedFade);

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, shadowAlpha);
            DrawBox(400 + shadowOffset, slideY + shadowOffset,
                SCREEN_W - 400 + shadowOffset, slideY + windowHeight + shadowOffset,
                GetColor(0, 0, 0), TRUE);
        }

        // **�E�B���h�E�̒i�K�I�\���i�X�P�[�����ʕt���j**
        float scaleEffect = 0.9f + easedFade * 0.1f; // �������X�^�[�g���Ċg��
        int windowW = (int)((SCREEN_W - 800) * scaleEffect);
        int windowH = (int)(windowHeight * scaleEffect);
        int windowX = 400 + (SCREEN_W - 800 - windowW) / 2;
        int windowYScaled = slideY + (windowHeight - windowH) / 2;

        // **�O���f�[�V�����w�i�i���i���j**
        for (int i = 0; i < windowH; i++) {
            float gradientRatio = (float)i / windowH;
            int r = (int)(45 + gradientRatio * 10);
            int g = (int)(45 + gradientRatio * 10);
            int b = (int)(65 + gradientRatio * 15);
            int alpha = (int)(230 * easedFade * (0.8f + gradientRatio * 0.2f));

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawLine(windowX, windowYScaled + i, windowX + windowW, windowYScaled + i,
                GetColor(r, g, b), TRUE);
        }

        // **������ʁi�㕔�n�C���C�g�j**
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 * easedFade));
        for (int i = 0; i < 30; i++) {
            float highlightRatio = (float)i / 30.0f;
            int alpha = (int)(40 * easedFade * (1.0f - highlightRatio));
            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawLine(windowX, windowYScaled + i, windowX + windowW, windowYScaled + i,
                GetColor(120, 160, 220), TRUE);
        }

        // **�������ꂽ�E�B���h�E�g�i���d�g�j**
        int windowAlpha = (int)(255 * easedFade);

        // �O�g�i�Â��j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
        DrawBox(windowX - 2, windowYScaled - 2, windowX + windowW + 2, windowYScaled + windowH + 2,
            GetColor(60, 80, 120), FALSE);

        // ���g�i���邢�j
        DrawBox(windowX, windowYScaled, windowX + windowW, windowYScaled + windowH,
            GetColor(120, 160, 220), FALSE);

        // **�^�C�g���ƃR���e���c�̒i�K�I�t�F�[�h�C��**
        float contentFade = max(0.0f, (easedFade - 0.3f) / 0.7f); // �t�F�[�h��30%�������Ă���R���e���c�\��

        // �I�v�V�����^�C�g���i�t�F�[�h�C�� + �y���X�P�[�����ʁj
        float titleScale = 0.95f + contentFade * 0.05f;
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(windowAlpha * contentFade));

        string optionsTitle = "OPTIONS";
        int titleWidth = GetDrawStringWidthToHandle(optionsTitle.c_str(), (int)optionsTitle.length(), largeFontHandle);
        int titleX = windowX + windowW / 2 - (int)(titleWidth * titleScale) / 2;
        int titleY = windowYScaled + 30;

        // �^�C�g���̉e
        DrawStringToHandle(titleX + 2, titleY + 2, optionsTitle.c_str(), GetColor(0, 0, 0), largeFontHandle);
        DrawStringToHandle(titleX, titleY, optionsTitle.c_str(), GetColor(255, 255, 255), largeFontHandle);

        // **�R���e���c�G���A�̌v�Z�i�X�P�[����̍��W�j**
        int contentAlpha = (int)(windowAlpha * contentFade);

        // BGM/SE���x���ƃX���C�_�[�i�i�K�I�\���j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, contentAlpha);

        int labelStartY = windowYScaled + 100;
        int sliderStartY = windowYScaled + 140;

        // BGM�Z�N�V����
        DrawStringToHandle(windowX + 50, labelStartY, "BGM Volume", GetColor(200, 200, 200), fontHandle);

        // BGM�X���C�_�[�ʒu����
        bgmSlider.x = windowX + 50;
        bgmSlider.y = sliderStartY;
        bgmSlider.w = windowW - 150;
        DrawBGMSlider();

        // BGM���l�\��
        int bgmPercent = (int)(bgmSlider.valueDisplay * 100);
        string bgmText = to_string(bgmPercent) + "%";
        DrawStringToHandle(bgmSlider.x + bgmSlider.w + 20, sliderStartY - 5, bgmText.c_str(), GetColor(255, 215, 0), fontHandle);

        // SE�Z�N�V����
        int seLabelsY = labelStartY + 80;
        int seSliderY = sliderStartY + 80;

        DrawStringToHandle(windowX + 50, seLabelsY, "SE Volume", GetColor(200, 200, 200), fontHandle);

        // SE�X���C�_�[�ʒu����
        seSlider.x = windowX + 50;
        seSlider.y = seSliderY;
        seSlider.w = windowW - 150;
        DrawSESlider();

        // SE���l�\��
        int sePercent = (int)(seSlider.valueDisplay * 100);
        string seText = to_string(sePercent) + "%";
        DrawStringToHandle(seSlider.x + seSlider.w + 20, seSliderY - 5, seText.c_str(), GetColor(255, 215, 0), fontHandle);

        // **�߂邽�߂̐������i�t�F�[�h�C���j**
        float instructionFade = max(0.0f, (contentFade - 0.5f) / 0.5f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * instructionFade));
        DrawStringToHandle(windowX + 50, windowYScaled + windowH - 60,
            "Press [Backspace] to return", GetColor(150, 150, 150), fontHandle);

        // **����{�^���i�������ꂽ�A�j���[�V�����j**
        int closeX = windowX + windowW - 80;
        int closeY = windowYScaled + 20;
        bool closeHovered = IsMouseOver(closeX, closeY, 60, 60);

        if (closeHovered && contentFade > 0.8f) {
            DrawGlowEffect(closeX - 5, closeY - 5, 70, 70, 0.3f * contentFade, GetColor(255, 120, 120));
        }

        float closeScale = closeHovered ? 1.05f : 1.0f;
        float closeFade = contentFade * (closeHovered ? 1.0f : 0.8f);

        int closeSize = (int)(40 * closeScale);
        int closeOffsetX = (60 - closeSize) / 2;
        int closeOffsetY = (60 - closeSize) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * closeFade));
        DrawExtendGraph(
            closeX + closeOffsetX, closeY + closeOffsetY,
            closeX + closeOffsetX + closeSize, closeY + closeOffsetY + closeSize,
            homeHandle, TRUE
        );

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // **6. ��ʑJ�ڃt�F�[�h�I�[�o�[���C�i�őO�ʁj**
    if (transitionState != None) {
        int fadeAlpha = (int)(255 * transitionFadeProgress);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, fadeAlpha);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}
// UpdateButtons�֐��̉��ǔŁiBGM�������薾�m�Ɂj
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
            // �{�^���N���b�N��
            SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);
        }
        else {
            btn.pressProgress = max(0.0f, btn.pressProgress - 0.15f);
        }
    }

    // �{�^���N���b�N�����i��ʑJ�ڒ��łȂ��ꍇ�̂݁j
    if (optionState == Hidden && transitionState == None) {
        // START �{�^��
        if (IsClicked(buttons[0].x, buttons[0].y, buttons[0].w, buttons[0].h)) {
          

            transitionState = FadingOut;
            transitionFadeProgress = 0.0f;

            OutputDebugStringA("TitleScene: START clicked - BGM stopped for game transition\n");
        }

        // OPTIONS �{�^��
        if (IsClicked(buttons[1].x, buttons[1].y, buttons[1].w, buttons[1].h)) {
            optionState = Showing;
            optionAnimProgress = 0.0f;

            // **�I�v�V������ʁFBGM�͌p���Đ��i�������Ȃ��j**
            OutputDebugStringA("TitleScene: OPTIONS opened - BGM continues\n");
        }

        // EXIT �{�^��
        if (IsClicked(buttons[2].x, buttons[2].y, buttons[2].w, buttons[2].h)) {
            // **�A�v���P�[�V�����I���F�S�Ẳ������~**
            SoundManager::GetInstance().StopBGM();
            SoundManager::GetInstance().StopAllSE();

            exitRequested = true;

            OutputDebugStringA("TitleScene: EXIT clicked - All sounds stopped\n");
        }
    }

    // �I�v�V������ʂ���鏈���i��ʑJ�ڒ��łȂ��ꍇ�̂݁j
    if (optionState == Visible && transitionState == None) {
        int slideY = 150; // Visible��Ԃł͌Œ�ʒu
        if (IsClicked(450, slideY + 30, 60, 60)) {
            optionState = Hiding;

            // **�I�v�V������ʂ����FBGM�͌p��**
            OutputDebugStringA("TitleScene: OPTIONS closed - BGM continues\n");
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
        // �����̃}�X�^�[�{�����[���X�V�̌��
        SoundManager::GetInstance().SetMasterVolume(slider.value);
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

// BGM�X���C�_�[�X�V�֐��̉��ǔ�
void TitleScene::UpdateBGMSlider()
{
    int knobSize = 32;
    float knobX = bgmSlider.x + bgmSlider.value * bgmSlider.w - knobSize / 2;
    float knobY = bgmSlider.y + bgmSlider.h / 2 - knobSize / 2;

    bool knobHovered = IsMouseOver((int)knobX, (int)knobY, knobSize, knobSize);
    bgmSlider.hoverProgress = Lerp(bgmSlider.hoverProgress, knobHovered ? 1.0f : 0.0f, 0.2f);

    // �h���b�O�J�n
    if (!bgmSlider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        bgmSlider.dragging = true;
    }

    // �h���b�O���̒l�X�V
    if (bgmSlider.dragging) {
        float newValue = (float)(mouseX - bgmSlider.x) / bgmSlider.w;
        bgmSlider.value = clamp(newValue, 0.0f, 1.0f);
        bgmSlider.glowEffect = min(1.0f, bgmSlider.glowEffect + 0.1f);

        // **BGM�{�����[�������A���^�C���X�V**
        SoundManager::GetInstance().SetBGMVolume(bgmSlider.value);

        // **�^�C�g��BGM���Đ����̏ꍇ�A�����Ƀ{�����[���ύX��K�p**
        // SetBGMVolume���Ŋ��ɏ�������Ă��邪�A�m���ɂ��邽��
    }
    else {
        bgmSlider.glowEffect = max(0.0f, bgmSlider.glowEffect - 0.05f);
    }

    // �h���b�O�I��
    if (!mousePressed) {
        bgmSlider.dragging = false;
    }

    // �\���l�̊��炩�ȕ��
    bgmSlider.valueDisplay = Lerp(bgmSlider.valueDisplay, bgmSlider.value, 0.25f);
}

void TitleScene::UpdateSESlider()
{
    int knobSize = 32;
    float knobX = seSlider.x + seSlider.value * seSlider.w - knobSize / 2;
    float knobY = seSlider.y + seSlider.h / 2 - knobSize / 2;

    bool knobHovered = IsMouseOver((int)knobX, (int)knobY, knobSize, knobSize);
    seSlider.hoverProgress = Lerp(seSlider.hoverProgress, knobHovered ? 1.0f : 0.0f, 0.2f);

    // �h���b�O�J�n
    if (!seSlider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        seSlider.dragging = true;
    }

    // �h���b�O���̒l�X�V
    if (seSlider.dragging) {
        float newValue = (float)(mouseX - seSlider.x) / seSlider.w;
        seSlider.value = clamp(newValue, 0.0f, 1.0f);
        seSlider.glowEffect = min(1.0f, seSlider.glowEffect + 0.1f);

        // SE�{�����[�������A���^�C���X�V
        SoundManager::GetInstance().SetSEVolume(seSlider.value);
    }
    else {
        seSlider.glowEffect = max(0.0f, seSlider.glowEffect - 0.05f);
    }

    // �h���b�O�I��
    if (!mousePressed) {
        seSlider.dragging = false;
    }

    // �\���l�̊��炩�ȕ��
    seSlider.valueDisplay = Lerp(seSlider.valueDisplay, seSlider.value, 0.25f);
}

void TitleScene::DrawBGMSlider()
{
    // �X���C�_�[�o�[�S�̂�`��
    DrawExtendGraph(bgmSlider.x, bgmSlider.y, bgmSlider.x + bgmSlider.w, bgmSlider.y + bgmSlider.h,
        slideBarHandle, TRUE);

    // �l�̕����𖾂邭���ăR���g���X�g��t����
    int valueWidth = (int)(bgmSlider.valueDisplay * bgmSlider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(bgmSlider.x, bgmSlider.y, bgmSlider.x + valueWidth, bgmSlider.y + bgmSlider.h,
        GetColor(255, 100, 100), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �m�u�`��
    int knobSize = 32;
    float knobX = bgmSlider.x + bgmSlider.valueDisplay * bgmSlider.w - knobSize / 2;
    float knobY = bgmSlider.y + bgmSlider.h / 2 - knobSize / 2;

    float hoverScale = 1.0f + bgmSlider.hoverProgress * 0.2f;
    int scaledKnobSize = (int)(knobSize * hoverScale);
    int scaleOffset = (scaledKnobSize - knobSize) / 2;

    if (bgmSlider.glowEffect > 0.01f || bgmSlider.hoverProgress > 0.01f) {
        float totalGlow = max(bgmSlider.glowEffect, bgmSlider.hoverProgress * 0.5f);
        DrawGlowEffect((int)knobX - scaleOffset - 5, (int)knobY - scaleOffset - 5,
            scaledKnobSize + 10, scaledKnobSize + 10, totalGlow, GetColor(255, 100, 100));
    }

    DrawExtendGraph(
        (int)knobX - scaleOffset, (int)knobY - scaleOffset,
        (int)knobX - scaleOffset + scaledKnobSize, (int)knobY - scaleOffset + scaledKnobSize,
        slideHandle, TRUE
    );
}

void TitleScene::DrawSESlider()
{
    // �X���C�_�[�o�[�S�̂�`��
    DrawExtendGraph(seSlider.x, seSlider.y, seSlider.x + seSlider.w, seSlider.y + seSlider.h,
        slideBarHandle, TRUE);

    // �l�̕����𖾂邭���ăR���g���X�g��t����
    int valueWidth = (int)(seSlider.valueDisplay * seSlider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(seSlider.x, seSlider.y, seSlider.x + valueWidth, seSlider.y + seSlider.h,
        GetColor(100, 255, 100), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �m�u�`��
    int knobSize = 32;
    float knobX = seSlider.x + seSlider.valueDisplay * seSlider.w - knobSize / 2;
    float knobY = seSlider.y + seSlider.h / 2 - knobSize / 2;

    float hoverScale = 1.0f + seSlider.hoverProgress * 0.2f;
    int scaledKnobSize = (int)(knobSize * hoverScale);
    int scaleOffset = (scaledKnobSize - knobSize) / 2;

    if (seSlider.glowEffect > 0.01f || seSlider.hoverProgress > 0.01f) {
        float totalGlow = max(seSlider.glowEffect, seSlider.hoverProgress * 0.5f);
        DrawGlowEffect((int)knobX - scaleOffset - 5, (int)knobY - scaleOffset - 5,
            scaledKnobSize + 10, scaledKnobSize + 10, totalGlow, GetColor(100, 255, 100));
    }

    DrawExtendGraph(
        (int)knobX - scaleOffset, (int)knobY - scaleOffset,
        (int)knobX - scaleOffset + scaledKnobSize, (int)knobY - scaleOffset + scaledKnobSize,
        slideHandle, TRUE
    );
}

void TitleScene::EnsureTitleBGMPlaying()
{
    static int bgmCheckCounter = 0;
    bgmCheckCounter++;

    // 60�t���[���i��1�b�j���Ƃ�BGM�̏�Ԃ��`�F�b�N
    if (bgmCheckCounter % 60 == 0) {
        SoundManager& soundManager = SoundManager::GetInstance();

        // �^�C�g��BGM���Đ�����Ă��Ȃ��ꍇ�͍ĊJ
        if (!soundManager.IsBGMPlaying(SoundManager::BGM_TITLE)) {
            soundManager.PlayBGM(SoundManager::BGM_TITLE);

            // �f�o�b�O�o��
            OutputDebugStringA("TitleScene: Title BGM was restarted\n");
        }
    }
}