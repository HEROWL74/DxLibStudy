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
    , alienGlowPhase(0.0f)          // **新追加**
    , starFieldPhase(0.0f)          // **新追加**
    , titleColorPhase(0.0f)         // **新追加**
    , mousePressed(false)
    , mousePressedPrev(false)
    , particleSpawnTimer(0.0f)
    , randomEngine(std::random_device{}())
    , creditsState(CreditsHidden)
    , creditsAnimProgress(0.0f)
    , creditsFadeProgress(0.0f)
    , creditsSlideProgress(0.0f)
    , creditsScrollOffset(0.0f)
    , creditsContentHeight(1200.0f)
    , creditsAutoScroll(true)
{
    // パーティクル配列を初期化
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
    // テクスチャ読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    buttonHandle = LoadGraph("UI/PNG/Yellow/button_rectangle_depth_gradient.png");
    slideHandle = LoadGraph("UI/PNG/Yellow/slide_hangle.png");
    slideBarHandle = LoadGraph("UI/PNG/Yellow/slide_horizontal_color.png");
    homeHandle = LoadGraph("UI/PNG/Yellow/home.png");
    checkboxCheckedHandle = LoadGraph("UI/PNG/Red/check_square_color_checkmark.png");
    checkboxUncheckedHandle = LoadGraph("UI/PNG/Red/check_square_color.png");
    fontHandle = CreateFontToHandle(NULL, 28, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);

    // ボタン配置（左寄りに調整）
    int centerX = SCREEN_W / 2;
    int buttonX = centerX - BUTTON_W / 2 - 100; // 400ピクセル左に移動
    creditsState = CreditsHidden;
    creditsAnimProgress = 0.0f;
    creditsFadeProgress = 0.0f;
    creditsSlideProgress = 0.0f;
    creditsScrollOffset = 0.0f;
    creditsContentHeight = 1200.0f;
    creditsAutoScroll = true;

    // **BGM開始（タイトル画面とオプション画面で継続再生）**
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);


    buttons = {
      { buttonX, 500, BUTTON_W, BUTTON_H, "START",   NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
      { buttonX, 620, BUTTON_W, BUTTON_H, "OPTIONS", NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f },
      { buttonX, 740, BUTTON_W, BUTTON_H, "CREDITS", NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f }, // 新規追加
      { buttonX, 860, BUTTON_W, BUTTON_H, "EXIT",    NORMAL_SCALE, 0.0f, false, 0.0f, 0.0f }
    };

    // クレジット状態初期化
    creditsState = CreditsHidden;
    creditsAnimProgress = 0.0f;
    creditsFadeProgress = 0.0f;
    creditsSlideProgress = 0.0f;
    creditsScrollOffset = 0.0f;
    creditsContentHeight = 1200.0f; // クレジットコンテンツの高さ
    creditsAutoScroll = true;
    // BGMスライダー設定（既存のsliderの設定の後に追加）
    bgmSlider = {
        500, 0, 800, 24,  // x, y, w, h (yは後で動的に設定)
        0.7f,             // value (BGM初期値)
        false,            // dragging
        0.0f,             // hoverProgress
        0.7f,             // valueDisplay
        0.0f              // glowEffect
    };

    seSlider = {
        500, 0, 800, 24,  // x, y, w, h (yは後で動的に設定)
        0.8f,             // value (SE初期値)
        false,            // dragging
        0.0f,             // hoverProgress
        0.8f,             // valueDisplay
        0.0f              // glowEffect
    };

    // チェックボックス初期化
    tutorialEnabledCheckbox = { 0, 0, 32, 32, true, 0.0f, false }; // デフォルトでチュートリアル有効

    // **SoundManagerの現在の音量設定を取得してスライダーに反映**
    bgmSlider.value = SoundManager::GetInstance().GetBGMVolume();
    bgmSlider.valueDisplay = bgmSlider.value;

    seSlider.value = SoundManager::GetInstance().GetSEVolume();
    seSlider.valueDisplay = seSlider.value;
}

void TitleScene::Update()
{
    // マウス座標取得
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    // キーボード入力取得
    backspacePressedPrev = backspacePressed;
    backspacePressed = CheckHitKey(KEY_INPUT_BACK) != 0;

    // デバッグ：Creditsボタンの状態を定期的にチェック
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 300 == 0) { // 5秒に1回
        char debugMsg[128];
        sprintf_s(debugMsg, "Debug: creditsState=%d, transitionState=%d\n",
            (int)creditsState, (int)transitionState);
        OutputDebugStringA(debugMsg);
    }

    // タイトル脈動効果
    titlePulsePhase += 0.02f;

    // ボタン更新
    UpdateButtons();

    // クレジット更新
    UpdateCredits();

    // 重要：タイトルBGMの継続再生をシーン間で行う
    EnsureTitleBGMPlaying();

    // オプション開閉アニメーション
    switch (optionState) {
    case Showing:
        optionAnimProgress += ANIM_SPEED;
        optionFadeProgress += FADE_SPEED;
        optionSlideProgress += ANIM_SPEED * 0.8f;

        if (optionAnimProgress >= 1.0f) {
            optionAnimProgress = 1.0f;
            optionFadeProgress = 1.0f;
            optionSlideProgress = 1.0f;
            optionState = Visible;
            OutputDebugStringA("TitleScene: Options fully visible\n");
        }
        break;

    case Hiding:
        optionAnimProgress -= ANIM_SPEED;
        optionFadeProgress -= FADE_SPEED * 0.8f;
        optionSlideProgress -= ANIM_SPEED;

        if (optionAnimProgress <= 0.0f) {
            optionAnimProgress = 0.0f;
            optionFadeProgress = 0.0f;
            optionSlideProgress = 0.0f;
            optionState = Hidden;
            OutputDebugStringA("TitleScene: Options hidden\n");
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

    // 画面遷移フェード処理
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

    // 背景フェード効果
    float targetFade = (optionState != Hidden) ? 1.0f : 0.0f;
    backgroundFadeProgress = Lerp(backgroundFadeProgress, targetFade, FADE_SPEED * 0.7f);

    // Backspaceキーでオプション画面を閉じる
    if ((optionState == Visible || optionState == Showing) &&
        backspacePressed && !backspacePressedPrev && transitionState == None) {
        optionState = Hiding;
        OutputDebugStringA("TitleScene: Options closed by Backspace\n");
    }

    // スライダー更新（オプション画面が表示されている時のみ）
    if (optionState != Hidden && transitionState == None) {
        UpdateBGMSlider();
        UpdateSESlider();
        UpdateTutorialCheckbox();
    }

    // パーティクル更新
    UpdateParticles();

    // パーティクル生成
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
    // **1. 背景描画（最背面）**
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // **2. パーティクル描画（背景の上）**
    DrawParticles();

    // **3. タイトルロゴ描画（やるほん風スタイル）**
    // やるやかなアニメーション効果
    float titleScale = 1.0f + sinf(titlePulsePhase) * 0.03f;
    int titleAlpha = (int)(255 * (0.95f + sinf(titlePulsePhase * 0.8f) * 0.05f));

    string titleText = "Alien's Days";
    int baseWidth = GetDrawStringWidthToHandle(titleText.c_str(), (int)titleText.length(), largeFontHandle);
    int titleX = SCREEN_W / 2.23 - baseWidth / 2;
    int titleY = 200 + (int)(sinf(titlePulsePhase * 0.6f) * 3.0f);

    // やるほんな背景ボックス
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
    DrawBox(titleX - 50, titleY - 25, titleX + baseWidth + 50, titleY + 85, GetColor(255, 255, 255), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ソフトなグロー効果
    DrawGlowEffect(titleX - 20, titleY - 10, baseWidth + 40, 70, 0.2f + sinf(titlePulsePhase) * 0.1f, GetColor(255, 200, 220));

    // メインタイトル描画
    int scaledFontSize = (int)(52 * titleScale);
    int tempLargeFontHandle = CreateFontToHandle(NULL, scaledFontSize, 5);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawStringToHandle(titleX + 3, titleY + 3, titleText.c_str(), GetColor(150, 150, 150), tempLargeFontHandle);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, titleAlpha);
    DrawStringToHandle(titleX, titleY, titleText.c_str(), GetColor(80, 120, 160), tempLargeFontHandle);

    // サブタイトル
    string subTitle = "~ A Gentle Adventure ~";
    int subTitleWidth = GetDrawStringWidthToHandle(subTitle.c_str(), (int)subTitle.length(), fontHandle);
    int subTitleX = SCREEN_W / 2.23 - subTitleWidth / 2;
    int subTitleY = titleY + 70;

    int subAlpha = (int)(180 * (0.8f + sinf(titlePulsePhase * 0.4f) * 0.2f));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, subAlpha);
    DrawStringToHandle(subTitleX, subTitleY, subTitle.c_str(), GetColor(120, 150, 100), fontHandle);

    DeleteFontToHandle(tempLargeFontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **4. ボタン描画**
    for (const auto& btn : buttons) {
        DrawEnhancedButton(btn);
    }

    // **5. オプション画面描画**
    if (optionState != Hidden) {
        DrawOptionsPanel();
    }

    // **6. ★修正: クレジット画面を独立して描画**
    if (creditsState != CreditsHidden) {
        DrawCreditsPanel();
    }

    // **7. 画面遷移フェードオーバーレイ（最前面）**
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

        // スケールアニメーション
        float targetScale = over ? HOVER_SCALE : NORMAL_SCALE;
        btn.scale = Lerp(btn.scale, targetScale, SCALE_LERP);

        // フロート効果
        btn.floatPhase += FLOAT_SPEED;

        // グロー効果
        float targetGlow = over ? 1.0f : 0.0f;
        btn.glowIntensity = Lerp(btn.glowIntensity, targetGlow, 0.15f);

        // プレス効果
        if (over && mousePressed) {
            btn.pressProgress = min(1.0f, btn.pressProgress + 0.2f);
            SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);
        }
        else {
            btn.pressProgress = max(0.0f, btn.pressProgress - 0.15f);
        }
    }

    // ボタンクリック処理
    if (transitionState == None) {
        // START ボタン
        if (IsClicked(buttons[0].x, buttons[0].y, buttons[0].w, buttons[0].h)) {
            transitionState = FadingOut;
            transitionFadeProgress = 0.0f;
            startRequested = true;
            OutputDebugStringA("TitleScene: START clicked\n");
        }

        // OPTIONS ボタン
        if (optionState == Hidden &&
            IsClicked(buttons[1].x, buttons[1].y, buttons[1].w, buttons[1].h)) {
            optionState = Showing;
            optionAnimProgress = 0.0f;
            OutputDebugStringA("TitleScene: OPTIONS clicked\n");
        }

        // ★ CREDITS ボタン（トグル動作に修正）
        if (IsClicked(buttons[2].x, buttons[2].y, buttons[2].w, buttons[2].h)) {
            char debugMsg[256];
            sprintf_s(debugMsg, "CREDITS button clicked! creditsState=%d\n", (int)creditsState);
            OutputDebugStringA(debugMsg);

            // 現在の状態に応じてトグル動作
            switch (creditsState) {
            case CreditsHidden:
                // 閉じている→開く
                creditsState = CreditsShowing;
                creditsAnimProgress = 0.0f;
                OutputDebugStringA("TitleScene: CREDITS opened\n");
                break;

            case CreditsVisible:
                // 開いている→閉じる
                creditsState = CreditsHiding;
                OutputDebugStringA("TitleScene: CREDITS closed by button\n");
                break;

            case CreditsShowing:
                // 開いている最中→閉じる方向に変更
                creditsState = CreditsHiding;
                OutputDebugStringA("TitleScene: CREDITS interrupted while showing, now hiding\n");
                break;

            case CreditsHiding:
                // 閉じている最中→開く方向に変更
                creditsState = CreditsShowing;
                creditsAnimProgress = 0.0f;
                OutputDebugStringA("TitleScene: CREDITS interrupted while hiding, now showing\n");
                break;

            default:
                OutputDebugStringA("TitleScene: CREDITS unknown state, resetting to showing\n");
                creditsState = CreditsShowing;
                creditsAnimProgress = 0.0f;
                break;
            }
        }

        // EXIT ボタン
        if (IsClicked(buttons[3].x, buttons[3].y, buttons[3].w, buttons[3].h)) {
            SoundManager::GetInstance().StopBGM();
            SoundManager::GetInstance().StopAllSE();
            exitRequested = true;
            OutputDebugStringA("TitleScene: EXIT clicked\n");
        }
    }

    // オプション画面を閉じる処理
    if (optionState == Visible && transitionState == None) {
        int windowX = 400;
        int windowW = SCREEN_W - 800;
        int windowY = 150;
        int closeX = windowX + windowW - 80;
        int closeY = windowY + 20;

        if (IsClicked(closeX, closeY, 60, 60)) {
            optionState = Hiding;
            OutputDebugStringA("TitleScene: Options closed by close button\n");
        }
    }

    // クレジット画面の閉じるボタン処理
    if (creditsState == CreditsVisible && transitionState == None) {
        // クレジットパネルの右上の閉じるボタン
        int windowX = 300;
        int windowW = SCREEN_W - 600;
        int slideY = 100;
        int closeX = windowX + windowW - 80;
        int closeY = slideY + 20;

        if (IsClicked(closeX, closeY, 60, 60)) {
            creditsState = CreditsHiding;
            OutputDebugStringA("TitleScene: Credits closed by close button\n");
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

    // ドラッグ開始
    if (!slider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        slider.dragging = true;
    }

    // ドラッグ中の値更新
    if (slider.dragging) {
        float newValue = (float)(mouseX - slider.x) / slider.w;
        slider.value = clamp(newValue, 0.0f, 1.0f);
        slider.glowEffect = min(1.0f, slider.glowEffect + 0.1f);
        // 既存のマスターボリューム更新の後に
        SoundManager::GetInstance().SetMasterVolume(slider.value);
    }
    else {
        slider.glowEffect = max(0.0f, slider.glowEffect - 0.05f);
    }

    // ドラッグ終了
    if (!mousePressed) {
        slider.dragging = false;
    }

    // 表示値の滑らかな補間
    slider.valueDisplay = Lerp(slider.valueDisplay, slider.value, 0.25f);
}

void TitleScene::UpdateParticles()
{
    for (auto& p : particles) {
        if (!p.active) continue;

        p.x += p.vx;
        p.y += p.vy;
        p.life -= 0.016f; // 60FPS想定

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

        // ゴールデンカラーのバリエーション
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

    // グロー効果
    if (btn.glowIntensity > 0.01f) {
        DrawGlowEffect((int)dx - 10, (int)dy - 10, (int)sw + 20, (int)sh + 20,
            btn.glowIntensity * 0.6f, GetColor(255, 215, 0));
    }

    // ボタン本体
    DrawExtendGraph((int)dx, (int)dy, (int)(dx + sw), (int)(dy + sh), buttonHandle, TRUE);

    // テキスト描画（改良されたセンタリング）
    int textWidth = GetDrawStringWidthToHandle(btn.label.c_str(), (int)btn.label.length(), fontHandle);
    int textX = btn.x + btn.w / 2 - textWidth / 2;
    int textY = (int)(btn.y + yOffset + pressOffset + btn.h / 2 - 14);

    // テキストの影効果
    DrawStringToHandle(textX + 2, textY + 2, btn.label.c_str(), GetColor(20, 20, 20), fontHandle);
    DrawStringToHandle(textX, textY, btn.label.c_str(), GetColor(50, 50, 50), fontHandle);
}

void TitleScene::DrawEnhancedSlider()
{
    // スライダーバー画像を使用（slide_horizontal_color.png）
    int barWidth = slider.w;
    int barHeight = slider.h;

    // スライダーバー全体を描画
    DrawExtendGraph(slider.x, slider.y, slider.x + barWidth, slider.y + barHeight,
        slideBarHandle, TRUE);

    // 値の部分を暗くしてコントラストを付ける（オプション）
    int valueWidth = (int)(slider.valueDisplay * slider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(slider.x, slider.y, slider.x + valueWidth, slider.y + slider.h,
        GetColor(255, 215, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ノブ（改良版）
    int knobSize = 32;
    float knobX = slider.x + slider.valueDisplay * slider.w - knobSize / 2;
    float knobY = slider.y + slider.h / 2 - knobSize / 2;

    // ホバー時の拡大効果
    float hoverScale = 1.0f + slider.hoverProgress * 0.2f;
    int scaledKnobSize = (int)(knobSize * hoverScale);
    int scaleOffset = (scaledKnobSize - knobSize) / 2;

    // ノブのグロー効果
    if (slider.glowEffect > 0.01f || slider.hoverProgress > 0.01f) {
        float totalGlow = max(slider.glowEffect, slider.hoverProgress * 0.5f);
        DrawGlowEffect((int)knobX - scaleOffset - 5, (int)knobY - scaleOffset - 5,
            scaledKnobSize + 10, scaledKnobSize + 10, totalGlow, GetColor(255, 215, 0));
    }

    // ノブ本体
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

    // 複数のレイヤーでグローを描画
    for (int i = 0; i < 3; i++) {
        int offset = (i + 1) * 3;
        DrawBox(x - offset, y - offset, x + w + offset, y + h + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}
void TitleScene::DrawOptionsPanel()
{
    // より洗練されたイージング適用
    float easedSlide = ApplyEasing(optionSlideProgress, EASE_OUT_ELASTIC);
    float easedFade = ApplyEasing(optionFadeProgress, EASE_IN_OUT_CUBIC);

    // 改良された背景オーバーレイ（段階的フェード）
    float overlayProgress = backgroundFadeProgress;

    // 背景ブラー効果のシミュレーション（複数レイヤー）
    for (int layer = 0; layer < 3; layer++) {
        int layerAlpha = (int)(60 * overlayProgress * (1.0f - layer * 0.2f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, layerAlpha);

        int offset = layer * 2;
        DrawBox(offset, offset, SCREEN_W - offset, SCREEN_H - offset,
            GetColor(20 + layer * 10, 20 + layer * 10, 40 + layer * 15), TRUE);
    }

    // ビネット効果（画面周辺を暗く）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(80 * overlayProgress));
    for (int i = 0; i < 100; i++) {
        float ratio = (float)i / 100.0f;
        int alpha = (int)(80 * overlayProgress * ratio);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        int thickness = (int)(100 - i);
        DrawBox(0, 0, thickness, SCREEN_H, GetColor(0, 0, 0), TRUE);
        DrawBox(SCREEN_W - thickness, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        DrawBox(0, 0, SCREEN_W, thickness, GetColor(0, 0, 0), TRUE);
        DrawBox(0, SCREEN_H - thickness, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
    }

    // 改良されたウィンドウアニメーション
    int windowHeight = SCREEN_H - 300;
    int targetY = 150;

    float slideT = easedSlide;
    int slideY = SCREEN_H - (int)((SCREEN_H - targetY) * slideT);

    // 高品質な影効果（多重影）
    for (int shadowLayer = 0; shadowLayer < 5; shadowLayer++) {
        int shadowOffset = (shadowLayer + 1) * 3;
        int shadowAlpha = (int)((80 - shadowLayer * 15) * easedFade);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, shadowAlpha);
        DrawBox(400 + shadowOffset, slideY + shadowOffset,
            SCREEN_W - 400 + shadowOffset, slideY + windowHeight + shadowOffset,
            GetColor(0, 0, 0), TRUE);
    }

    // ウィンドウの段階的表示（スケール効果付き）
    float scaleEffect = 0.9f + easedFade * 0.1f;
    int windowW = (int)((SCREEN_W - 800) * scaleEffect);
    int windowH = (int)(windowHeight * scaleEffect);
    int windowX = 400 + (SCREEN_W - 800 - windowW) / 2;
    int windowYScaled = slideY + (windowHeight - windowH) / 2;

    // グラデーション背景（高品質）
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

    // 光沢効果（上部ハイライト）
    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 * easedFade));
    for (int i = 0; i < 30; i++) {
        float highlightRatio = (float)i / 30.0f;
        int alpha = (int)(40 * easedFade * (1.0f - highlightRatio));
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawLine(windowX, windowYScaled + i, windowX + windowW, windowYScaled + i,
            GetColor(120, 160, 220), TRUE);
    }

    // 洗練されたウィンドウ枠（多重枠）
    int windowAlpha = (int)(255 * easedFade);

    // 外枠（暗い）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
    DrawBox(windowX - 2, windowYScaled - 2, windowX + windowW + 2, windowYScaled + windowH + 2,
        GetColor(60, 80, 120), FALSE);

    // 内枠（明るい）
    DrawBox(windowX, windowYScaled, windowX + windowW, windowYScaled + windowH,
        GetColor(120, 160, 220), FALSE);

    // タイトルとコンテンツの段階的フェードイン
    float contentFade = max(0.0f, (easedFade - 0.3f) / 0.7f);

    // オプションタイトル（フェードイン + 軽いスケール効果）
    float titleScale = 0.95f + contentFade * 0.05f;
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(windowAlpha * contentFade));

    string optionsTitle = "OPTIONS";
    int titleWidth = GetDrawStringWidthToHandle(optionsTitle.c_str(), (int)optionsTitle.length(), largeFontHandle);
    int titleX = windowX + windowW / 2 - (int)(titleWidth * titleScale) / 2;
    int titleY = windowYScaled + 30;

    // タイトルの影
    DrawStringToHandle(titleX + 2, titleY + 2, optionsTitle.c_str(), GetColor(0, 0, 0), largeFontHandle);
    DrawStringToHandle(titleX, titleY, optionsTitle.c_str(), GetColor(255, 255, 255), largeFontHandle);

    // コンテンツエリアの計算（スケール後の座標）
    int contentAlpha = (int)(windowAlpha * contentFade);

    // BGM/SEラベルとスライダー（段階的表示）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, contentAlpha);

    int labelStartY = windowYScaled + 100;
    int sliderStartY = windowYScaled + 140;

    // BGMセクション
    DrawStringToHandle(windowX + 50, labelStartY, "BGM Volume", GetColor(200, 200, 200), fontHandle);

    // BGMスライダー位置調整
    bgmSlider.x = windowX + 50;
    bgmSlider.y = sliderStartY;
    bgmSlider.w = windowW - 150;
    DrawBGMSlider();

    // BGM数値表示
    int bgmPercent = (int)(bgmSlider.valueDisplay * 100);
    string bgmText = to_string(bgmPercent) + "%";
    DrawStringToHandle(bgmSlider.x + bgmSlider.w + 20, sliderStartY - 5, bgmText.c_str(), GetColor(255, 215, 0), fontHandle);

    // SEセクション
    int seLabelsY = labelStartY + 80;
    int seSliderY = sliderStartY + 80;

    DrawStringToHandle(windowX + 50, seLabelsY, "SE Volume", GetColor(200, 200, 200), fontHandle);

    // SEスライダー位置調整
    seSlider.x = windowX + 50;
    seSlider.y = seSliderY;
    seSlider.w = windowW - 150;
    DrawSESlider();

    // SE数値表示
    int sePercent = (int)(seSlider.valueDisplay * 100);
    string seText = to_string(sePercent) + "%";
    DrawStringToHandle(seSlider.x + seSlider.w + 20, seSliderY - 5, seText.c_str(), GetColor(255, 215, 0), fontHandle);

    // チュートリアル設定チェックボックス描画
    float instructionFade = max(0.0f, (contentFade - 0.5f) / 0.5f);
    if (instructionFade > 0.1f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * instructionFade));
        DrawTutorialCheckbox();
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 戻るための説明文（フェードイン）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * instructionFade));
    DrawStringToHandle(windowX + 50, windowYScaled + windowH - 60,
        "Press [Backspace] to return", GetColor(150, 150, 150), fontHandle);

    // 閉じるボタン（洗練されたアニメーション）
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

void TitleScene::DrawCreditsPanel()
{
    // イージング適用
    float easedSlide = ApplyEasing(creditsSlideProgress, EASE_OUT_ELASTIC);
    float easedFade = ApplyEasing(creditsFadeProgress, EASE_IN_OUT_CUBIC);

    // 背景オーバーレイ
    float overlayProgress = creditsFadeProgress;
    for (int layer = 0; layer < 3; layer++) {
        int layerAlpha = (int)(60 * overlayProgress * (1.0f - layer * 0.2f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, layerAlpha);
        int offset = layer * 2;
        DrawBox(offset, offset, SCREEN_W - offset, SCREEN_H - offset,
            GetColor(10 + layer * 5, 10 + layer * 5, 20 + layer * 10), TRUE);
    }

    // ビネット効果
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(80 * overlayProgress));
    for (int i = 0; i < 100; i++) {
        float ratio = (float)i / 100.0f;
        int alpha = (int)(80 * overlayProgress * ratio);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        int thickness = (int)(100 - i);
        DrawBox(0, 0, thickness, SCREEN_H, GetColor(0, 0, 0), TRUE);
        DrawBox(SCREEN_W - thickness, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        DrawBox(0, 0, SCREEN_W, thickness, GetColor(0, 0, 0), TRUE);
        DrawBox(0, SCREEN_H - thickness, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
    }

    // ウィンドウアニメーション
    int windowHeight = SCREEN_H - 200;
    int targetY = 100;

    float slideT = easedSlide;
    int slideY = SCREEN_H - (int)((SCREEN_H - targetY) * slideT);

    // 高品質な影効果
    for (int shadowLayer = 0; shadowLayer < 5; shadowLayer++) {
        int shadowOffset = (shadowLayer + 1) * 3;
        int shadowAlpha = (int)((80 - shadowLayer * 15) * easedFade);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, shadowAlpha);
        DrawBox(300 + shadowOffset, slideY + shadowOffset,
            SCREEN_W - 300 + shadowOffset, slideY + windowHeight + shadowOffset,
            GetColor(0, 0, 0), TRUE);
    }

    // ウィンドウ本体
    float scaleEffect = 0.9f + easedFade * 0.1f;
    int windowW = (int)((SCREEN_W - 600) * scaleEffect);
    int windowH = (int)(windowHeight * scaleEffect);
    int windowX = 300 + (SCREEN_W - 600 - windowW) / 2;
    int windowYScaled = slideY + (windowHeight - windowH) / 2;

    // グラデーション背景
    for (int i = 0; i < windowH; i++) {
        float gradientRatio = (float)i / windowH;
        int r = (int)(25 + gradientRatio * 10);
        int g = (int)(25 + gradientRatio * 15);
        int b = (int)(45 + gradientRatio * 20);
        int alpha = (int)(240 * easedFade * (0.8f + gradientRatio * 0.2f));

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawLine(windowX, windowYScaled + i, windowX + windowW, windowYScaled + i,
            GetColor(r, g, b), TRUE);
    }

    // 光沢効果
    SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(30 * easedFade));
    for (int i = 0; i < 40; i++) {
        float highlightRatio = (float)i / 40.0f;
        int alpha = (int)(30 * easedFade * (1.0f - highlightRatio));
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawLine(windowX, windowYScaled + i, windowX + windowW, windowYScaled + i,
            GetColor(100, 120, 180), TRUE);
    }

    // ウィンドウ枠
    int windowAlpha = (int)(255 * easedFade);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, windowAlpha);
    DrawBox(windowX - 2, windowYScaled - 2, windowX + windowW + 2, windowYScaled + windowH + 2,
        GetColor(80, 100, 140), FALSE);
    DrawBox(windowX, windowYScaled, windowX + windowW, windowYScaled + windowH,
        GetColor(120, 140, 200), FALSE);

    // コンテンツフェードイン
    float contentFade = max(0.0f, (easedFade - 0.3f) / 0.7f);
    int contentAlpha = (int)(windowAlpha * contentFade);

    // タイトル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, contentAlpha);
    string creditsTitle = "CREDITS";
    int titleWidth = GetDrawStringWidthToHandle(creditsTitle.c_str(), (int)creditsTitle.length(), largeFontHandle);
    int titleX = windowX + windowW / 2 - titleWidth / 2;
    int titleY = windowYScaled + 30;

    DrawStringToHandle(titleX + 2, titleY + 2, creditsTitle.c_str(), GetColor(0, 0, 0), largeFontHandle);
    DrawStringToHandle(titleX, titleY, creditsTitle.c_str(), GetColor(255, 255, 255), largeFontHandle);

    // スクロール可能なコンテンツエリア
    int contentStartY = windowYScaled + 80;
    int contentAreaH = windowH - 160;

    // クリッピング領域設定
    SetDrawArea(windowX + 20, contentStartY, windowX + windowW - 20, contentStartY + contentAreaH);

    // **クレジットコンテンツ描画**
    int lineHeight = 40;
    int currentY = contentStartY - (int)creditsScrollOffset;

    // ゲームタイトル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, contentAlpha);
    string gameTitle = "Alien's Days - A Gentle Adventure";
    int gameTitleWidth = GetDrawStringWidthToHandle(gameTitle.c_str(), (int)gameTitle.length(), largeFontHandle);
    DrawStringToHandle(windowX + windowW / 2 - gameTitleWidth / 2, currentY,
        gameTitle.c_str(), GetColor(255, 215, 0), largeFontHandle);
    currentY += lineHeight * 2;

    // バージョン情報
    string version = "Version 1.0.0";
    int versionWidth = GetDrawStringWidthToHandle(version.c_str(), (int)version.length(), fontHandle);
    DrawStringToHandle(windowX + windowW / 2 - versionWidth / 2, currentY,
        version.c_str(), GetColor(200, 200, 200), fontHandle);
    currentY += lineHeight * 2;

    // 開発チーム
    DrawStringToHandle(windowX + 50, currentY, "DEVELOPMENT TEAM", GetColor(100, 200, 255), largeFontHandle);
    currentY += lineHeight * 1.5f;

    // 開発者情報
    string developers[] = {
     "Game Design & Programming:",
     "Hiroaki Togawa : 砥川展明",  // ← あなたの名前に変更
     "",
     "Character Art & Animation:",
     "  Kenney Assets Pack",
     "",
     "Background Art:",
     " Kenney",
     "",
     "UI Design:",
     "  Kenney",
     "",
     "Sound Effects:",
     " Springin,Kenney",
     "",
     "Music Composition:",
     "Kenney"
    };

    for (const string& line : developers) {
        int color = line.empty() ? GetColor(0, 0, 0) :
            (line.find(":") != string::npos) ? GetColor(150, 255, 150) :
            (line.find("  ") == 0) ? GetColor(200, 200, 200) :
            GetColor(255, 255, 255);

        DrawStringToHandle(windowX + 60, currentY, line.c_str(), color, fontHandle);
        currentY += lineHeight;
    }

    currentY += lineHeight;

    // 特別な感謝
    DrawStringToHandle(windowX + 50, currentY, "SPECIAL THANKS", GetColor(255, 150, 150), largeFontHandle);
    currentY += lineHeight * 1.5f;

    string thanks[] = {
        "DxLib Development Team",
        "  - For providing excellent 2D game library",
        "",
        "Pixel Art Community",
        "  - For inspiring beautiful sprite designs",
        "",
        "Game Development Community",
        "  - For sharing knowledge and techniques",
        "",
        "All Beta Testers",
        "  - For valuable feedback and bug reports"
    };

    for (const string& line : thanks) {
        int color = line.empty() ? GetColor(0, 0, 0) :
            (line.find("  -") == 0) ? GetColor(180, 180, 180) :
            GetColor(255, 200, 200);

        DrawStringToHandle(windowX + 60, currentY, line.c_str(), color, fontHandle);
        currentY += lineHeight;
    }

    currentY += lineHeight * 2;

    // 著作権情報
    DrawStringToHandle(windowX + 50, currentY, "COPYRIGHT & LICENSING", GetColor(255, 215, 0), largeFontHandle);
    currentY += lineHeight * 1.5f;
    string copyright[] = {
        "Programming: C++ with DxLib",
        "Graphics: Kenney Asset Packs",
        "Music: Kenney Asset Packs",
        "",
        "Game Engine: DxLib",
        "Platform: Windows",
        "Language: C++"
    };

    for (const string& line : copyright) {
        int color = line.empty() ? GetColor(0, 0, 0) : GetColor(200, 200, 200);
        DrawStringToHandle(windowX + 60, currentY, line.c_str(), color, fontHandle);
        currentY += lineHeight;
    }

    currentY += lineHeight * 3;

    // 最終メッセージ
    string finalMessage = "Thank you for playing!";
    int finalWidth = GetDrawStringWidthToHandle(finalMessage.c_str(), (int)finalMessage.length(), largeFontHandle);

    // 虹色効果
    for (int i = 0; i < (int)finalMessage.length(); i++) {
        float colorPhase = (GetNowCount() * 0.005f) + i * 0.3f;
        int r = (int)(128 + 127 * sin(colorPhase));
        int g = (int)(128 + 127 * sin(colorPhase + 2.1f));
        int b = (int)(128 + 127 * sin(colorPhase + 4.2f));

        string charStr = finalMessage.substr(i, 1);
        int charWidth = GetDrawStringWidthToHandle(charStr.c_str(), 1, largeFontHandle);
        int charX = windowX + windowW / 2 - finalWidth / 2 + i * (charWidth - 5);

        DrawStringToHandle(charX, currentY, charStr.c_str(), GetColor(r, g, b), largeFontHandle);
    }

    // クリッピング領域をリセット
    SetDrawArea(0, 0, SCREEN_W, SCREEN_H);

    // スクロールバー
    if (creditsContentHeight > contentAreaH) {
        float scrollBarH = (float)contentAreaH * contentAreaH / creditsContentHeight;
        float scrollBarY = contentStartY + (creditsScrollOffset / creditsContentHeight) * (contentAreaH - scrollBarH);

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(150 * contentFade));
        DrawBox(windowX + windowW - 15, (int)scrollBarY, windowX + windowW - 5, (int)(scrollBarY + scrollBarH),
            GetColor(200, 200, 200), TRUE);
    }

    // 操作説明
    float instructionFade = max(0.0f, (contentFade - 0.5f) / 0.5f);
    if (instructionFade > 0.1f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * instructionFade));
        DrawStringToHandle(windowX + 50, windowYScaled + windowH - 80,
            "Mouse Wheel: Scroll | [Backspace]: Return", GetColor(150, 150, 150), fontHandle);

        if (creditsAutoScroll) {
            DrawStringToHandle(windowX + 50, windowYScaled + windowH - 55,
                "Auto-scroll: ON (Touch wheel to disable)", GetColor(100, 200, 100), fontHandle);
        }
        else {
            DrawStringToHandle(windowX + 50, windowYScaled + windowH - 55,
                "Auto-scroll: OFF", GetColor(200, 100, 100), fontHandle);
        }
    }

    // 閉じるボタン
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

// ユーティリティ関数の実装
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

// BGMスライダー更新関数の改良版
void TitleScene::UpdateBGMSlider()
{
    int knobSize = 32;
    float knobX = bgmSlider.x + bgmSlider.value * bgmSlider.w - knobSize / 2;
    float knobY = bgmSlider.y + bgmSlider.h / 2 - knobSize / 2;

    bool knobHovered = IsMouseOver((int)knobX, (int)knobY, knobSize, knobSize);
    bgmSlider.hoverProgress = Lerp(bgmSlider.hoverProgress, knobHovered ? 1.0f : 0.0f, 0.2f);

    // ドラッグ開始
    if (!bgmSlider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        bgmSlider.dragging = true;
    }

    // ドラッグ中の値更新
    if (bgmSlider.dragging) {
        float newValue = (float)(mouseX - bgmSlider.x) / bgmSlider.w;
        bgmSlider.value = clamp(newValue, 0.0f, 1.0f);
        bgmSlider.glowEffect = min(1.0f, bgmSlider.glowEffect + 0.1f);

        // **BGMボリュームをリアルタイム更新**
        SoundManager::GetInstance().SetBGMVolume(bgmSlider.value);

        // **タイトルBGMが再生中の場合、即座にボリューム変更を適用**
        // SetBGMVolume内で既に処理されているが、確実にするため
    }
    else {
        bgmSlider.glowEffect = max(0.0f, bgmSlider.glowEffect - 0.05f);
    }

    // ドラッグ終了
    if (!mousePressed) {
        bgmSlider.dragging = false;
    }

    // 表示値の滑らかな補間
    bgmSlider.valueDisplay = Lerp(bgmSlider.valueDisplay, bgmSlider.value, 0.25f);
}

void TitleScene::UpdateSESlider()
{
    int knobSize = 32;
    float knobX = seSlider.x + seSlider.value * seSlider.w - knobSize / 2;
    float knobY = seSlider.y + seSlider.h / 2 - knobSize / 2;

    bool knobHovered = IsMouseOver((int)knobX, (int)knobY, knobSize, knobSize);
    seSlider.hoverProgress = Lerp(seSlider.hoverProgress, knobHovered ? 1.0f : 0.0f, 0.2f);

    // ドラッグ開始
    if (!seSlider.dragging && mousePressed && !mousePressedPrev && knobHovered) {
        seSlider.dragging = true;
    }

    // ドラッグ中の値更新
    if (seSlider.dragging) {
        float newValue = (float)(mouseX - seSlider.x) / seSlider.w;
        seSlider.value = clamp(newValue, 0.0f, 1.0f);
        seSlider.glowEffect = min(1.0f, seSlider.glowEffect + 0.1f);

        // SEボリュームをリアルタイム更新
        SoundManager::GetInstance().SetSEVolume(seSlider.value);
    }
    else {
        seSlider.glowEffect = max(0.0f, seSlider.glowEffect - 0.05f);
    }

    // ドラッグ終了
    if (!mousePressed) {
        seSlider.dragging = false;
    }

    // 表示値の滑らかな補間
    seSlider.valueDisplay = Lerp(seSlider.valueDisplay, seSlider.value, 0.25f);
}

void TitleScene::DrawBGMSlider()
{
    // スライダーバー全体を描画
    DrawExtendGraph(bgmSlider.x, bgmSlider.y, bgmSlider.x + bgmSlider.w, bgmSlider.y + bgmSlider.h,
        slideBarHandle, TRUE);

    // 値の部分を明るくしてコントラストを付ける
    int valueWidth = (int)(bgmSlider.valueDisplay * bgmSlider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(bgmSlider.x, bgmSlider.y, bgmSlider.x + valueWidth, bgmSlider.y + bgmSlider.h,
        GetColor(255, 100, 100), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ノブ描画
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
    // スライダーバー全体を描画
    DrawExtendGraph(seSlider.x, seSlider.y, seSlider.x + seSlider.w, seSlider.y + seSlider.h,
        slideBarHandle, TRUE);

    // 値の部分を明るくしてコントラストを付ける
    int valueWidth = (int)(seSlider.valueDisplay * seSlider.w);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(seSlider.x, seSlider.y, seSlider.x + valueWidth, seSlider.y + seSlider.h,
        GetColor(100, 255, 100), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ノブ描画
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

    // 60フレーム（約1秒）ごとにBGMの状態をチェック
    if (bgmCheckCounter % 60 == 0) {
        SoundManager& soundManager = SoundManager::GetInstance();

        // タイトルBGMが再生されていない場合は再開
        if (!soundManager.IsBGMPlaying(SoundManager::BGM_TITLE)) {
            soundManager.PlayBGM(SoundManager::BGM_TITLE);

            // デバッグ出力
            OutputDebugStringA("TitleScene: Title BGM was restarted\n");
        }
    }
}

void TitleScene::UpdateTutorialCheckbox()
{
    // チェックボックスの位置はDrawメソッドで動的に設定されるため、ここでは hover 判定のみ
    bool over = IsMouseOver(tutorialEnabledCheckbox.x, tutorialEnabledCheckbox.y,
        tutorialEnabledCheckbox.w, tutorialEnabledCheckbox.h);
    tutorialEnabledCheckbox.hovered = over;

    // ホバー効果
    float targetHover = over ? 1.0f : 0.0f;
    tutorialEnabledCheckbox.hoverProgress = Lerp(tutorialEnabledCheckbox.hoverProgress, targetHover, 0.2f);

    // クリック処理
    if (over && mousePressed && !mousePressedPrev) {
        tutorialEnabledCheckbox.checked = !tutorialEnabledCheckbox.checked;
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);

        char debugMsg[128];
        sprintf_s(debugMsg, "TitleScene: Tutorial checkbox toggled to %s\n",
            tutorialEnabledCheckbox.checked ? "enabled" : "disabled");
        OutputDebugStringA(debugMsg);
    }
}

void TitleScene::DrawTutorialCheckbox()
{
    // チェックボックスの位置を計算（SE slider の下）
    int windowX = 400;
    int windowW = SCREEN_W - 800;
    int checkboxY = seSlider.y + 80;

    tutorialEnabledCheckbox.x = windowX + 50;
    tutorialEnabledCheckbox.y = checkboxY;

    // ラベル表示
    DrawStringToHandle(tutorialEnabledCheckbox.x + 50, checkboxY + 4, "Show Tutorial Button",
        GetColor(200, 200, 200), fontHandle);

    // チェックボックス本体
    int checkboxHandle = tutorialEnabledCheckbox.checked ? checkboxCheckedHandle : checkboxUncheckedHandle;

    // ホバー効果
    if (tutorialEnabledCheckbox.hoverProgress > 0.01f) {
        float glowIntensity = tutorialEnabledCheckbox.hoverProgress * 0.5f;
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * glowIntensity));
        DrawExtendGraph(tutorialEnabledCheckbox.x - 3, tutorialEnabledCheckbox.y - 3,
            tutorialEnabledCheckbox.x + tutorialEnabledCheckbox.w + 3,
            tutorialEnabledCheckbox.y + tutorialEnabledCheckbox.h + 3,
            checkboxHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 通常表示
    DrawExtendGraph(tutorialEnabledCheckbox.x, tutorialEnabledCheckbox.y,
        tutorialEnabledCheckbox.x + tutorialEnabledCheckbox.w,
        tutorialEnabledCheckbox.y + tutorialEnabledCheckbox.h,
        checkboxHandle, TRUE);
}

// 新規追加: チュートリアル設定取得メソッド
bool TitleScene::IsTutorialEnabled() const
{
    return tutorialEnabledCheckbox.checked;
}

void TitleScene::UpdateCredits()
{
    // クレジットアニメーション更新
    switch (creditsState) {
    case CreditsShowing:
        creditsAnimProgress += ANIM_SPEED;
        creditsFadeProgress += FADE_SPEED;
        creditsSlideProgress += ANIM_SPEED * 0.8f;

        if (creditsAnimProgress >= 1.0f) {
            creditsAnimProgress = 1.0f;
            creditsFadeProgress = 1.0f;
            creditsSlideProgress = 1.0f;
            creditsState = CreditsVisible;
            creditsScrollOffset = 0.0f; // スクロール位置リセット
            OutputDebugStringA("TitleScene: Credits panel fully visible\n");
        }
        break;

    case CreditsHiding:
        creditsAnimProgress -= ANIM_SPEED;
        creditsFadeProgress -= FADE_SPEED * 0.8f;
        creditsSlideProgress -= ANIM_SPEED;

        if (creditsAnimProgress <= 0.0f) {
            ResetCreditsState();
            OutputDebugStringA("TitleScene: Credits panel hidden\n");
        }
        break;

    case CreditsVisible: {
        creditsAnimProgress = 1.0f;
        creditsFadeProgress = 1.0f;
        creditsSlideProgress = 1.0f;

        // 自動スクロール処理
        if (creditsAutoScroll) {
            creditsScrollOffset += 0.5f; // スクロール速度
            if (creditsScrollOffset > creditsContentHeight + 200) {
                creditsScrollOffset = -400.0f; // 上に戻る
            }
        }

        // 手動スクロール（マウスホイール）
        int wheelRotation = GetMouseWheelRotVol();
        if (wheelRotation != 0) {
            creditsScrollOffset -= wheelRotation * 20.0f;
            creditsScrollOffset = max(-400.0f, min(creditsScrollOffset, creditsContentHeight + 200));
            creditsAutoScroll = false; // 手動操作時は自動スクロール停止
        }
        break;
    }
    default:
        creditsAnimProgress = 0.0f;
        creditsFadeProgress = 0.0f;
        creditsSlideProgress = 0.0f;
        break;
    }

    // Backspaceキーでクレジット画面を閉じる
    if ((creditsState == CreditsVisible || creditsState == CreditsShowing) &&
        backspacePressed && !backspacePressedPrev && transitionState == None) {
        creditsState = CreditsHiding;
        OutputDebugStringA("TitleScene: Credits closed by Backspace\n");
    }
}

void TitleScene::ResetCreditsState()
{
    creditsState = CreditsHidden;
    creditsAnimProgress = 0.0f;
    creditsFadeProgress = 0.0f;
    creditsSlideProgress = 0.0f;
    creditsScrollOffset = 0.0f;
    creditsAutoScroll = true;
    OutputDebugStringA("TitleScene: Credits state reset to Hidden\n");
}
void TitleScene::ForceResetCredits()
{
    OutputDebugStringA("TitleScene: Force resetting credits state\n");
    ResetCreditsState();
}