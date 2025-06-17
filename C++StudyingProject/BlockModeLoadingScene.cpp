#include "BlockModeLoadingScene.h"
#include "SoundManager.h"
#include <cmath>
#include <algorithm>
#include <random>
using namespace std;

BlockModeLoadingScene::BlockModeLoadingScene()
    : loadingComplete(false)
    , loadingProgress(0.0f)
    , loadingTimer(0.0f)
    , selectedCharacterIndex(0)
    , particleSpawnTimer(0.0f)
    , titlePulsePhase(0.0f)
    , iconFloatPhase(0.0f)
    , glowPulsePhase(0.0f)
    , currentStage(STAGE_INTRO)
    , stageTimer(0.0f)
    , currentMessageIndex(0)
    , messageTimer(0.0f)
{
    // ハンドル初期化
    backgroundHandle = -1;
    fontHandle = largeFontHandle = titleFontHandle = -1;
    dirtBlockHandle = stoneBlockHandle = crateBlockHandle = -1;
    coinHandle = explosiveHandle = playerIconHandle = -1;
    toolHammerHandle = toolPickaxeHandle = -1;
    progressBarBgHandle = progressBarFillHandle = -1;
    decorFrameHandle = glowEffectHandle = -1;
}

BlockModeLoadingScene::~BlockModeLoadingScene()
{
    // リソース解放
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);
    DeleteFontToHandle(titleFontHandle);

    DeleteGraph(dirtBlockHandle);
    DeleteGraph(stoneBlockHandle);
    DeleteGraph(crateBlockHandle);
    DeleteGraph(coinHandle);
    DeleteGraph(explosiveHandle);
    // キャラクター用テクスチャも削除
    DeleteGraph(playerIconHandle);
    DeleteGraph(toolHammerHandle);
    DeleteGraph(toolPickaxeHandle);
    DeleteGraph(progressBarBgHandle);
    DeleteGraph(progressBarFillHandle);
    DeleteGraph(decorFrameHandle);
    DeleteGraph(glowEffectHandle);
}

void BlockModeLoadingScene::Initialize()
{
    LoadTextures();

    // ローディングメッセージの準備（ほのぼの調に変更）
    loadingMessages = {
        "Setting up your cozy block world...",
        "Preparing friendly building blocks...",
        "Creating a safe play environment...",
        "Loading colorful block pieces...",
        "Setting up fun adventures...",
        "Preparing your building tools...",
        "Almost ready for block fun...",
        "Welcome to Block Athletics!"
    };

    // ほのぼのしたローディングTips
    loadingTips = {
        "Take your time and enjoy the adventure",
        "Collect coins at your own pace",
        "Use arrow keys to move around",
        "Jump with the space bar when ready",
        "Look for hidden surprises along the way",
        "Building blocks can be fun to destroy too",
        "Every player finds their own path",
        "Have fun and don't worry about speed"
    };

    OutputDebugStringA("BlockModeLoadingScene: Professional loading system initialized\n");
}

void BlockModeLoadingScene::StartLoading(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    characterName = GetCharacterDisplayName(selectedCharacter);

    // 状態リセット
    loadingComplete = false;
    loadingProgress = 0.0f;
    loadingTimer = 0.0f;
    particleSpawnTimer = 0.0f;
    titlePulsePhase = 0.0f;
    iconFloatPhase = 0.0f;
    glowPulsePhase = 0.0f;
    currentStage = STAGE_INTRO;
    stageTimer = 0.0f;
    currentMessageIndex = 0;
    messageTimer = 0.0f;

    // エフェクトクリア
    particles.clear();
    loadingElements.clear();

    // ローディング要素初期化
    InitializeLoadingElements();

    // BGM開始（静かでプロフェッショナルな雰囲気）
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

    char debugMsg[256];
    sprintf_s(debugMsg, "BlockModeLoadingScene: Started loading for character %d (%s)\n",
        selectedCharacter, characterName.c_str());
    OutputDebugStringA(debugMsg);
}

void BlockModeLoadingScene::Update()
{
    const float deltaTime = 0.016f; // 60FPS想定

    UpdateLoading();
    UpdateParticles();
    UpdateLoadingElements();
    UpdateLoadingStage();

    // アニメーションフェーズ更新
    titlePulsePhase += TITLE_PULSE_SPEED;
    iconFloatPhase += ICON_FLOAT_SPEED;
    glowPulsePhase += GLOW_PULSE_SPEED;

    // メッセージ更新
    messageTimer += deltaTime;
    if (messageTimer >= MESSAGE_DURATION) {
        messageTimer = 0.0f;
        currentMessageIndex = (currentMessageIndex + 1) % loadingMessages.size();
    }

    // パーティクル生成
    particleSpawnTimer += deltaTime;
    if (particleSpawnTimer >= PARTICLE_SPAWN_RATE) {
        particleSpawnTimer = 0.0f;
        CreateParticles();
    }
}

void BlockModeLoadingScene::UpdateLoading()
{
    const float deltaTime = 0.016f;
    loadingTimer += deltaTime;

    // スムーズなプログレス計算（イージング適用）
    float rawProgress = loadingTimer / LOADING_DURATION;
    if (rawProgress > 1.0f) rawProgress = 1.0f;

    loadingProgress = EaseOutCubic(rawProgress);

    // ローディング完了チェック
    if (loadingTimer >= LOADING_DURATION && !loadingComplete) {
        loadingComplete = true;
        currentStage = STAGE_COMPLETE;

        // 完了エフェクト
        for (int i = 0; i < 30; i++) {
            CreateSparkleParticle(
                SCREEN_W / 2 + (rand() % 400 - 200),
                SCREEN_H / 2 + (rand() % 200 - 100)
            );
        }

        OutputDebugStringA("BlockModeLoadingScene: Loading completed with professional effects\n");
    }
}

void BlockModeLoadingScene::UpdateParticles()
{
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= 0.016f;
        it->x += it->velocityX;
        it->y += it->velocityY;
        it->velocityY += 0.3f; // 重力
        it->rotation += it->rotationSpeed;
        it->scale = it->life / it->maxLife;

        if (it->life <= 0.0f) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void BlockModeLoadingScene::UpdateLoadingElements()
{
    for (auto& element : loadingElements) {
        if (!element.isVisible) continue;

        // ターゲット位置への移動
        element.x = Lerp(element.x, element.targetX, 0.08f);
        element.y = Lerp(element.y, element.targetY, 0.08f);

        // アニメーション更新
        element.animationPhase += 0.05f;
        element.rotation += 0.02f;

        // フローティング効果
        element.y += sinf(element.animationPhase) * 2.0f;
    }
}

void BlockModeLoadingScene::UpdateLoadingStage()
{
    stageTimer += 0.016f;
    float stageProgress = loadingProgress * 4.0f; // 4ステージに分割

    if (stageProgress >= 1.0f && currentStage == STAGE_INTRO) {
        currentStage = STAGE_BUILDING;
        stageTimer = 0.0f;

        // 構築ステージ用エフェクト
        for (int i = 0; i < 10; i++) {
            CreateBlockParticle(
                300 + i * 100,
                SCREEN_H / 2,
                rand() % 3
            );
        }
    }
    else if (stageProgress >= 2.0f && currentStage == STAGE_BUILDING) {
        currentStage = STAGE_FINALIZING;
        stageTimer = 0.0f;
    }
    else if (stageProgress >= 3.0f && currentStage == STAGE_FINALIZING) {
        currentStage = STAGE_COMPLETE;
        stageTimer = 0.0f;
    }
}

void BlockModeLoadingScene::CreateParticles()
{
    if (particles.size() >= MAX_PARTICLES) return;

    // ほのぼのしたブロックが上からゆっくり降ってくる
    switch (currentStage) {
    case STAGE_INTRO:
        if (rand() % 4 == 0) {
            CreateBlockParticle(
                rand() % SCREEN_W,
                -50,  // 画面上部から開始
                rand() % 3  // 土、石、クレート
            );
        }
        break;

    case STAGE_BUILDING:
        if (rand() % 5 == 0) {
            CreateBlockParticle(
                rand() % SCREEN_W,
                -50,
                rand() % 5  // 全種類のブロック
            );
        }
        if (rand() % 8 == 0) {
            CreateCoinParticle(
                rand() % SCREEN_W,
                -50
            );
        }
        break;

    case STAGE_FINALIZING:
        if (rand() % 6 == 0) {
            CreateSparkleParticle(
                rand() % SCREEN_W,
                -30
            );
        }
        break;

    default:
        break;
    }
}

void BlockModeLoadingScene::CreateBlockParticle(float x, float y, int blockType)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 40 - 20) * 0.02f;  // ゆっくりした横移動
    p.velocityY = (rand() % 30 + 20) * 0.03f;  // ゆっくりした落下
    p.life = p.maxLife = 8.0f + (rand() % 400) * 0.01f;  // 長い寿命
    p.scale = 0.6f + (rand() % 40) * 0.01f;    // 小さめサイズ
    p.rotation = 0.0f;
    p.rotationSpeed = (rand() % 100 - 50) * 0.001f;  // ゆっくり回転
    p.type = blockType;

    // 温かみのある色合い
    switch (blockType) {
    case 0: p.color = GetColor(160, 100, 60); break;   // 温かい土色
    case 1: p.color = GetColor(150, 140, 130); break;  // 優しい石色
    case 2: p.color = GetColor(180, 120, 80); break;   // 温かいクレート色
    case 3: p.color = GetColor(200, 140, 100); break;  // 優しい爆発クレート色
    case 4: p.color = GetColor(220, 200, 80); break;   // 優しいコイン色
    }

    particles.push_back(p);
}

void BlockModeLoadingScene::CreateSparkleParticle(float x, float y)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 100 - 50) * 0.08f;
    p.velocityY = (rand() % 100 - 50) * 0.08f;
    p.life = p.maxLife = 2.0f;
    p.scale = 1.2f;
    p.rotation = 0.0f;
    p.rotationSpeed = (rand() % 200 - 100) * 0.01f;
    p.type = 1;
    p.color = GetColor(255, 255, 100 + rand() % 155);

    particles.push_back(p);
}

void BlockModeLoadingScene::CreateCoinParticle(float x, float y)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 80 - 40) * 0.06f;
    p.velocityY = (rand() % 60 - 80) * 0.06f;
    p.life = p.maxLife = 2.5f;
    p.scale = 1.0f;
    p.rotation = 0.0f;
    p.rotationSpeed = 0.1f;
    p.type = 2;
    p.color = GetColor(255, 215, 0);

    particles.push_back(p);
}

void BlockModeLoadingScene::InitializeLoadingElements()
{
    loadingElements.clear();

    // 画面中央にブロック要素を配置
    int centerX = SCREEN_W / 2;
    int centerY = SCREEN_H / 2;

    for (int i = 0; i < 8; i++) {
        LoadingElement element;
        element.x = -100; // 画面外から開始
        element.y = centerY + (i - 4) * 60;
        element.targetX = centerX - 300 + i * 80;
        element.targetY = element.y;
        element.scale = 1.0f;
        element.rotation = 0.0f;
        element.alpha = 1.0f;
        element.animationPhase = i * 0.3f;
        element.isVisible = true;
        element.elementType = i % 4;

        loadingElements.push_back(element);
    }
}

void BlockModeLoadingScene::UpdateElementAnimations()
{
}

void BlockModeLoadingScene::Draw()
{
    DrawBackground();
    DrawLoadingElements();
    DrawBlockConstructionAnimation();
    DrawCharacterInfo();
    DrawTitleAndInfo();
    DrawProgressBar();
    DrawLoadingTips();
    DrawParticles();
    DrawEffects();
}

void BlockModeLoadingScene::DrawBackground()
{
    // ほのぼのした砂色背景（タイリング）
    if (backgroundHandle != -1) {
        int bgWidth = 512;
        int bgHeight = 512;

        int tilesX = (SCREEN_W / bgWidth) + 2;
        int tilesY = (SCREEN_H / bgHeight) + 2;

        for (int x = 0; x < tilesX; x++) {
            for (int y = 0; y < tilesY; y++) {
                DrawGraph(x * bgWidth, y * bgHeight, backgroundHandle, TRUE);
            }
        }
    }
    else {
        // フォールバック：温かみのあるグラデーション
        for (int y = 0; y < SCREEN_H; y += 4) {
            float t = (float)y / SCREEN_H;
            int r = (int)Lerp(245, 222, t);  // 砂色のグラデーション
            int g = (int)Lerp(222, 184, t);
            int b = (int)Lerp(179, 135, t);

            DrawLine(0, y, SCREEN_W, y, GetColor(r, g, b));
            DrawLine(0, y + 1, SCREEN_W, y + 1, GetColor(r, g, b));
            DrawLine(0, y + 2, SCREEN_W, y + 2, GetColor(r, g, b));
            DrawLine(0, y + 3, SCREEN_W, y + 3, GetColor(r, g, b));
        }
    }
}

void BlockModeLoadingScene::DrawTitleAndInfo()
{
    // ほのぼのタイトル（控えめなパルス効果）
    string mainTitle = "Block Athletics Loading...";

    // 優しいタイトル描画（グローを控えめに）
    int titleWidth = GetDrawStringWidthToHandle(mainTitle.c_str(), mainTitle.length(), largeFontHandle);
    int titleX = SCREEN_W / 2 - titleWidth / 2;

    // 影効果（優しい色）
    DrawStringToHandle(titleX + 2, 122, mainTitle.c_str(), GetColor(150, 120, 80), largeFontHandle);
    DrawStringToHandle(titleX, 120, mainTitle.c_str(), GetColor(80, 60, 40), largeFontHandle);

    // サブタイトル
    string subtitle = "Preparing your cozy adventure...";
    int subtitleWidth = GetDrawStringWidthToHandle(subtitle.c_str(), subtitle.length(), fontHandle);
    int subtitleX = SCREEN_W / 2 - subtitleWidth / 2;
    DrawStringToHandle(subtitleX, 180, subtitle.c_str(), GetColor(120, 100, 70), fontHandle);

    // 現在のローディングメッセージ（優しい色調）
    if (currentMessageIndex < loadingMessages.size()) {
        string message = loadingMessages[currentMessageIndex];
        int messageWidth = GetDrawStringWidthToHandle(message.c_str(), message.length(), fontHandle);
        int messageX = SCREEN_W / 2 - messageWidth / 2;

        // メッセージの背景（優しい色）
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
        DrawBox(messageX - 20, 700, messageX + messageWidth + 20, 740, GetColor(200, 180, 140), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        DrawStringToHandle(messageX, 710, message.c_str(), GetColor(90, 70, 50), fontHandle);
    }
}

void BlockModeLoadingScene::DrawProgressBar()
{
    DrawAnimatedProgressBar();
}

void BlockModeLoadingScene::DrawAnimatedProgressBar()
{
    int barX = SCREEN_W / 2 - PROGRESS_BAR_WIDTH / 2;
    int barY = SCREEN_H - 200;

    // ほのぼの背景
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(barX - 5, barY - 5, barX + PROGRESS_BAR_WIDTH + 5, barY + PROGRESS_BAR_HEIGHT + 5,
        GetColor(160, 130, 90), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 背景
    DrawBox(barX, barY, barX + PROGRESS_BAR_WIDTH, barY + PROGRESS_BAR_HEIGHT,
        GetColor(220, 200, 160), TRUE);

    // プログレス部分（温かい色調）
    int progressWidth = (int)(PROGRESS_BAR_WIDTH * loadingProgress);
    if (progressWidth > 0) {
        // 温かいグラデーション
        for (int i = 0; i < progressWidth; i += 4) {
            float t = (float)i / progressWidth;
            int r = (int)Lerp(180, 140, t);
            int g = (int)Lerp(140, 110, t);
            int b = (int)Lerp(100, 70, t);

            DrawBox(barX + i, barY + 2, barX + i + 4, barY + PROGRESS_BAR_HEIGHT - 2,
                GetColor(r, g, b), TRUE);
        }

        // 控えめな光沢効果
        SetDrawBlendMode(DX_BLENDMODE_ADD, 60);
        DrawBox(barX + 2, barY + 4, barX + progressWidth - 2, barY + PROGRESS_BAR_HEIGHT / 2,
            GetColor(255, 220, 180), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // 枠線（優しい色）
    DrawBox(barX, barY, barX + PROGRESS_BAR_WIDTH, barY + PROGRESS_BAR_HEIGHT,
        GetColor(120, 100, 70), FALSE);

    // パーセント表示
    char percentText[32];
    sprintf_s(percentText, "%.1f%%", loadingProgress * 100);
    int percentWidth = GetDrawStringWidthToHandle(percentText, strlen(percentText), fontHandle);
    DrawStringToHandle(barX + PROGRESS_BAR_WIDTH / 2 - percentWidth / 2, barY + 5,
        percentText, GetColor(80, 60, 40), fontHandle);
}

void BlockModeLoadingScene::DrawCharacterInfo()
{
    // 選択されたキャラクター情報（ほのぼの調）
    int infoX = 100;
    int infoY = 300;

    // キャラクター情報パネル（温かい色調）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(infoX - 20, infoY - 20, infoX + 400, infoY + 200, GetColor(210, 190, 150), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // パネルの枠線（優しい色）
    DrawBox(infoX - 20, infoY - 20, infoX + 400, infoY + 200, GetColor(140, 120, 80), FALSE);

    // キャラクター名
    string charTitle = "Your Character:";
    DrawStringToHandle(infoX, infoY, charTitle.c_str(), GetColor(100, 80, 60), fontHandle);
    DrawStringToHandle(infoX, infoY + 40, characterName.c_str(), GetColor(80, 60, 40), largeFontHandle);

    // キャラクターステータス（ほのぼの文言）
    DrawStringToHandle(infoX, infoY + 100, "Status: Ready for Adventure!", GetColor(90, 130, 70), fontHandle);
    DrawStringToHandle(infoX, infoY + 140, "Specialty: Block Building Fun", GetColor(90, 110, 130), fontHandle);
}

void BlockModeLoadingScene::DrawStageInfo()
{
}

void BlockModeLoadingScene::DrawLoadingTips()
{
    // ほのぼのしたTipを表示
    static int currentTipIndex = 0;
    static float tipTimer = 0.0f;

    tipTimer += 0.016f;
    if (tipTimer >= 3.0f) { // 3秒ごとにTip変更（ゆっくり）
        tipTimer = 0.0f;
        currentTipIndex = (currentTipIndex + 1) % loadingTips.size();
    }

    int tipX = SCREEN_W - 500;
    int tipY = 300;

    // Tipパネル（ほのぼの色調）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(tipX - 20, tipY - 20, tipX + 480, tipY + 180, GetColor(200, 180, 140), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawBox(tipX - 20, tipY - 20, tipX + 480, tipY + 180, GetColor(120, 100, 70), FALSE);

    // Tip内容（優しい色）
    DrawStringToHandle(tipX, tipY, "Helpful Tips:", GetColor(90, 70, 50), fontHandle);

    if (currentTipIndex < loadingTips.size()) {
        string tip = loadingTips[currentTipIndex];
        DrawStringToHandle(tipX, tipY + 50, tip.c_str(), GetColor(80, 60, 40), fontHandle);
    }
}

void BlockModeLoadingScene::DrawBlockConstructionAnimation()
{
    // ほのぼのブロック組み立てアニメーション
    int constructionX = SCREEN_W / 2 - 300;
    int constructionY = SCREEN_H / 2 - 50;

    // 構築進度に応じてブロックを表示（ゆっくり）
    int blocksToShow = (int)(loadingProgress * 8);

    for (int i = 0; i < blocksToShow && i < 8; i++) {
        int blockX = constructionX + (i % 4) * 120;
        int blockY = constructionY + (i / 4) * 120;

        // ブロックの種類をインデックスで決定
        int blockType = i % 5;
        int blockHandle = -1;

        switch (blockType) {
        case 0: blockHandle = dirtBlockHandle; break;
        case 1: blockHandle = stoneBlockHandle; break;
        case 2: blockHandle = crateBlockHandle; break;
        case 3: blockHandle = explosiveHandle; break;
        case 4: blockHandle = coinHandle; break;
        }

        // ブロックの登場アニメーション（ゆっくり）
        float appearProgress = (loadingProgress * 8 - i);
        if (appearProgress > 1.0f) appearProgress = 1.0f;
        if (appearProgress < 0.0f) appearProgress = 0.0f;

        float scale = EaseOutCubic(appearProgress);
        int size = (int)(100 * scale);

        if (size > 0 && blockHandle != -1) {
            // 優しい影効果
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
            DrawBox(blockX + 5, blockY + 5, blockX + size + 5, blockY + size + 5, GetColor(120, 100, 70), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // 実際のブロックテクスチャを描画
            int drawX = blockX + (100 - size) / 2;
            int drawY = blockY + (100 - size) / 2;

            DrawExtendGraph(drawX, drawY, drawX + size, drawY + size, blockHandle, TRUE);

            // 控えめな光沢効果
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 * scale));
            DrawBox(drawX + 5, drawY + 5, drawX + size - 5, drawY + size / 3, GetColor(255, 240, 200), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // 選択されたキャラクターもほのぼの表示
    if (loadingProgress > 0.7f && playerIconHandle != -1) {
        float charScale = EaseOutCubic((loadingProgress - 0.7f) * 3.33f);
        int charSize = (int)(120 * charScale);

        if (charSize > 0) {
            int charX = SCREEN_W / 2 + 150;
            int charY = SCREEN_H / 2 - 40;

            // 優しい影
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
            DrawBox(charX + 5, charY + 5, charX + charSize + 5, charY + charSize + 5, GetColor(120, 100, 70), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // キャラクターテクスチャ描画
            DrawExtendGraph(charX, charY, charX + charSize, charY + charSize, playerIconHandle, TRUE);
        }
    }
}

void BlockModeLoadingScene::DrawLoadingElements()
{
    for (const auto& element : loadingElements) {
        if (!element.isVisible) continue;

        // 浮遊効果
        float yOffset = sinf(element.animationPhase) * 5.0f;

        // 要素タイプに応じた描画
        int elementColor;
        switch (element.elementType) {
        case 0: elementColor = GetColor(255, 215, 0); break;  // コイン
        case 1: elementColor = GetColor(255, 100, 100); break; // 爆発物
        case 2: elementColor = GetColor(100, 255, 100); break; // ツール
        case 3: elementColor = GetColor(100, 200, 255); break; // その他
        }

        int elementX = (int)element.x;
        int elementY = (int)(element.y + yOffset);
        int elementSize = (int)(40 * element.scale);

        // グロー効果
        DrawGlowEffect(elementX - 5, elementY - 5, elementSize + 10, elementSize + 10,
            0.4f, elementColor);

        // 要素本体（回転効果付き）
        float rotation = element.rotation;
        DrawCircle(elementX + elementSize / 2, elementY + elementSize / 2, elementSize / 2,
            elementColor, TRUE);

        // 内部の詳細
        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
        DrawCircle(elementX + elementSize / 2, elementY + elementSize / 2, elementSize / 4,
            GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void BlockModeLoadingScene::DrawParticles()
{
    for (const auto& particle : particles) {
        if (particle.life <= 0.0f) continue;

        int alpha = (int)(180 * (particle.life / particle.maxLife));  // 控えめなアルファ
        int size = (int)(16 * particle.scale);  // 少し大きめのパーティクル

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        switch (particle.type) {
        case 0: // ブロック片（丸い形でほのぼの）
        case 2: // コイン
            DrawCircle((int)particle.x, (int)particle.y, size, particle.color, TRUE);
            // 控えめなハイライト
            DrawCircle((int)particle.x - 2, (int)particle.y - 2, size / 3, GetColor(255, 240, 200), TRUE);
            break;

        case 1: // スパークル（優しい星型）
            // 優しい星型の描画
            for (int i = 0; i < 4; i++) {
                float angle = particle.rotation + i * 1.57f; // 90度ずつ
                int x1 = (int)(particle.x + cosf(angle) * size * 0.8f);
                int y1 = (int)(particle.y + sinf(angle) * size * 0.8f);

                DrawCircle(x1, y1, size / 4, particle.color, TRUE);
            }
            DrawCircle((int)particle.x, (int)particle.y, size / 2, particle.color, TRUE);
            break;
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void BlockModeLoadingScene::DrawEffects()
{
    // 完了時のほのぼのエフェクト
    if (currentStage == STAGE_COMPLETE) {
        float completeGlow = sinf(glowPulsePhase) * 0.3f + 0.7f;  // 控えめなパルス

        // 画面全体の優しい光
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(30 * completeGlow));
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 240, 200), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // 完了メッセージ（ほのぼの）
        string completeText = "Ready for your cozy adventure!";
        int completeWidth = GetDrawStringWidthToHandle(completeText.c_str(), completeText.length(), largeFontHandle);
        int completeX = SCREEN_W / 2 - completeWidth / 2;

        // 優しい影
        DrawStringToHandle(completeX + 2, SCREEN_H / 2 + 122, completeText.c_str(), GetColor(150, 120, 80), largeFontHandle);
        DrawStringToHandle(completeX, SCREEN_H / 2 + 120, completeText.c_str(), GetColor(80, 60, 40), largeFontHandle);
    }
}

void BlockModeLoadingScene::DrawGlowEffect(int x, int y, int width, int height, float intensity, int color)
{
    if (intensity <= 0.01f) return;

    int alpha = (int)(intensity * 100);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // 複数レイヤーでグロー効果
    for (int i = 0; i < 5; i++) {
        int offset = (i + 1) * 3;
        DrawBox(x - offset, y - offset, x + width + offset, y + height + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void BlockModeLoadingScene::DrawPulsingText(const std::string& text, int x, int y, int fontHandle, int color, float pulsePhase)
{
    float pulse = 1.0f + sinf(pulsePhase * 2) * 0.1f;

    // 影効果
    DrawStringToHandle(x + 3, y + 3, text.c_str(), GetColor(0, 0, 0), fontHandle);

    // グロー効果
    for (int i = 0; i < 3; i++) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 30);
        DrawStringToHandle(x - i, y - i, text.c_str(), color, fontHandle);
        DrawStringToHandle(x + i, y + i, text.c_str(), color, fontHandle);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // メインテキスト
    DrawStringToHandle(x, y, text.c_str(), color, fontHandle);
}

void BlockModeLoadingScene::LoadTextures()
{
    // ほのぼの背景テクスチャ
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_solid_sand.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);
    titleFontHandle = CreateFontToHandle(NULL, 64, 7);

    // ブロック関連テクスチャ（BlockAthleticsSceneと同じ画像を使用）
    dirtBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_dirt.png");
    stoneBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_stone.png");
    crateBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate.png");
    coinHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_box_coin.png");
    explosiveHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate_explosive.png");

    // キャラクター用テクスチャも追加
    switch (selectedCharacterIndex) {
    case 0: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_d.png"); break; // beige
    case 1: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_a.png"); break; // green
    case 2: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_e.png"); break; // pink
    case 3: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_b.png"); break; // purple
    case 4: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_c.png"); break; // yellow
    default: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_d.png"); break;
    }

    OutputDebugStringA("BlockModeLoadingScene: Cozy textures loaded\n");
}

std::string BlockModeLoadingScene::GetCharacterDisplayName(int index)
{
    // BlockAthleticsSceneと同じキャラクター名を使用
    switch (index) {
    case 0: return "Beige Builder";
    case 1: return "Green Constructor";
    case 2: return "Pink Architect";
    case 3: return "Purple Engineer";
    case 4: return "Yellow Athlete";
    default: return "Block Runner";
    }
}

std::string BlockModeLoadingScene::GetRandomLoadingTip()
{
    if (loadingTips.empty()) return "";
    return loadingTips[rand() % loadingTips.size()];
}

float BlockModeLoadingScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float BlockModeLoadingScene::EaseOutCubic(float t)
{
    return 1 - powf(1 - t, 3);
}

float BlockModeLoadingScene::EaseInOutQuad(float t)
{
    return t < 0.5f ? 2 * t * t : 1 - powf(-2 * t + 2, 2) / 2;
}

int BlockModeLoadingScene::GetRandomColor()
{
    int colors[] = {
        GetColor(255, 100, 100),
        GetColor(100, 255, 100),
        GetColor(100, 100, 255),
        GetColor(255, 255, 100),
        GetColor(255, 100, 255),
        GetColor(100, 255, 255)
    };

    return colors[rand() % 6];
}

void BlockModeLoadingScene::ResetState()
{
    loadingComplete = false;
    loadingProgress = 0.0f;
    loadingTimer = 0.0f;
    particleSpawnTimer = 0.0f;
    titlePulsePhase = 0.0f;
    iconFloatPhase = 0.0f;
    glowPulsePhase = 0.0f;
    currentStage = STAGE_INTRO;
    stageTimer = 0.0f;
    currentMessageIndex = 0;
    messageTimer = 0.0f;

    particles.clear();
    loadingElements.clear();

    OutputDebugStringA("BlockModeLoadingScene: State reset for new loading session\n");
}