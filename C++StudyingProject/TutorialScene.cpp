#include "TutorialScene.h"
#include <math.h>
using namespace std;

TutorialScene::TutorialScene()
    : selectedCharacterIndex(-1)
    , cameraX(0.0f)
    , previousPlayerX(0.0f)
    , exitRequested(false)
    , tutorialCompleted(false)
    , currentStep(STEP_WELCOME)
    , stepTimer(0.0f)
    , messageTimer(0.0f)
    , messageVisible(true)
    , escPressed(false), escPressedPrev(false)
    , rPressed(false), rPressedPrev(false)
    , leftPressed(false), leftPressedPrev(false)
    , rightPressed(false), rightPressedPrev(false)
    , spacePressed(false), spacePressedPrev(false)
    , downPressed(false), downPressedPrev(false)
    , startX(300.0f)
    , goalX(2500.0f)
    , hasMovedLeft(false)
    , hasMovedRight(false)
    , hasJumped(false)
    , hasDucked(false)
    , hasSlid(false)
    , fadeAlpha(0.0f)
    , fadingIn(true)
    , fadingOut(false)
    , successEffectTimer(0.0f)
    , showingSuccessEffect(false)
    , uiPulseTimer(0.0f)
    , iconRotation(0.0f)
    , progressBarGlow(0.0f)
    , stepJustChanged(false)
    , stepChangeEffectTimer(0.0f)
{
    backgroundHandle = -1;
    fontHandle = -1;
    largeFontHandle = -1;
    smallFontHandle = -1;

    // 入力インジケーター初期化
    leftIndicator = { false, 0.0f, 0.0f };
    rightIndicator = { false, 0.0f, 0.0f };
    spaceIndicator = { false, 0.0f, 0.0f };
    downIndicator = { false, 0.0f, 0.0f };
}

TutorialScene::~TutorialScene()
{
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);
    DeleteFontToHandle(smallFontHandle);
}

void TutorialScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    characterName = GetCharacterDisplayName(selectedCharacter);

    // **状態を完全にリセット**
    exitRequested = false;
    tutorialCompleted = false;
    currentStep = STEP_WELCOME;
    stepTimer = 0.0f;
    messageTimer = 0.0f;
    messageVisible = true;

    // **チュートリアル進行状況をリセット**
    hasMovedLeft = false;
    hasMovedRight = false;
    hasJumped = false;
    hasDucked = false;
    hasSlid = false;

    // **フェード状態をリセット**
    fadeAlpha = 1.0f;  // 黒画面から開始
    fadingIn = true;
    fadingOut = false;

    // 既存の初期化処理...

    // テクスチャとフォント読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);
    smallFontHandle = CreateFontToHandle(NULL, 24, 2);

    // プレイヤー初期化
    tutorialPlayer.Initialize(selectedCharacter);
    tutorialPlayer.SetPosition(startX, 200.0f);

    // チュートリアル専用ステージ初期化
    InitializeTutorialStage();

    // HUD初期化
    hudSystem.Initialize();
    hudSystem.SetPlayerCharacter(selectedCharacterIndex);
    hudSystem.SetMaxLife(6);
    hudSystem.SetCurrentLife(6);
    hudSystem.SetPosition(30, 30);
    hudSystem.SetVisible(true);

    // カメラ初期位置
    cameraX = 0.0f;
    previousPlayerX = tutorialPlayer.GetX();

    // チュートリアルテキスト初期化
    InitializeTutorialTexts();

    // エフェクトシステム初期化
    particles.clear();
    floatingTexts.clear();

    // **UI効果をリセット**
    uiPulseTimer = 0.0f;
    iconRotation = 0.0f;
    progressBarGlow = 0.0f;
    stepJustChanged = false;
    stepChangeEffectTimer = 0.0f;

    // BGM開始
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_GAME);

    OutputDebugStringA("TutorialScene: Fully initialized and ready for new tutorial session\n");
}

void TutorialScene::InitializeTutorialStage()
{
    stageManager.LoadStage(StageManager::GRASS);
}

void TutorialScene::InitializeTutorialTexts()
{
    tutorialTexts.clear();

    // ステップ0: ようこそ
    tutorialTexts.push_back({
        "Welcome to the Adventure!",
        "Master the basic controls to become a true hero",
        "Press [Left/Right Arrow] to begin your journey",
        5.0f, false
        });

    // ステップ1: 移動
    tutorialTexts.push_back({
        "Perfect! Feel the movement",
        "Use Left and Right arrow keys to explore the world",
        "Move in both directions to continue",
        0.0f, false
        });

    // ステップ2: ジャンプ
    tutorialTexts.push_back({
        "Time to soar through the sky!",
        "Use the Space key to leap over obstacles with grace",
        "Press [Space] to jump high",
        0.0f, false
        });

    // ステップ3: しゃがみ
    tutorialTexts.push_back({
        "Master the art of stealth",
        "Use the Down arrow key to duck under low passages",
        "Press [Down Arrow] to crouch low",
        0.0f, false
        });

    // ステップ4: スライディング
    tutorialTexts.push_back({
        "The ultimate technique awaits!",
        "While moving, press Down to slide with incredible speed",
        "Run fast and press [Down] to slide",
        0.0f, false
        });

    // ステップ5: ゴール
    tutorialTexts.push_back({
        "Magnificent! Now reach your destiny",
        "Use all your newfound skills to reach the golden goal",
        "Touch the shining beacon to complete your training",
        0.0f, false
        });

    // ステップ6: 完了
    tutorialTexts.push_back({
        "Congratulations, Hero!",
        "You have mastered the fundamental arts of adventure",
        "Press [R] to return or [ESC] to begin your true quest",
        0.0f, false
        });
}

void TutorialScene::Update()
{
    // 入力更新
    UpdateInput();

    // フェード効果更新
    UpdateFade();

    // キー入力処理
    if (currentStep == STEP_COMPLETED) {
        if (rPressed && !rPressedPrev) {
            exitRequested = true;
            OutputDebugStringA("TutorialScene: R key pressed - returning to character selection\n");
        }
        else if (escPressed && !escPressedPrev) {
            exitRequested = true;
            OutputDebugStringA("TutorialScene: ESC key pressed - proceeding to game\n");
        }
    }
    else {
        if (escPressed && !escPressedPrev) {
            exitRequested = true;
            OutputDebugStringA("TutorialScene: ESC pressed during tutorial\n");
        }
    }

    // プレイヤー更新
    tutorialPlayer.Update(&stageManager);

    // カメラ更新
    UpdateCamera();

    // ステージ更新
    stageManager.Update(cameraX);

    // チュートリアル進行ロジック更新
    UpdateTutorialLogic();

    // エフェクト更新
    UpdateEffects();

    // HUD更新
    hudSystem.Update();
}

void TutorialScene::UpdateInput()
{
    // 前フレームの入力を保存
    escPressedPrev = escPressed;
    rPressedPrev = rPressed;
    leftPressedPrev = leftPressed;
    rightPressedPrev = rightPressed;
    spacePressedPrev = spacePressed;
    downPressedPrev = downPressed;

    // 現在の入力を取得
    escPressed = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    rPressed = CheckHitKey(KEY_INPUT_R) != 0;
    leftPressed = CheckHitKey(KEY_INPUT_LEFT) != 0;
    rightPressed = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;
    downPressed = CheckHitKey(KEY_INPUT_DOWN) != 0;
}

void TutorialScene::UpdateCamera()
{
    float currentPlayerX = tutorialPlayer.GetX();
    float targetCameraX = currentPlayerX - SCREEN_W * 0.35f;

    // 範囲制限
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) {
        targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;
    }

    // 滑らかな追従
    cameraX = Lerp(cameraX, targetCameraX, CAMERA_FOLLOW_SPEED);
    previousPlayerX = currentPlayerX;
}

void TutorialScene::UpdateTutorialLogic()
{
    stepTimer += 0.016f;
    messageTimer += 0.016f;

    // メッセージの点滅効果
    if (messageTimer >= 1.5f) {
        messageVisible = !messageVisible;
        messageTimer = 0.0f;
    }

    // 入力の記録
    if (leftPressed && !leftPressedPrev) {
        hasMovedLeft = true;
        CreateSuccessParticles(tutorialPlayer.GetX() - 50, tutorialPlayer.GetY());
        AddFloatingText("Left!", tutorialPlayer.GetX() - 80, tutorialPlayer.GetY() - 50, GetColor(100, 255, 100));
    }
    if (rightPressed && !rightPressedPrev) {
        hasMovedRight = true;
        CreateSuccessParticles(tutorialPlayer.GetX() + 50, tutorialPlayer.GetY());
        AddFloatingText("Right!", tutorialPlayer.GetX() + 80, tutorialPlayer.GetY() - 50, GetColor(100, 255, 100));
    }
    if (spacePressed && !spacePressedPrev) {
        hasJumped = true;
        CreateSuccessParticles(tutorialPlayer.GetX(), tutorialPlayer.GetY() - 80);
        AddFloatingText("Jump!", tutorialPlayer.GetX(), tutorialPlayer.GetY() - 100, GetColor(255, 255, 100));
    }
    if (downPressed && !downPressedPrev) {
        hasDucked = true;
        CreateSuccessParticles(tutorialPlayer.GetX(), tutorialPlayer.GetY() + 30);
        AddFloatingText("Duck!", tutorialPlayer.GetX(), tutorialPlayer.GetY() + 50, GetColor(100, 200, 255));
    }
    if (tutorialPlayer.IsSliding()) {
        if (!hasSlid) {
            hasSlid = true;
            CreateSuccessParticles(tutorialPlayer.GetX(), tutorialPlayer.GetY());
            AddFloatingText("Slide!", tutorialPlayer.GetX(), tutorialPlayer.GetY() - 30, GetColor(255, 200, 100), true);
        }
    }

    // ステップ完了チェック
    CheckStepCompletion();
}

void TutorialScene::CheckStepCompletion()
{
    bool stepCompleted = false;

    switch (currentStep) {
    case STEP_WELCOME:
        if (stepTimer >= 3.0f || hasMovedLeft || hasMovedRight) {
            stepCompleted = true;
        }
        break;

    case STEP_MOVEMENT:
        if (hasMovedLeft && hasMovedRight) {
            stepCompleted = true;
        }
        break;

    case STEP_JUMPING:
        if (hasJumped) {
            stepCompleted = true;
        }
        break;

    case STEP_DUCKING:
        if (hasDucked) {
            stepCompleted = true;
        }
        break;

    case STEP_SLIDING:
        if (hasSlid) {
            stepCompleted = true;
        }
        break;

    case STEP_GOAL:
        if (IsPlayerInRange(goalX - 100, goalX + 100)) {
            stepCompleted = true;
        }
        break;

    case STEP_COMPLETED:
        break;
    }

    if (stepCompleted) {
        NextStep();
    }
}

void TutorialScene::NextStep()
{
    if (currentStep < STEP_COMPLETED) {
        currentStep = static_cast<TutorialStep>(currentStep + 1);
        stepTimer = 0.0f;
        messageTimer = 0.0f;
        messageVisible = true;

        // ステップ変更エフェクト
        stepJustChanged = true;
        stepChangeEffectTimer = 0.0f;
        CreateStepChangeEffect();

        // ステップ変更時の効果音
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_SELECT);

        // 完了メッセージ
        if (currentStep == STEP_COMPLETED) {
            AddFloatingText("TUTORIAL COMPLETE!", SCREEN_W / 2, SCREEN_H / 2 - 100, GetColor(255, 215, 0), true);
            CompleteTutorial();
        }
        else {
            AddFloatingText("Step Complete!", tutorialPlayer.GetX(), tutorialPlayer.GetY() - 150, GetColor(100, 255, 255), true);
        }

        char debugMsg[128];
        sprintf_s(debugMsg, "TutorialScene: Advanced to step %d\n", currentStep);
        OutputDebugStringA(debugMsg);
    }
}

void TutorialScene::CompleteTutorial()
{
    // 完了時の特別エフェクト
    for (int i = 0; i < 20; i++) {
        CreateSuccessParticles(SCREEN_W / 2 + (rand() % 400 - 200), SCREEN_H / 2 + (rand() % 200 - 100));
    }

    OutputDebugStringA("TutorialScene: Tutorial completed with celebration effects!\n");
}

void TutorialScene::UpdateFade()
{
    const float FADE_SPEED = 0.02f;

    if (fadingIn) {
        fadeAlpha -= FADE_SPEED;
        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fadingIn = false;
        }
    }
    else if (fadingOut) {
        fadeAlpha += FADE_SPEED;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            fadingOut = false;
        }
    }
}

void TutorialScene::UpdateEffects()
{
    UpdateParticles();
    UpdateFloatingTexts();
    UpdateInputIndicators();
    UpdateUIEffects();
}

void TutorialScene::UpdateParticles()
{
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= 0.016f;
        it->x += it->velocityX;
        it->y += it->velocityY;
        it->velocityY += 0.2f; // 重力
        it->scale = it->life / it->maxLife;

        if (it->life <= 0.0f) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void TutorialScene::UpdateFloatingTexts()
{
    for (auto it = floatingTexts.begin(); it != floatingTexts.end();) {
        it->life -= 0.016f;
        it->y += it->velocityY;

        if (it->life <= 0.0f) {
            it = floatingTexts.erase(it);
        }
        else {
            ++it;
        }
    }
}

void TutorialScene::UpdateInputIndicators()
{
    // 入力インジケーター更新
    leftIndicator.isActive = leftPressed;
    rightIndicator.isActive = rightPressed;
    spaceIndicator.isActive = spacePressed;
    downIndicator.isActive = downPressed;

    // インジケーターの強度を更新
    auto updateIndicator = [](InputIndicator& indicator) {
        if (indicator.isActive) {
            indicator.intensity = min(1.0f, indicator.intensity + 0.1f);
            indicator.timer += 0.016f;
        }
        else {
            indicator.intensity = max(0.0f, indicator.intensity - 0.05f);
            indicator.timer = 0.0f;
        }
        };

    updateIndicator(leftIndicator);
    updateIndicator(rightIndicator);
    updateIndicator(spaceIndicator);
    updateIndicator(downIndicator);
}

void TutorialScene::UpdateUIEffects()
{
    uiPulseTimer += 0.016f;
    iconRotation += 1.0f;
    if (iconRotation >= 360.0f) iconRotation -= 360.0f;

    progressBarGlow = sinf(uiPulseTimer * 3.0f) * 0.3f + 0.7f;

    if (stepJustChanged) {
        stepChangeEffectTimer += 0.016f;
        if (stepChangeEffectTimer >= 1.0f) {
            stepJustChanged = false;
        }
    }
}

void TutorialScene::CreateSuccessParticles(float x, float y)
{
    for (int i = 0; i < 8; i++) {
        Particle p;
        p.x = x + (rand() % 40 - 20);
        p.y = y + (rand() % 40 - 20);
        p.velocityX = (rand() % 100 - 50) * 0.1f;
        p.velocityY = (rand() % 100 - 150) * 0.1f;
        p.life = p.maxLife = 1.0f + (rand() % 100) * 0.01f;
        p.color = GetColor(100 + rand() % 155, 200 + rand() % 55, 100 + rand() % 155);
        p.scale = 1.0f;
        particles.push_back(p);
    }
}

void TutorialScene::CreateStepChangeEffect()
{
    // ステップ変更時の控えめなエフェクト
    for (int i = 0; i < 15; i++) {
        Particle p;
        p.x = SCREEN_W / 2 + (rand() % 200 - 100);
        p.y = 280 + (rand() % 40 - 20);
        p.velocityX = (rand() % 60 - 30) * 0.1f;
        p.velocityY = (rand() % 40 - 60) * 0.1f;
        p.life = p.maxLife = 2.0f;
        p.color = GetColor(255, 215, 0);
        p.scale = 1.0f;
        particles.push_back(p);
    }
}

void TutorialScene::AddFloatingText(const string& text, float x, float y, int color, bool important)
{
    FloatingText ft;
    ft.text = text;
    ft.x = x;
    ft.y = y;
    ft.velocityY = -1.0f;
    ft.life = ft.maxLife = important ? 3.0f : 2.0f;
    ft.color = color;
    ft.isImportant = important;
    floatingTexts.push_back(ft);
}

void TutorialScene::Draw()
{
    // 背景描画
    DrawBackground();

    // ステージ描画
    stageManager.Draw(cameraX);

    // プレイヤー描画
    tutorialPlayer.Draw(cameraX);
    tutorialPlayer.DrawShadow(cameraX, &stageManager);

    // ゴールマーカー描画
    if (currentStep >= STEP_GOAL) {
        int goalScreenX = (int)(goalX - cameraX);
        if (goalScreenX > -100 && goalScreenX < SCREEN_W + 100) {
            // より美しいゴール表示
            float glowIntensity = sinf(stepTimer * 3.0f) * 0.4f + 0.6f;

            // 外側のグロー
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * glowIntensity));
            DrawCircle(goalScreenX, Stage::GROUND_LEVEL - 50, 60, GetColor(255, 215, 0), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(180 * glowIntensity));
            DrawCircle(goalScreenX, Stage::GROUND_LEVEL - 50, 45, GetColor(255, 255, 100), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // 中央のコア
            DrawCircle(goalScreenX, Stage::GROUND_LEVEL - 50, 35, GetColor(255, 215, 0), TRUE);

            DrawStringToHandle(goalScreenX - 30, Stage::GROUND_LEVEL - 120, "GOAL",
                GetColor(255, 215, 0), largeFontHandle);
        }
    }

    // エフェクト描画
    DrawEffects();

    // チュートリアルUI描画
    DrawEnhancedUI();

    // HUD描画
    hudSystem.Draw();

    // フェード描画
    DrawFade();
}

void TutorialScene::DrawBackground()
{
    int bgWidth, bgHeight;
    GetGraphSize(backgroundHandle, &bgWidth, &bgHeight);

    float parallaxSpeed = 0.3f;
    float bgOffsetX = cameraX * parallaxSpeed;

    int startTileX = (int)(bgOffsetX / bgWidth) - 1;
    int endTileX = startTileX + (SCREEN_W / bgWidth) + 3;

    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
        float drawX = tileX * bgWidth - bgOffsetX;
        DrawExtendGraph((int)drawX, 0, (int)drawX + bgWidth, SCREEN_H, backgroundHandle, TRUE);
    }
}

void TutorialScene::DrawEnhancedUI()
{
    // メインパネルの背景（グロー効果付き）
    DrawGlowingBox(40, 40, SCREEN_W - 40, 260, GetColor(0, 0, 0), 0.8f);

    // パネルの枠（脈動効果）
    int frameAlpha = GetPulsingAlpha(255, 2.0f);
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, frameAlpha);
    for (int i = 0; i < 3; i++) {
        DrawBox(45 - i, 45 - i, SCREEN_W - 45 + i, 265 + i, GetColor(255, 215, 0), FALSE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // チュートリアルタイトル（脈動効果）
    string title = "TUTORIAL - " + characterName;
    DrawPulsingText(title, SCREEN_W / 2, 70, GetColor(255, 215, 0), largeFontHandle, 1.2f);

    // 現在のステップ情報表示
    if (currentStep < tutorialTexts.size()) {
        const auto& currentText = tutorialTexts[currentStep];

        // メインテキスト（エレガントな表示）
        DrawPulsingText(currentText.mainText, SCREEN_W / 2, 120, GetColor(255, 255, 255), fontHandle, 1.1f);

        // サブテキスト
        int subTextWidth = GetDrawStringWidthToHandle(currentText.subText.c_str(),
            (int)currentText.subText.length(), fontHandle);
        DrawStringToHandle(SCREEN_W / 2 - subTextWidth / 2, 160,
            currentText.subText.c_str(), GetColor(200, 200, 200), fontHandle);

        // 入力指示（点滅 + グロー効果）
        if (messageVisible && !currentText.inputText.empty()) {
            float glowIntensity = sinf(messageTimer * 4.0f) * 0.5f + 0.5f;

            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(100 * glowIntensity));
            int inputTextWidth = GetDrawStringWidthToHandle(currentText.inputText.c_str(),
                (int)currentText.inputText.length(), fontHandle);

            // グロー効果
            for (int i = 1; i <= 3; i++) {
                DrawStringToHandle(SCREEN_W / 2 - inputTextWidth / 2 + i, 200 + i,
                    currentText.inputText.c_str(), GetColor(255, 215, 0), fontHandle);
                DrawStringToHandle(SCREEN_W / 2 - inputTextWidth / 2 - i, 200 - i,
                    currentText.inputText.c_str(), GetColor(255, 215, 0), fontHandle);
            }
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // メインテキスト
            DrawStringToHandle(SCREEN_W / 2 - inputTextWidth / 2, 200,
                currentText.inputText.c_str(), GetColor(255, 215, 0), fontHandle);
        }
    }

    // ステップインジケーター描画
    DrawStepIndicator();

    // 基本操作説明描画
    DrawInstructions();
}

void TutorialScene::DrawStepIndicator()
{
    // ステップ進行度バー（強化版）
    int barWidth = 500;
    int barHeight = 25;
    int barX = SCREEN_W / 2 - barWidth / 2;
    int barY = 280;

    // バーの背景（深度感のある表示）
    for (int i = 3; i >= 0; i--) {
        int alpha = 150 - (i * 30);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(barX - i, barY - i, barX + barWidth + i, barY + barHeight + i, GetColor(20, 20, 20), TRUE);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 進行度バー（グラデーション効果）
    float progress = (float)currentStep / (STEP_COMPLETED);
    int progressWidth = (int)(barWidth * progress);

    // グラデーション進行度バー
    for (int i = 0; i < progressWidth; i++) {
        float ratio = (float)i / barWidth;
        int r = (int)(255 * (1.0f - ratio * 0.3f));
        int g = (int)(215 + 40 * ratio);
        int b = (int)(100 * ratio);

        DrawBox(barX + i, barY, barX + i + 1, barY + barHeight, GetColor(r, g, b), TRUE);
    }

    // プログレスバーのグロー効果
    if (stepJustChanged) {
        float effectIntensity = 1.0f - (stepChangeEffectTimer / 1.0f);
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(200 * effectIntensity));
        DrawBox(barX, barY - 5, barX + progressWidth, barY + barHeight + 5, GetColor(255, 255, 200), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // バーの枠線
    DrawBox(barX, barY, barX + barWidth, barY + barHeight, GetColor(255, 255, 255), FALSE);

    // ステップ番号（美しいフォント）
    string stepText = "Step " + to_string(currentStep + 1) + " / " + to_string(STEP_COMPLETED + 1);
    DrawPulsingText(stepText, SCREEN_W / 2, 320, GetColor(255, 255, 255), fontHandle, 1.0f);
}

void TutorialScene::DrawInstructions()
{
    // 操作説明（半透明背景付き、強化版）
    DrawGlowingBox(20, SCREEN_H - 220, SCREEN_W - 20, SCREEN_H - 20, GetColor(0, 0, 0), 0.7f);

    // タイトル
    DrawStringToHandle(30, SCREEN_H - 210, "Controls Guide:", GetColor(255, 255, 255), fontHandle);

    // 基本操作（アイコン風表示）
    int yOffset = SCREEN_H - 170;
    int lineHeight = 40;

    // 入力インジケーター付きの操作説明
    DrawInputIndicators();

    DrawStringToHandle(100, yOffset, "Left/Right Arrow: Move", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(100, yOffset + lineHeight, "Space: Jump", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(100, yOffset + lineHeight * 2, "Down Arrow: Duck/Slide", GetColor(200, 200, 200), fontHandle);

    // 特別な操作説明
    if (currentStep == STEP_COMPLETED) {
        DrawPulsingText("R: Return to Character Select, ESC: Start Game",
            30, SCREEN_H - 40, GetColor(255, 215, 0), fontHandle, 1.1f);
    }
    else {
        DrawStringToHandle(30, SCREEN_H - 40, "ESC: Exit Tutorial", GetColor(150, 150, 150), fontHandle);
    }

    // 現在の入力状態表示（デバッグ兼ユーザーフィードバック）
    string inputStatus = "Active: ";
    if (leftPressed) inputStatus += "[LEFT] ";
    if (rightPressed) inputStatus += "[RIGHT] ";
    if (spacePressed) inputStatus += "[SPACE] ";
    if (downPressed) inputStatus += "[DOWN] ";
    if (rPressed) inputStatus += "[R] ";
    if (inputStatus == "Active: ") inputStatus += "None";

    DrawStringToHandle(SCREEN_W - 450, SCREEN_H - 170, inputStatus.c_str(), GetColor(100, 255, 100), smallFontHandle);

    // プレイヤー状態表示
    string playerState = "Hero Status: ";
    switch (tutorialPlayer.GetState()) {
    case Player::IDLE: playerState += "Ready"; break;
    case Player::WALKING: playerState += "Moving"; break;
    case Player::JUMPING: playerState += "Soaring"; break;
    case Player::FALLING: playerState += "Descending"; break;
    case Player::DUCKING: playerState += "Crouching"; break;
    case Player::SLIDING: playerState += "Sliding"; break;
    default: playerState += "Training"; break;
    }

    DrawStringToHandle(SCREEN_W - 450, SCREEN_H - 130, playerState.c_str(), GetColor(100, 200, 255), smallFontHandle);

    // アクション完了状況
    string actions = "Mastered: ";
    if (hasMovedLeft) actions += "◄ ";
    if (hasMovedRight) actions += "► ";
    if (hasJumped) actions += "↑ ";
    if (hasDucked) actions += "↓ ";
    if (hasSlid) actions += "→ ";

    DrawStringToHandle(SCREEN_W - 450, SCREEN_H - 90, actions.c_str(), GetColor(255, 200, 100), smallFontHandle);
}

void TutorialScene::DrawEffects()
{
    DrawParticles();
    DrawFloatingTexts();
}

void TutorialScene::DrawParticles()
{
    for (const auto& particle : particles) {
        if (particle.life > 0) {
            int alpha = (int)(255 * (particle.life / particle.maxLife));
            int size = (int)(6 * particle.scale);

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawCircle((int)(particle.x - cameraX), (int)particle.y, size, particle.color, TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }
}

void TutorialScene::DrawFloatingTexts()
{
    for (const auto& text : floatingTexts) {
        if (text.life > 0) {
            int alpha = (int)(255 * (text.life / text.maxLife));
            int fontToUse = text.isImportant ? largeFontHandle : fontHandle;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            int textWidth = GetDrawStringWidthToHandle(text.text.c_str(), (int)text.text.length(), fontToUse);
            int drawX = (int)(text.x - cameraX) - textWidth / 2;

            // 重要なテキストにはグロー効果
            if (text.isImportant) {
                SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 3);
                for (int i = 1; i <= 2; i++) {
                    DrawStringToHandle(drawX + i, (int)text.y + i, text.text.c_str(), text.color, fontToUse);
                    DrawStringToHandle(drawX - i, (int)text.y - i, text.text.c_str(), text.color, fontToUse);
                }
            }

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawStringToHandle(drawX, (int)text.y, text.text.c_str(), text.color, fontToUse);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }
}

void TutorialScene::DrawInputIndicators()
{
    // 左右矢印キーのインジケーター
    int indicatorY = SCREEN_H - 170;

    // 左矢印
    if (leftIndicator.intensity > 0) {
        int alpha = (int)(200 * leftIndicator.intensity);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(60, indicatorY + 10, 15, GetColor(100, 255, 100), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawStringToHandle(55, indicatorY + 5, "◄", GetColor(255, 255, 255), fontHandle);
    }

    // 右矢印
    if (rightIndicator.intensity > 0) {
        int alpha = (int)(200 * rightIndicator.intensity);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(60, indicatorY + 50, 15, GetColor(100, 255, 100), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawStringToHandle(55, indicatorY + 45, "►", GetColor(255, 255, 255), fontHandle);
    }

    // スペースキー
    if (spaceIndicator.intensity > 0) {
        int alpha = (int)(200 * spaceIndicator.intensity);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(60, indicatorY + 90, 15, GetColor(255, 255, 100), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawStringToHandle(55, indicatorY + 85, "↑", GetColor(255, 255, 255), fontHandle);
    }

    // ダウン矢印
    if (downIndicator.intensity > 0) {
        int alpha = (int)(200 * downIndicator.intensity);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawCircle(60, indicatorY + 130, 15, GetColor(100, 200, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        DrawStringToHandle(55, indicatorY + 125, "↓", GetColor(255, 255, 255), fontHandle);
    }
}

void TutorialScene::DrawFade()
{
    if (fadeAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * fadeAlpha));
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void TutorialScene::DrawGlowingBox(int x1, int y1, int x2, int y2, int color, float glowIntensity)
{
    // 外側のグロー効果
    for (int i = 8; i > 0; i--) {
        int alpha = (int)(30 * glowIntensity * (1.0f - (float)i / 8.0f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(x1 - i, y1 - i, x2 + i, y2 + i, color, TRUE);
    }

    // メインボックス
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(180 * glowIntensity));
    DrawBox(x1, y1, x2, y2, color, TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void TutorialScene::DrawPulsingText(const string& text, int x, int y, int color, int fontHandle, float pulseIntensity)
{
    int textWidth = GetDrawStringWidthToHandle(text.c_str(), (int)text.length(), fontHandle);
    int drawX = x - textWidth / 2;

    // 脈動効果の計算
    float pulseValue = sinf(uiPulseTimer * 2.0f) * 0.2f + 1.0f;
    int alpha = GetPulsingAlpha(255, pulseIntensity);

    // グロー効果（控えめ）
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha / 4);
    for (int i = 1; i <= 2; i++) {
        DrawStringToHandle(drawX + i, y + i, text.c_str(), color, fontHandle);
        DrawStringToHandle(drawX - i, y - i, text.c_str(), color, fontHandle);
    }

    // メインテキスト
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
    DrawStringToHandle(drawX, y, text.c_str(), color, fontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

int TutorialScene::GetPulsingAlpha(float baseAlpha, float pulseSpeed)
{
    float pulse = sinf(uiPulseTimer * pulseSpeed) * 0.3f + 0.7f;
    return (int)(baseAlpha * pulse);
}

void TutorialScene::DrawTutorialUI()
{
    // 後方互換性のため、新しいDrawEnhancedUI()を呼び出す
    DrawEnhancedUI();
}

string TutorialScene::GetCharacterDisplayName(int index)
{
    switch (index) {
    case 0: return "Beige Knight";
    case 1: return "Green Ranger";
    case 2: return "Pink Warrior";
    case 3: return "Purple Mage";
    case 4: return "Yellow Hero";
    default: return "Unknown Hero";
    }
}

float TutorialScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

bool TutorialScene::IsPlayerInRange(float minX, float maxX)
{
    float playerX = tutorialPlayer.GetX();
    return playerX >= minX && playerX <= maxX;
}