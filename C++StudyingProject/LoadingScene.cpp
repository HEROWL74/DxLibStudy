#include "LoadingScene.h"
#include "SoundManager.h"
#include <algorithm>
#include <math.h>

LoadingScene::LoadingScene()
    : backgroundTexture(-1)
    , loadingIconTexture(-1)
    , fontHandle(-1)
    , titleFontHandle(-1)
    , loadingActive(false)
    , loadingComplete(false)
    , loadingProgress(0.0f)
    , displayProgress(0.0f)
    , currentTaskIndex(0)
    , currentTaskProgress(0.0f)
    , animationTimer(0.0f)
    , iconRotation(0.0f)
    , pulsePhase(0.0f)
    , currentLoadingType(LOADING_GAME_START)
    , characterIndex(-1)
    , stageIndex(-1)
    , fadeAlpha(0.0f)
    , fadeIn(true)
    , fadeOut(false)
    , currentTaskText("")
    , loadingTitle("Loading...")
{
}

LoadingScene::~LoadingScene()
{
    if (backgroundTexture != -1) DeleteGraph(backgroundTexture);
    if (loadingIconTexture != -1) DeleteGraph(loadingIconTexture);
    if (fontHandle != -1) DeleteFontToHandle(fontHandle);
    if (titleFontHandle != -1) DeleteFontToHandle(titleFontHandle);
}

void LoadingScene::Initialize()
{
    LoadTextures();
    ResetState();
}

void LoadingScene::LoadTextures()
{
    // ローディング画面用の背景（シンプルなグラデーション想定）
    backgroundTexture = LoadGraph("Sprites/Backgrounds/loading_background.png");
    if (backgroundTexture == -1) {
        // ファイルが見つからない場合は、プレースホルダーとして無効値のまま
        OutputDebugStringA("LoadingScene: Background texture not found, using solid color\n");
    }

    // ローディングアイコン
    loadingIconTexture = LoadGraph("Sprites/Tiles/Saw.png"); // または spinner.png
    if (loadingIconTexture == -1) {
        OutputDebugStringA("LoadingScene: Loading icon not found, using default graphics\n");
    }

    // フォント作成
    fontHandle = CreateFontToHandle(NULL, 24, 3);
    titleFontHandle = CreateFontToHandle(NULL, 36, 4);
}

void LoadingScene::StartLoading(LoadingType type, int selectedCharacter, int targetStage)
{
    currentLoadingType = type;
    characterIndex = selectedCharacter;
    stageIndex = targetStage;

    ResetState();
    SetupTasks();

    loadingActive = true;
    loadingComplete = false;

    // ローディングタイプに応じたタイトル設定
    switch (type) {
    case LOADING_GAME_START:
        loadingTitle = "Initializing Game...";
        break;
    case LOADING_STAGE_CHANGE:
        loadingTitle = "Loading Stage " + std::to_string(targetStage + 1) + "...";
        break;
    case LOADING_CHARACTER_CHANGE:
        loadingTitle = "Loading Character...";
        break;
    case LOADING_RESOURCES:
        loadingTitle = "Loading Resources...";
        break;
    }

    OutputDebugStringA("LoadingScene: Started loading\n");
}

void LoadingScene::SetupTasks()
{
    loadingTasks.clear();

    switch (currentLoadingType) {
    case LOADING_GAME_START:
        SetupGameStartTasks();
        break;
    case LOADING_STAGE_CHANGE:
        SetupStageChangeTasks();
        break;
    case LOADING_CHARACTER_CHANGE:
        SetupCharacterChangeTasks();
        break;
    case LOADING_RESOURCES:
        SetupResourceTasks();
        break;
    }
}

void LoadingScene::SetupGameStartTasks()
{
    loadingTasks.emplace_back("Loading game resources...",
        [this]() { return LoadGameResources(); }, 0.3f);

    loadingTasks.emplace_back("Loading character data...",
        [this]() { return LoadCharacterSprites(characterIndex); }, 0.2f);

    loadingTasks.emplace_back("Loading sound system...",
        [this]() { return LoadSoundResources(); }, 0.2f);

    loadingTasks.emplace_back("Loading UI elements...",
        [this]() { return LoadUIResources(); }, 0.15f);

    loadingTasks.emplace_back("Initializing game systems...",
        [this]() { return InitializeGameSystems(); }, 0.1f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.05f);
}

void LoadingScene::SetupStageChangeTasks()
{
    loadingTasks.emplace_back("Loading stage data...",
        [this]() { return LoadStageData(stageIndex); }, 0.4f);

    loadingTasks.emplace_back("Loading stage graphics...",
        [this]() { return LoadGameResources(); }, 0.3f);

    loadingTasks.emplace_back("Preparing enemies...",
        [this]() { return InitializeGameSystems(); }, 0.2f);

    loadingTasks.emplace_back("Finalizing stage...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::SetupCharacterChangeTasks()
{
    loadingTasks.emplace_back("Loading character sprites...",
        [this]() { return LoadCharacterSprites(characterIndex); }, 0.6f);

    loadingTasks.emplace_back("Updating character data...",
        [this]() { return InitializeGameSystems(); }, 0.3f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::SetupResourceTasks()
{
    loadingTasks.emplace_back("Loading textures...",
        [this]() { return LoadGameResources(); }, 0.4f);

    loadingTasks.emplace_back("Loading sounds...",
        [this]() { return LoadSoundResources(); }, 0.3f);

    loadingTasks.emplace_back("Loading UI...",
        [this]() { return LoadUIResources(); }, 0.2f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::Update()
{
    if (!loadingActive || loadingComplete) return;

    UpdateProgress();
    UpdateAnimations();
    UpdateFade();
}

void LoadingScene::UpdateProgress()
{
    if (currentTaskIndex >= loadingTasks.size()) {
        loadingComplete = true;
        fadeOut = true;
        return;
    }

    // 現在のタスクを実行
    LoadingTask& currentTask = loadingTasks[currentTaskIndex];
    if (!currentTask.completed) {
        currentTaskText = currentTask.description;

        // タスクを実行（複数フレームにわたって実行される可能性がある）
        bool taskCompleted = currentTask.taskFunc();

        if (taskCompleted) {
            currentTask.completed = true;
            currentTaskIndex++;

            // デバッグ出力
            OutputDebugStringA(("LoadingScene: Completed - " + currentTask.description + "\n").c_str());
        }
    }

    // 全体の進行度を計算
    float totalWeight = 0.0f;
    float completedWeight = 0.0f;

    for (size_t i = 0; i < loadingTasks.size(); i++) {
        totalWeight += loadingTasks[i].weight;
        if (loadingTasks[i].completed) {
            completedWeight += loadingTasks[i].weight;
        }
    }

    loadingProgress = (totalWeight > 0.0f) ? (completedWeight / totalWeight) : 0.0f;

    // 表示用進行度をスムーズに更新
    displayProgress += (loadingProgress - displayProgress) * PROGRESS_SMOOTH_SPEED;
}

void LoadingScene::UpdateAnimations()
{
    animationTimer += 0.016f; // 60FPS想定

    // アイコンの回転
    iconRotation += ICON_ROTATION_SPEED;
    if (iconRotation >= 2.0f * DX_PI) {
        iconRotation -= 2.0f * DX_PI;
    }

    // パルス効果
    pulsePhase += PULSE_SPEED;
    if (pulsePhase >= 2.0f * DX_PI) {
        pulsePhase -= 2.0f * DX_PI;
    }
}

void LoadingScene::UpdateFade()
{
    if (fadeIn) {
        fadeAlpha += FADE_SPEED;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            fadeIn = false;
        }
    }
    else if (fadeOut) {
        fadeAlpha -= FADE_SPEED;
        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fadeOut = false;
            loadingActive = false;
        }
    }
}

void LoadingScene::Draw()
{
    if (!loadingActive) return;

    DrawBackground();
    DrawProgressBar();
    DrawLoadingIcon();
    DrawLoadingText();  // 名前変更: DrawText → DrawLoadingText
    DrawFade();
}

void LoadingScene::DrawBackground()
{
    if (backgroundTexture != -1) {
        // 背景画像を画面全体に描画
        DrawExtendGraph(0, 0, 1920, 1080, backgroundTexture, TRUE);
    }
    else {
        // 背景画像がない場合は、グラデーション風の背景を描画
        for (int y = 0; y < 1080; y++) {
            int alpha = 255 - (y * 100 / 1080);
            int color = GetColor(20 + alpha / 10, 30 + alpha / 8, 50 + alpha / 5);
            DrawLine(0, y, 1920, y, color);
        }
    }
}

void LoadingScene::DrawProgressBar()
{
    const int BAR_WIDTH = 600;
    const int BAR_HEIGHT = 20;
    const int BAR_X = (1920 - BAR_WIDTH) / 2;  // 完全に中央配置
    const int BAR_Y = 1080 / 2 + 80;  // 少し上に移動

    // プログレスバーの背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(BAR_X - 2, BAR_Y - 2, BAR_X + BAR_WIDTH + 2, BAR_Y + BAR_HEIGHT + 2,
        GetColor(255, 255, 255), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawBox(BAR_X, BAR_Y, BAR_X + BAR_WIDTH, BAR_Y + BAR_HEIGHT,
        GetColor(40, 40, 60), TRUE);

    // プログレスバーの進行部分
    int progressWidth = (int)(BAR_WIDTH * displayProgress);
    if (progressWidth > 0) {
        // グラデーション効果
        for (int x = 0; x < progressWidth; x++) {
            float ratio = (float)x / progressWidth;
            int r = (int)(100 + 100 * ratio);
            int g = (int)(150 + 50 * ratio);
            int b = (int)(255 - 50 * ratio);

            DrawLine(BAR_X + x, BAR_Y, BAR_X + x, BAR_Y + BAR_HEIGHT,
                GetColor(r, g, b));
        }

        // 光る効果
        float glowIntensity = sinf(animationTimer * 3.0f) * 0.3f + 0.7f;
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(80 * glowIntensity));
        DrawBox(BAR_X, BAR_Y, BAR_X + progressWidth, BAR_Y + BAR_HEIGHT,
            GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 進行度テキスト（完全中央配置）
    std::string progressText = std::to_string((int)(displayProgress * 100)) + "%";
    int textWidth = GetDrawStringWidthToHandle(progressText.c_str(),
        progressText.length(), fontHandle);
    DrawStringToHandle(1920 / 2 - textWidth / 2, BAR_Y + BAR_HEIGHT + 15,
        progressText.c_str(), GetColor(255, 255, 255), fontHandle);
}

void LoadingScene::DrawLoadingIcon()
{
    const int ICON_SIZE = 80;
    const int ICON_X = 1920 / 2;  // 完全に中央
    const int ICON_Y = 1080 / 2 - 80;  // 少し上に調整

    if (loadingIconTexture != -1) {
        // アイコンの回転描画
        float scale = 1.0f + sinf(pulsePhase) * 0.1f; // パルス効果
        int scaledSize = (int)(ICON_SIZE * scale);

        // 回転描画（DxLibには直接的な回転描画がないため、簡易的な効果）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
        DrawExtendGraph(
            ICON_X - scaledSize / 2, ICON_Y - scaledSize / 2,
            ICON_X + scaledSize / 2, ICON_Y + scaledSize / 2,
            loadingIconTexture, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else {
        // アイコンがない場合は、回転する円を描画
        float scale = 1.0f + sinf(pulsePhase) * 0.2f;
        int radius = (int)(30 * scale);

        // 回転する円弧
        for (int i = 0; i < 8; i++) {
            float angle = iconRotation + (i * DX_PI / 4);
            int x = ICON_X + (int)(cosf(angle) * radius);
            int y = ICON_Y + (int)(sinf(angle) * radius);

            int alpha = 255 - (i * 25);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawCircle(x, y, 8, GetColor(100, 150, 255), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::DrawLoadingText()  // 名前変更: DrawText → DrawLoadingText
{
    // タイトル（完全中央配置）
    int titleWidth = GetDrawStringWidthToHandle(loadingTitle.c_str(),
        loadingTitle.length(), titleFontHandle);
    DrawStringToHandle(1920 / 2 - titleWidth / 2, 1080 / 2 - 180,  // 少し上に移動
        loadingTitle.c_str(), GetColor(255, 255, 255), titleFontHandle);

    // 現在のタスク説明（完全中央配置）
    if (!currentTaskText.empty()) {
        int taskWidth = GetDrawStringWidthToHandle(currentTaskText.c_str(),
            currentTaskText.length(), fontHandle);

        // 点滅効果
        float alpha = 0.7f + 0.3f * sinf(animationTimer * 2.0f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
        DrawStringToHandle(1920 / 2 - taskWidth / 2, 1080 / 2 + 160,  // 位置調整
            currentTaskText.c_str(), GetColor(200, 200, 255), fontHandle);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::DrawFade()
{
    if (fadeAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * (1.0f - fadeAlpha)));
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::AddCustomTask(const std::string& description,
    std::function<bool()> task, float weight)
{
    loadingTasks.emplace_back(description, task, weight);
}

void LoadingScene::ResetState()
{
    loadingActive = false;
    loadingComplete = false;
    loadingProgress = 0.0f;
    displayProgress = 0.0f;
    currentTaskIndex = 0;
    currentTaskProgress = 0.0f;
    animationTimer = 0.0f;
    iconRotation = 0.0f;
    pulsePhase = 0.0f;
    fadeAlpha = 0.0f;
    fadeIn = true;
    fadeOut = false;
    currentTaskText = "";

    loadingTasks.clear();
}

float LoadingScene::EaseInOut(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

// ローディング処理の実装関数

bool LoadingScene::LoadGameResources()
{
    static int loadingStep = 0;
    static int frameCounter = 0;

    frameCounter++;

    // 段階的に読み込み（フレーム分散）- より長い待機時間
    switch (loadingStep) {
    case 0:
        // ステップ1：基本リソース（1.5秒）
        if (frameCounter >= 90) { // 90フレーム = 1.5秒
            loadingStep++;
            frameCounter = 0;
        }
        break;
    case 1:
        // ステップ2：ゲームリソース（2秒）
        if (frameCounter >= 120) { // 120フレーム = 2秒
            loadingStep++;
            frameCounter = 0;
        }
        break;
    case 2:
        // ステップ3：最終化（1秒）
        if (frameCounter >= 60) { // 60フレーム = 1秒
            loadingStep = 0;
            frameCounter = 0;
            return true;
        }
        break;
    }

    return false;
}

bool LoadingScene::LoadCharacterSprites(int characterIdx)
{
    static int frameCounter = 0;
    frameCounter++;

    // キャラクタースプライトの読み込みをシミュレート（2秒）
    if (frameCounter >= 120) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadStageData(int stageIdx)
{
    static int frameCounter = 0;
    frameCounter++;

    // ステージデータの読み込みをシミュレート（2.5秒）
    if (frameCounter >= 150) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadSoundResources()
{
    static int frameCounter = 0;
    frameCounter++;

    // サウンドリソースの読み込みをシミュレート（3秒）
    if (frameCounter >= 180) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadUIResources()
{
    static int frameCounter = 0;
    frameCounter++;

    // UIリソースの読み込みをシミュレート（1.5秒）
    if (frameCounter >= 90) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::InitializeGameSystems()
{
    static int frameCounter = 0;
    frameCounter++;

    // ゲームシステムの初期化をシミュレート（2秒）
    if (frameCounter >= 120) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::WaitMinimumTime()
{
    static float waitTimer = 0.0f;
    waitTimer += 0.016f;

    // 最小表示時間を確保（3秒）
    if (waitTimer >= MIN_LOADING_TIME) {
        waitTimer = 0.0f;
        return true;
    }

    return false;
}