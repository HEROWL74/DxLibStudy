#include "BlockAthleticsScene.h"
#include <cmath>
#include <algorithm>
#include "BlockSystem.h"
using namespace std;

// 静的メンバの定義
int BlockAthleticsScene::STAGE_WIDTH = 3840;

BlockAthleticsScene::BlockAthleticsScene()
    : exitRequested(false)
    , gameCompleted(false)
    , currentState(STATE_PLAYING)
    , selectedCharacterIndex(0)
    , cameraX(0.0f)
    , previousPlayerX(0.0f)
    , coinsCollected(0)
    , totalCoins(0)
    , cratesDestroyed(0)
    , totalCrates(0)
    , playerLife(6)  // ライフの初期化を追加
    , gameTimer(0.0f)
    , bestTime(0.0f)
    , escPressed(false), escPressedPrev(false)
    , spacePressed(false), spacePressedPrev(false)
    , leftPressed(false), rightPressed(false)
    , downPressed(false), downPressedPrev(false)
    , bPressed(false), bPressedPrev(false)
    , showGoalHint(false)
    , goalHintTimer(0.0f)
{
    // ハンドル初期化
    backgroundHandle = -1;
    fontHandle = -1;
    largeFontHandle = -1;

    for (int i = 0; i < 5; i++) {
        characterHandles[i] = -1;
    }

    dirtHandle = dirtTopHandle = dirtDetailHandle = -1;
    stoneHandle = stoneTopHandle = stoneDetailHandle = -1;
    crateHandle = explosiveCrateHandle = coinHandle = -1;

    // ゴールエリア初期化
    goalArea = { 0.0f, 0.0f, 0.0f, 0.0f, false, 0.0f, -1, -1 };
}

BlockAthleticsScene::~BlockAthleticsScene()
{
    // リソース解放
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);

    for (int i = 0; i < 5; i++) {
        DeleteGraph(characterHandles[i]);
    }

    DeleteGraph(dirtHandle);
    DeleteGraph(dirtTopHandle);
    DeleteGraph(dirtDetailHandle);
    DeleteGraph(stoneHandle);
    DeleteGraph(stoneTopHandle);
    DeleteGraph(stoneDetailHandle);
    DeleteGraph(crateHandle);
    DeleteGraph(explosiveCrateHandle);
    DeleteGraph(coinHandle);
}

void BlockAthleticsScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    characterName = GetCharacterDisplayName(selectedCharacter);

    // テクスチャ読み込み
    LoadTextures();

    // ステージ初期化
    InitializeStage();

    // プレイヤー初期化
    InitializePlayer();

    // ゴールエリア初期化
    InitializeGoalArea();

    // HUD初期化
    hudSystem.Initialize();
    hudSystem.SetPlayerCharacter(selectedCharacterIndex);
    hudSystem.SetMaxLife(6);
    hudSystem.SetCurrentLife(6);  // 初期ライフ設定
    hudSystem.SetPosition(30, 30);
    hudSystem.SetVisible(true);

    // ゲーム状態リセット
    currentState = STATE_PLAYING;
    gameCompleted = false;
    exitRequested = false;
    gameTimer = 0.0f;
    coinsCollected = 0;
    cratesDestroyed = 0;
    playerLife = 6;  // ライフの初期化を追加

    // カメラ初期化
    cameraX = 0.0f;
    previousPlayerX = player.x;

    // エフェクト初期化
    particles.clear();

    // BGM開始
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_GAME);

    OutputDebugStringA("BlockAthleticsScene: Initialized successfully\n");
}

void BlockAthleticsScene::LoadTextures()
{
    // 背景とフォント
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);

    // ブロックテクスチャ
    dirtHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_dirt.png");
    dirtTopHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_dirt_top.png");
    dirtDetailHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_dirt_detail.png");
    stoneHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_stone.png");
    stoneTopHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_stone_top.png");
    stoneDetailHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_stone_detail.png");
    crateHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate.png");
    explosiveCrateHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate_explosive.png");
    coinHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_box_coin.png");

    // キャラクターテクスチャ（a=green, b=purple, c=yellow, d=beige, e=pink）
    characterHandles[0] = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_d.png"); // beige
    characterHandles[1] = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_a.png"); // green
    characterHandles[2] = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_e.png"); // pink
    characterHandles[3] = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_b.png"); // purple
    characterHandles[4] = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_c.png"); // yellow
}

// **新規追加: 破片テクスチャ生成**
void BlockAthleticsScene::LoadFragmentTextures()
{
    // 各ブロックタイプの破片を生成
    CreateFragmentFromTexture(dirtHandle, BLOCK_DIRT);
    CreateFragmentFromTexture(dirtTopHandle, BLOCK_DIRT_TOP);
    CreateFragmentFromTexture(stoneHandle, BLOCK_STONE);
    CreateFragmentFromTexture(stoneTopHandle, BLOCK_STONE_TOP);
    CreateFragmentFromTexture(crateHandle, BLOCK_CRATE);
    CreateFragmentFromTexture(explosiveCrateHandle, BLOCK_EXPLOSIVE);
    CreateFragmentFromTexture(coinHandle, BLOCK_COIN);

    OutputDebugStringA("BlockAthleticsScene: Fragment textures created successfully\n");
}

void BlockAthleticsScene::CreateFragmentFromTexture(int originalHandle, BlockType blockType)
{
    if (originalHandle == -1) return;

    // 元テクスチャのサイズを取得
    int originalWidth, originalHeight;
    GetGraphSize(originalHandle, &originalWidth, &originalHeight);

    // 4分割サイズ
    int fragmentWidth = originalWidth / 2;
    int fragmentHeight = originalHeight / 2;

    // 4つの破片を作成
    for (int i = 0; i < 4; i++) {
        // 分割位置計算
        int srcX = (i % 2) * fragmentWidth;
        int srcY = (i / 2) * fragmentHeight;

        // 新しいテクスチャ作成
        int fragmentHandle = MakeScreen(fragmentWidth, fragmentHeight, TRUE);

        if (fragmentHandle != -1) {
            // 元の描画先を保存
            int prevTarget = GetDrawScreen();

            // 新しいテクスチャに描画
            SetDrawScreen(fragmentHandle);
            ClearDrawScreen();

            // 元テクスチャの一部を描画
            DrawRectGraph(0, 0, srcX, srcY, fragmentWidth, fragmentHeight, originalHandle, TRUE, FALSE);

            // 描画先を復元
            SetDrawScreen(prevTarget);

            // ハンドルを保存
            fragmentHandles[blockType][i] = fragmentHandle;

            char debugMsg[128];
            sprintf_s(debugMsg, "BlockAthleticsScene: Created fragment %d for block type %d\n", i, blockType);
            OutputDebugStringA(debugMsg);
        }
    }
}

void BlockAthleticsScene::InitializeStage()
{
    blocks.clear();
    totalCoins = 0;
    totalCrates = 0;

    // より遊びやすい平地中心のアスレチックコース設計
    // === セクション1: スタートエリア ===
    // スタート地点の安全地帯
    for (int x = 0; x < 4; x++) {
        Block dirtBlock = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(dirtBlock);
        Block dirtTopBlock = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(dirtTopBlock);
    }

    // === セクション2: 基本ジャンプ練習エリア ===
    // 段差ジャンプ（低め）
    Block stoneTop1 = { BLOCK_STONE_TOP, BLOCK_SIZE * 6, STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(stoneTop1);
    Block stone1 = { BLOCK_STONE, BLOCK_SIZE * 6, STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
    blocks.push_back(stone1);

    Block stoneTop2 = { BLOCK_STONE_TOP, BLOCK_SIZE * 8, STAGE_HEIGHT - BLOCK_SIZE * 3, true, false, 0.0f, false };
    blocks.push_back(stoneTop2);
    Block stone2 = { BLOCK_STONE, BLOCK_SIZE * 8, STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(stone2);
    Block stone3 = { BLOCK_STONE, BLOCK_SIZE * 8, STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
    blocks.push_back(stone3);

    // 最初のコイン（低い位置）
    Block coin1 = { BLOCK_COIN, BLOCK_SIZE * 7, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(coin1);
    totalCoins++;

    // === セクション3: 障害物回避エリア ===
    // クレートの壁（低め）
    for (int y = 0; y < 2; y++) {
        Block crate = { BLOCK_CRATE, BLOCK_SIZE * 10, STAGE_HEIGHT - BLOCK_SIZE * (2 + y), true, true, 0.0f, true };
        blocks.push_back(crate);
        totalCrates++;
    }

    // 回避用の足場（適度な高さ）
    Block dirtTop1 = { BLOCK_DIRT_TOP, BLOCK_SIZE * 12, STAGE_HEIGHT - BLOCK_SIZE * 3, true, false, 0.0f, false };
    blocks.push_back(dirtTop1);
    Block dirt1 = { BLOCK_DIRT, BLOCK_SIZE * 12, STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(dirt1);

    // === セクション4: 精密ジャンプエリア ===
    // 小さな足場の連続（低め）
    for (int i = 0; i < 5; i++) {
        int x = 14 + i * 2;
        int height = 2 + (i % 2); // 高さを2-3ブロックに制限
        Block stoneTop = { BLOCK_STONE_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * height, true, false, 0.0f, false };
        blocks.push_back(stoneTop);
    }

    // 空中のコイン（届く高さ）
    Block coin2 = { BLOCK_COIN, BLOCK_SIZE * 15, STAGE_HEIGHT - BLOCK_SIZE * 4, true, true, 0.0f, false };
    blocks.push_back(coin2);
    totalCoins++;
    Block coin3 = { BLOCK_COIN, BLOCK_SIZE * 19, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(coin3);
    totalCoins++;

    // === セクション5: 平地エリア ===
    // 地上の足場
    for (int x = 24; x < 27; x++) {
        Block dirtTop = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(dirtTop);
        Block dirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(dirt);
    }

    // 地上のコイン
    Block coin4 = { BLOCK_COIN, BLOCK_SIZE * 25, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(coin4);
    totalCoins++;

    // === セクション6: 爆発クレートの迷路（低め） ===
    // L字型の爆発クレート配置（地上）
    for (int i = 0; i < 3; i++) {
        Block explosive1 = { BLOCK_EXPLOSIVE, BLOCK_SIZE * (28 + i), STAGE_HEIGHT - BLOCK_SIZE * 2, true, true, 0.0f, true };
        blocks.push_back(explosive1);
        totalCrates++;
    }
    Block explosive2 = { BLOCK_EXPLOSIVE, BLOCK_SIZE * 30, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, true };
    blocks.push_back(explosive2);
    totalCrates++;

    // 回避ルート（低い足場）
    Block stoneTop3 = { BLOCK_STONE_TOP, BLOCK_SIZE * 32, STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(stoneTop3);
    Block stone4 = { BLOCK_STONE, BLOCK_SIZE * 32, STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
    blocks.push_back(stone4);

    // === セクション7: 段差エリア ===
    // 適度な段差の足場
    for (int i = 0; i < 4; i++) {
        int x = 34 + i * 3;
        int height = 2 + (i % 2); // 2-3ブロックの高さ
        Block dirtTop = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * height, true, false, 0.0f, false };
        blocks.push_back(dirtTop);
        if (height > 2) {
            Block dirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (height - 1), true, false, 0.0f, false };
            blocks.push_back(dirt);
        }
    }

    // 段差のコイン
    Block coin5 = { BLOCK_COIN, BLOCK_SIZE * 37, STAGE_HEIGHT - BLOCK_SIZE * 4, true, true, 0.0f, false };
    blocks.push_back(coin5);
    totalCoins++;

    // === セクション8: 最終チャレンジエリア（適度な高さ） ===
    // 塔（高さを抑制）
    for (int y = 0; y < 4; y++) {
        Block stone = { BLOCK_STONE, BLOCK_SIZE * 46, STAGE_HEIGHT - BLOCK_SIZE * (1 + y), true, false, 0.0f, false };
        blocks.push_back(stone);
    }
    Block stoneTop4 = { BLOCK_STONE_TOP, BLOCK_SIZE * 46, STAGE_HEIGHT - BLOCK_SIZE * 5, true, false, 0.0f, false };
    blocks.push_back(stoneTop4);

    // 塔への足場（届く高さ）
    Block dirtTop2 = { BLOCK_DIRT_TOP, BLOCK_SIZE * 44, STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(dirtTop2);
    Block dirtTop3 = { BLOCK_DIRT_TOP, BLOCK_SIZE * 45, STAGE_HEIGHT - BLOCK_SIZE * 3, true, false, 0.0f, false };
    blocks.push_back(dirtTop3);

    // 塔のコイン（届く高さ）
    Block coin6 = { BLOCK_COIN, BLOCK_SIZE * 46, STAGE_HEIGHT - BLOCK_SIZE * 6, true, true, 0.0f, false };
    blocks.push_back(coin6);
    totalCoins++;

    // === セクション9: ゴールエリア ===
    // ゴール前の最後の障害（低め）
    for (int x = 48; x < 52; x++) {
        Block crate = { BLOCK_CRATE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, true, 0.0f, true };
        blocks.push_back(crate);
        totalCrates++;
    }

    // 最終コインボーナス（届く高さ）
    Block coin7 = { BLOCK_COIN, BLOCK_SIZE * 55, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(coin7);
    totalCoins++;
    Block coin8 = { BLOCK_COIN, BLOCK_SIZE * 57, STAGE_HEIGHT - BLOCK_SIZE * 4, true, true, 0.0f, false };
    blocks.push_back(coin8);
    totalCoins++;

    // === 追加エリア：平地コイン ===
    // 地上に散らばったコイン
    Block coin9 = { BLOCK_COIN, BLOCK_SIZE * 5, STAGE_HEIGHT - BLOCK_SIZE * 2, true, true, 0.0f, false };
    blocks.push_back(coin9);
    totalCoins++;
    Block coin10 = { BLOCK_COIN, BLOCK_SIZE * 13, STAGE_HEIGHT - BLOCK_SIZE * 2, true, true, 0.0f, false };
    blocks.push_back(coin10);
    totalCoins++;

    // ステージサイズ調整
    STAGE_WIDTH = BLOCK_SIZE * 65; // より長いコースに

    OutputDebugStringA("BlockAthleticsScene: Accessible athletics course initialized\n");

    char debugMsg[256];
    sprintf_s(debugMsg, "BlockAthleticsScene: Course stats - Coins: %d, Crates: %d, Length: %d blocks\n",
        totalCoins, totalCrates, STAGE_WIDTH / BLOCK_SIZE);
    OutputDebugStringA(debugMsg);
}

void BlockAthleticsScene::InitializeGoalArea()
{
    // ゴールエリアの設定（ステージ最後尾）
    goalArea.x = STAGE_WIDTH - BLOCK_SIZE * 6;  // ゴールエリアを6ブロック分に拡大
    goalArea.y = STAGE_HEIGHT - BLOCK_SIZE * 5;  // 高さも5ブロック分
    goalArea.width = BLOCK_SIZE * 6;
    goalArea.height = BLOCK_SIZE * 5;
    goalArea.isActive = false;  // 初期状態では非アクティブ
    goalArea.animationPhase = 0.0f;
    goalArea.goalTextureHandle = -1;
    goalArea.flagTextureHandle = -1;

    // ゴールフラグの豪華な台座を作成
    for (int x = 54; x < 60; x++) {
        for (int y = 1; y <= 3; y++) {
            Block stoneBlock = { BLOCK_STONE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * y, true, false, 0.0f, false };
            blocks.push_back(stoneBlock);
        }
        // 最上段は特別なトップブロック
        Block stoneTop = { BLOCK_STONE_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 4, true, false, 0.0f, false };
        blocks.push_back(stoneTop);
    }

    // ゴールの表示状態
    showGoalHint = false;
    goalHintTimer = 0.0f;

    OutputDebugStringA("BlockAthleticsScene: Goal area initialized\n");
}

void BlockAthleticsScene::InitializePlayer()
{
    player.characterIndex = selectedCharacterIndex;
    player.graphicHandle = characterHandles[selectedCharacterIndex];
    player.x = BLOCK_SIZE;
    player.y = STAGE_HEIGHT - BLOCK_SIZE * 3;
    player.velocityX = 0.0f;
    player.velocityY = 0.0f;
    player.onGround = false;
    player.facingRight = true;
    player.animationFrame = 0;
    player.animationTimer = 0.0f;
    player.jumpPower = -18.0f;  // ふわっとしたジャンプ力に調整
    player.moveSpeed = 10.0f;   // 移動速度はそのまま
}

void BlockAthleticsScene::Update()
{
    if (currentState == STATE_PLAYING) {
        gameTimer += 0.016f; // 60FPS想定
    }

    UpdateInput();

    if (currentState == STATE_PLAYING) {
        UpdatePlayer();
        UpdateBlocks();
        UpdateCollisions();
        UpdateBlockDestruction();
        UpdateGoalArea();
    }

    UpdateCamera();
    UpdateEffects();
    UpdateBlockFragments();

    // HUDを常に更新（ライフ表示のため）
    hudSystem.SetCurrentLife(playerLife);

    // ESCキーで終了
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
        OutputDebugStringA("BlockAthleticsScene: ESC pressed - exiting\n");
    }
}

void BlockAthleticsScene::UpdateGoalArea()
{
    // ゴールアクティブ条件：全コイン収集完了
    bool shouldActivate = (coinsCollected >= totalCoins);

    if (shouldActivate && !goalArea.isActive) {
        goalArea.isActive = true;
        showGoalHint = true;
        goalHintTimer = 5.0f;  // 5秒間ヒント表示

        // ゴールアクティブ化エフェクト
        for (int i = 0; i < 50; i++) {
            CreateParticles(goalArea.x + goalArea.width / 2, goalArea.y + goalArea.height / 2,
                GetColor(255, 215, 0), 3);
        }

        // 特別なサウンド再生
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);  // ゴール用の音があれば変更

        OutputDebugStringA("BlockAthleticsScene: GOAL AREA ACTIVATED!\n");
    }

    // アニメーション更新
    goalArea.animationPhase += 0.1f;
    if (goalArea.animationPhase >= 6.28f) {
        goalArea.animationPhase -= 6.28f;
    }

    // ヒントタイマー更新
    if (showGoalHint) {
        goalHintTimer -= 0.016f;
        if (goalHintTimer <= 0.0f) {
            showGoalHint = false;
        }
    }

    // ゴール到達チェック（新しい条件）
    if (goalArea.isActive) {
        float playerCenterX = player.x + BLOCK_SIZE / 2;
        float playerCenterY = player.y + BLOCK_SIZE / 2;

        if (playerCenterX >= goalArea.x && playerCenterX <= goalArea.x + goalArea.width &&
            playerCenterY >= goalArea.y && playerCenterY <= goalArea.y + goalArea.height) {

            if (currentState == STATE_PLAYING) {
                currentState = STATE_COMPLETED;
                gameCompleted = true;

                // 盛大なクリアエフェクト
                for (int i = 0; i < 100; i++) {
                    CreateParticles(goalArea.x + goalArea.width / 2, goalArea.y + goalArea.height / 2,
                        GetColor(255, 215, 0), 5);
                    CreateParticles(goalArea.x + goalArea.width / 2, goalArea.y + goalArea.height / 2,
                        GetColor(255, 255, 255), 3);
                }

                OutputDebugStringA("BlockAthleticsScene: GOAL REACHED! Game completed!\n");
            }
        }
    }
}

void BlockAthleticsScene::UpdateInput()
{
    escPressedPrev = escPressed;
    spacePressedPrev = spacePressed;
    downPressedPrev = downPressed;
    bPressedPrev = bPressed;  // **新規追加**

    escPressed = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;
    leftPressed = CheckHitKey(KEY_INPUT_LEFT) != 0;
    rightPressed = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    downPressed = CheckHitKey(KEY_INPUT_DOWN) != 0;
    bPressed = CheckHitKey(KEY_INPUT_B) != 0;  // **新規追加**

    // テスト用のライフ操作
    static bool key1Pressed = false, key1PressedPrev = false;
    static bool key2Pressed = false, key2PressedPrev = false;

    key1PressedPrev = key1Pressed;
    key2PressedPrev = key2Pressed;

    key1Pressed = CheckHitKey(KEY_INPUT_1) != 0;
    key2Pressed = CheckHitKey(KEY_INPUT_2) != 0;

    // テスト用操作
    if (key1Pressed && !key1PressedPrev) {
        if (playerLife > 0) {
            playerLife--;
            hudSystem.SetCurrentLife(playerLife);
            SoundManager::GetInstance().PlaySE(SoundManager::SFX_HURT);
            CreateParticles(player.x + BLOCK_SIZE / 2, player.y, GetColor(255, 0, 0), 10);
            OutputDebugStringA("BlockAthleticsScene: Manual life decrease for testing.\n");
        }
    }

    if (key2Pressed && !key2PressedPrev) {
        if (playerLife < 6) {
            playerLife++;
            hudSystem.SetCurrentLife(playerLife);
            CreateParticles(player.x + BLOCK_SIZE / 2, player.y, GetColor(0, 255, 0), 8);
            OutputDebugStringA("BlockAthleticsScene: Manual life increase for testing.\n");
        }
    }
}

void BlockAthleticsScene::UpdateBlockFragments()
{
    for (auto it = blockFragments.begin(); it != blockFragments.end();) {
        it->life -= 0.016f;
        it->x += it->velocityX;
        it->y += it->velocityY;
        it->velocityY += 0.4f; // 重力
        it->rotation += it->rotationSpeed;

        // スケールはライフに応じて徐々に小さく
        it->scale = (it->life / it->maxLife) * 0.9f + 0.1f;

        if (it->life <= 0.0f) {
            it = blockFragments.erase(it);
        }
        else {
            ++it;
        }
    }
}

void BlockAthleticsScene::UpdateEffects()
{
    // 既存のパーティクル更新
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

void BlockAthleticsScene::DrawBlockFragments()
{
    for (const auto& fragment : blockFragments) {
        if (fragment.life > 0 && fragment.graphicHandle != -1) {
            int alpha = (int)(255 * (fragment.life / fragment.maxLife));
            int size = (int)(BLOCK_SIZE / 2 * fragment.scale); // 1/2サイズの破片

            int screenX = (int)(fragment.x - cameraX);
            int screenY = (int)fragment.y;

            // 画面外カリング
            if (screenX < -size || screenX > SCREEN_W + size) continue;

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            // 回転描画
            float centerX = screenX + size / 2;
            float centerY = screenY + size / 2;

            DrawRotaGraph((int)centerX, (int)centerY, fragment.scale, fragment.rotation,
                fragment.graphicHandle, TRUE);

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }
}

void BlockAthleticsScene::UpdateBlockDestruction()
{
    // Bキーが押された時のブロック破壊処理
    if (bPressed && !bPressedPrev) {
        HandleBlockDestruction();
    }
}

void BlockAthleticsScene::UpdatePlayer()
{
    // ふわっとした重力適用
    player.velocityY += 0.5f;  // より軽やかな重力

    // 左右移動（より敏感な操作）
    if (leftPressed) {
        player.velocityX = -player.moveSpeed;
        player.facingRight = false;
        CreateMovementParticles(); // 移動時のパーティクル
    }
    else if (rightPressed) {
        player.velocityX = player.moveSpeed;
        player.facingRight = true;
        CreateMovementParticles(); // 移動時のパーティクル
    }
    else {
        player.velocityX *= 0.85f; // 少し強めの摩擦でより制御しやすく
    }

    // ふわっとしたジャンプ
    if (spacePressed && !spacePressedPrev && player.onGround) {
        player.velocityY = player.jumpPower;
        player.onGround = false;
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_JUMP);
    }

    // 高速落下（下キー押下時）
    if (downPressed && !player.onGround) {
        player.velocityY += 1.0f; // 少し弱めの高速落下
        CreateFastFallParticles();
    }

    // 水平移動と衝突判定
    float newX = player.x + player.velocityX;
    if (!IsBlockCollision(newX, player.y, BLOCK_SIZE, BLOCK_SIZE)) {
        player.x = newX;
    }
    else {
        player.velocityX = 0; // 壁に当たったら速度をリセット
        CreateWallHitParticles(); // 壁衝突エフェクト
    }

    // 垂直移動と衝突判定
    float newY = player.y + player.velocityY;
    if (!IsBlockCollision(player.x, newY, BLOCK_SIZE, BLOCK_SIZE)) {
        player.y = newY;
        player.onGround = false;
    }
    else {
        if (player.velocityY > 0) {
            // 下向きの移動で衝突 = 着地
            player.onGround = true;
            if (player.velocityY > 8.0f) { // 高速着地の閾値を下げる
                CreateHardLandingParticles();
            }
            else {
                CreateLandingParticles(); // 通常着地
            }
        }
        player.velocityY = 0;
    }

    // 画面外制限
    if (player.x < 0) player.x = 0;
    if (player.x > STAGE_WIDTH - BLOCK_SIZE) player.x = STAGE_WIDTH - BLOCK_SIZE;

    // 地面に落下したらリスポーン
    if (player.y > STAGE_HEIGHT + 100) {
        RespawnPlayer();
    }

    // アニメーション更新（移動状態に応じて）
    player.animationTimer += 0.016f;
    float animSpeed = abs(player.velocityX) > 1.0f ? 0.1f : 0.3f; // 移動時は高速アニメ
    if (player.animationTimer >= animSpeed) {
        player.animationFrame = (player.animationFrame + 1) % 4;
        player.animationTimer = 0.0f;
    }
}

void BlockAthleticsScene::UpdateBlocks()
{
    for (auto& block : blocks) {
        if (block.isActive) {
            // コインの回転アニメーション
            if (block.type == BLOCK_COIN) {
                block.animationPhase += 0.1f;
                if (block.animationPhase >= 6.28f) {
                    block.animationPhase -= 6.28f;
                }
            }
        }
    }
}

void BlockAthleticsScene::UpdateCollisions()
{
    // コイン（収集アイテム）との衝突判定のみ
    for (int i = 0; i < (int)blocks.size(); i++) {
        if (!blocks[i].isActive) continue;

        // コインのみ収集処理
        if (blocks[i].type != BLOCK_COIN) continue;

        // プレイヤーとコインの衝突判定
        float blockLeft = blocks[i].x;
        float blockRight = blocks[i].x + BLOCK_SIZE;
        float blockTop = blocks[i].y;
        float blockBottom = blocks[i].y + BLOCK_SIZE;

        float playerLeft = player.x;
        float playerRight = player.x + BLOCK_SIZE;
        float playerTop = player.y;
        float playerBottom = player.y + BLOCK_SIZE;

        // 衝突チェック（少し余裕を持たせる）
        if (playerRight > blockLeft + 10 && playerLeft < blockRight - 10 &&
            playerBottom > blockTop + 10 && playerTop < blockBottom - 10) {

            HandlePlayerBlockCollision(blocks[i], i);
        }
    }
}

void BlockAthleticsScene::HandlePlayerBlockCollision(const Block& block, int blockIndex)
{
    switch (block.type) {
    case BLOCK_COIN:
        if (block.isActive) {
            CollectCoin(blockIndex);
        }
        break;

    case BLOCK_CRATE:
    case BLOCK_EXPLOSIVE:
        // クレートは踏んでも消えない。単純に足場として機能
        // プレイヤーが上から着地した場合のみ着地処理
        if (player.velocityY > 0 && player.y < block.y) {
            player.y = block.y - BLOCK_SIZE;
            player.velocityY = 0;
            player.onGround = true;

            // 踏んだ時のエフェクトのみ（ブロックは破壊しない）
            if (block.type == BLOCK_EXPLOSIVE) {
                CreateParticles(block.x + BLOCK_SIZE / 2, block.y, GetColor(255, 100, 0), 5);
            }
            else {
                CreateParticles(block.x + BLOCK_SIZE / 2, block.y, GetColor(139, 69, 19), 3);
            }
        }
        break;

    case BLOCK_DIRT:
    case BLOCK_DIRT_TOP:
    case BLOCK_STONE:
    case BLOCK_STONE_TOP:
        // 固体ブロックとの衝突処理
        if (player.velocityY > 0 && player.y < block.y) {
            // 上からの着地
            player.y = block.y - BLOCK_SIZE;
            player.velocityY = 0;
            player.onGround = true;
        }
        break;

    default:
        break;
    }
}

void BlockAthleticsScene::HandleBlockDestruction()
{
    // プレイヤーの周囲のブロックをチェック
    float playerCenterX = player.x + BLOCK_SIZE / 2;
    float playerCenterY = player.y + BLOCK_SIZE / 2;
    const float DESTRUCTION_RANGE = BLOCK_SIZE * 1.5f; // 破壊範囲

    for (int i = 0; i < (int)blocks.size(); i++) {
        if (!blocks[i].isActive) continue;

        // コインは破壊不可
        if (blocks[i].type == BLOCK_COIN) continue;

        float blockCenterX = blocks[i].x + BLOCK_SIZE / 2;
        float blockCenterY = blocks[i].y + BLOCK_SIZE / 2;

        // 距離チェック
        float distance = sqrtf((playerCenterX - blockCenterX) * (playerCenterX - blockCenterX) +
            (playerCenterY - blockCenterY) * (playerCenterY - blockCenterY));

        if (distance <= DESTRUCTION_RANGE) {
            DestroyBlock(i);
            break; // 一度に一つのブロックのみ破壊
        }
    }
}

void BlockAthleticsScene::CreateBlockFragments(float x, float y, BlockType blockType)
{
    // 4つの破片を生成（2x2の格子状に分割）
    for (int i = 0; i < 4; i++) {
        BlockFragment fragment;

        // 破片の初期位置（ブロックを4分割）
        float offsetX = (i % 2) * (BLOCK_SIZE / 2);
        float offsetY = (i / 2) * (BLOCK_SIZE / 2);

        fragment.x = x + offsetX;
        fragment.y = y + offsetY;

        // 破片が飛び散る方向（中心から外向き + ランダム要素）
        float centerX = x + BLOCK_SIZE / 2;
        float centerY = y + BLOCK_SIZE / 2;
        float fragmentCenterX = fragment.x + BLOCK_SIZE / 4;
        float fragmentCenterY = fragment.y + BLOCK_SIZE / 4;

        float directionX = fragmentCenterX - centerX;
        float directionY = fragmentCenterY - centerY;

        // 正規化
        float length = sqrtf(directionX * directionX + directionY * directionY);
        if (length > 0) {
            directionX /= length;
            directionY /= length;
        }

        // 速度設定（基本方向 + ランダム）
        float baseSpeed = 4.0f + (rand() % 200) * 0.01f;
        float randomAngle = ((rand() % 120) - 60) * 0.0174f; // ±60度のランダム

        float cosA = cosf(randomAngle);
        float sinA = sinf(randomAngle);

        fragment.velocityX = (directionX * cosA - directionY * sinA) * baseSpeed;
        fragment.velocityY = (directionX * sinA + directionY * cosA) * baseSpeed - 3.0f; // 上向き成分

        // 回転設定
        fragment.rotation = (float)(rand() % 360) * 0.0174f;
        fragment.rotationSpeed = ((rand() % 400) - 200) * 0.005f;

        // ライフ設定
        fragment.life = fragment.maxLife = 3.0f + (rand() % 200) * 0.01f;

        // 見た目設定
        fragment.scale = 0.9f + (rand() % 20) * 0.01f;
        fragment.fragmentType = i;
        fragment.originalBlockType = blockType;
        fragment.graphicHandle = fragmentHandles[blockType][i];

        blockFragments.push_back(fragment);
    }

 
}

void BlockAthleticsScene::CollectCoin(int blockIndex)
{
    blocks[blockIndex].isActive = false;
    coinsCollected++;

    // 派手なコイン収集エフェクト
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(255, 215, 0), 15);
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(255, 255, 100), 10);
    CreateSparkleEffect(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2);

    SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

    char debugMsg[128];
    sprintf_s(debugMsg, "BlockAthleticsScene: Coin collected! %d/%d\n", coinsCollected, totalCoins);
    OutputDebugStringA(debugMsg);
}

void BlockAthleticsScene::DestroyCrate(int blockIndex)
{
    blocks[blockIndex].isActive = false;
    cratesDestroyed++;

    // 木箱破壊エフェクト
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(139, 69, 19), 20);
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(160, 82, 45), 15);
    CreateDebrisEffect(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2);

    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HIT);

    OutputDebugStringA("BlockAthleticsScene: Crate destroyed!\n");
}

void BlockAthleticsScene::ExplodeCrate(int blockIndex)
{
    blocks[blockIndex].isActive = false;
    cratesDestroyed++;

    // 超派手な爆発エフェクト
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(255, 100, 0), 25);
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(255, 255, 0), 20);
    CreateParticles(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2,
        GetColor(255, 0, 0), 15);
    CreateExplosionEffect(blocks[blockIndex].x + BLOCK_SIZE / 2, blocks[blockIndex].y + BLOCK_SIZE / 2);

    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HIT);

    OutputDebugStringA("BlockAthleticsScene: Explosive crate destroyed!\n");
}

void BlockAthleticsScene::CreateParticles(float x, float y, int color, int count)
{
    for (int i = 0; i < count; i++) {
        Particle p;
        p.x = x + (rand() % 60 - 30);
        p.y = y + (rand() % 60 - 30);
        p.velocityX = (rand() % 40 - 20) * 0.05f;
        p.velocityY = player.velocityY * 0.3f;
        p.life = p.maxLife = 0.8f;
        p.color = GetColor(100, 150, 255);
        p.scale = 0.6f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateWallHitParticles()
{
    for (int i = 0; i < 6; i++) {
        Particle p;
        p.x = player.x + (player.facingRight ? BLOCK_SIZE : 0);
        p.y = player.y + BLOCK_SIZE / 2 + (rand() % 60 - 30);
        p.velocityX = (player.facingRight ? -1 : 1) * (rand() % 80 + 20) * 0.1f;
        p.velocityY = (rand() % 80 - 40) * 0.1f;
        p.life = p.maxLife = 1.0f;
        p.color = GetColor(255, 200, 100);
        p.scale = 0.8f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::RespawnPlayer()
{
    // ライフを1つ減らす
    if (playerLife > 0) {
        playerLife--;
        hudSystem.SetCurrentLife(playerLife); // HUDを即座に更新

        // ダメージエフェクト
        CreateParticles(player.x + BLOCK_SIZE / 2, player.y + BLOCK_SIZE / 2, GetColor(255, 0, 0), 20);

        // ダメージ音
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_HURT);

        char debugMsg[128];
        sprintf_s(debugMsg, "BlockAthleticsScene: Player took fall damage! Life: %d\n", playerLife);
        OutputDebugStringA(debugMsg);
    }

    // プレイヤーを初期位置に戻す
    player.x = BLOCK_SIZE;
    player.y = STAGE_HEIGHT - BLOCK_SIZE * 3;
    player.velocityX = 0.0f;
    player.velocityY = 0.0f;
    player.onGround = false;

    // リスポーンエフェクト
    CreateParticles(player.x + BLOCK_SIZE / 2, player.y + BLOCK_SIZE / 2, GetColor(0, 255, 255), 15);

    // ライフが0になったらゲームオーバー
    if (playerLife <= 0) {
        // ゲームオーバー処理
        playerLife = 6; // ライフを全回復
        hudSystem.SetCurrentLife(playerLife);

        // 全てのアイテムをリセット
        InitializeStage();
        coinsCollected = 0;
        cratesDestroyed = 0;

        OutputDebugStringA("BlockAthleticsScene: Game Over! Stage reset with full life\n");
    }

    OutputDebugStringA("BlockAthleticsScene: Player respawned!\n");
}

void BlockAthleticsScene::UpdateCamera()
{
    float targetCameraX = player.x - SCREEN_W * 0.35f;

    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > STAGE_WIDTH - SCREEN_W) {
        targetCameraX = STAGE_WIDTH - SCREEN_W;
    }

    cameraX = Lerp(cameraX, targetCameraX, CAMERA_FOLLOW_SPEED);
    previousPlayerX = player.x;
}

void BlockAthleticsScene::Draw()
{
    // 背景描画
    DrawBackground();

    // ブロック描画
    DrawBlocks();

    // ゴールエリア描画
    DrawGoalArea();

    // プレイヤー描画
    DrawPlayer();

    // エフェクト描画
    DrawEffects();

    // ブロック破片描画
    DrawBlockFragments();

    // ゴール状況描画
    DrawGoalStatus();

    // UI描画
    DrawUI();
}

void BlockAthleticsScene::DrawGoalArea()
{
    if (!goalArea.isActive) {
        // 非アクティブ時は薄暗いゴールエリアを表示
        int screenX = (int)(goalArea.x - cameraX);
        int screenY = (int)goalArea.y;

        // 画面外カリング
        if (screenX < -goalArea.width || screenX > SCREEN_W) return;

        // 半透明の目標エリア
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawBox(screenX, screenY, screenX + (int)goalArea.width, screenY + (int)goalArea.height,
            GetColor(100, 100, 100), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // 「？」マーク
        DrawStringToHandle(screenX + (int)goalArea.width / 2 - 20, screenY + (int)goalArea.height / 2,
            "？", GetColor(150, 150, 150), largeFontHandle);
    }
    else {
        // アクティブ時は光るゴールエリア
        int screenX = (int)(goalArea.x - cameraX);
        int screenY = (int)goalArea.y;

        // 画面外カリング
        if (screenX < -goalArea.width || screenX > SCREEN_W) return;

        // 光るエフェクト（波打つ透明度）
        int alpha = (int)(150 + sinf(goalArea.animationPhase * 2) * 50);
        SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
        DrawBox(screenX, screenY, screenX + (int)goalArea.width, screenY + (int)goalArea.height,
            GetColor(255, 215, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // 光る枠線
        for (int i = 0; i < 3; i++) {
            int brightness = 200 + (int)(sinf(goalArea.animationPhase + i) * 55);
            DrawBox(screenX - i, screenY - i, screenX + (int)goalArea.width + i, screenY + (int)goalArea.height + i,
                GetColor(brightness, brightness, 0), FALSE);
        }

        // 「GOAL」テキスト
        string goalText = "GOAL";
        int textWidth = GetDrawStringWidthToHandle(goalText.c_str(), (int)goalText.length(), largeFontHandle);
        int textX = screenX + ((int)goalArea.width - textWidth) / 2;
        int textY = screenY + (int)goalArea.height / 2 - 30;

        // テキストの光る効果
        for (int i = 0; i < 3; i++) {
            DrawStringToHandle(textX - i, textY - i, goalText.c_str(), GetColor(255, 255, 255), largeFontHandle);
            DrawStringToHandle(textX + i, textY + i, goalText.c_str(), GetColor(255, 255, 255), largeFontHandle);
        }
        DrawStringToHandle(textX, textY, goalText.c_str(), GetColor(255, 215, 0), largeFontHandle);
    }
}

void BlockAthleticsScene::DrawGoalStatus()
{
    // 画面上部中央にゴール条件を大きく表示
    int centerX = SCREEN_W / 2;
    int topY = 50;

    // 背景パネル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(centerX - 300, topY - 20, centerX + 300, topY + 120, GetColor(20, 20, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 光る枠線
    DrawBox(centerX - 300, topY - 20, centerX + 300, topY + 120, GetColor(100, 200, 255), FALSE);

    // メインゴール表示
    string mainGoal = "【ゴール条件】";
    int titleWidth = GetDrawStringWidthToHandle(mainGoal.c_str(), (int)mainGoal.length(), largeFontHandle);
    DrawStringToHandle(centerX - titleWidth / 2, topY, mainGoal.c_str(), GetColor(255, 255, 255), largeFontHandle);

    // コイン収集状況（大きく目立つ表示）
    char coinStatus[128];
    sprintf_s(coinStatus, "コイン収集: %d/%d", coinsCollected, totalCoins);
    int coinWidth = GetDrawStringWidthToHandle(coinStatus, strlen(coinStatus), largeFontHandle);

    // コイン収集完了時は緑色、未完了時は黄色
    int coinColor = (coinsCollected >= totalCoins) ? GetColor(100, 255, 100) : GetColor(255, 255, 100);
    DrawStringToHandle(centerX - coinWidth / 2, topY + 40, coinStatus, coinColor, largeFontHandle);

    // 完了チェックマーク
    if (coinsCollected >= totalCoins) {
        DrawStringToHandle(centerX + coinWidth / 2 + 20, topY + 40, "✓", GetColor(100, 255, 100), largeFontHandle);
    }

    // ゴール到達指示
    string goalInstruction = "";
    if (coinsCollected >= totalCoins) {
        if (goalArea.isActive) {
            goalInstruction = "→ 光るゴールエリアに到達せよ！";
        }
        else {
            goalInstruction = "→ ステージ最後尾のゴールへ向かえ！";
        }
    }
    else {
        goalInstruction = "→ まずは全てのコインを集めよう！";
    }

    int instWidth = GetDrawStringWidthToHandle(goalInstruction.c_str(), (int)goalInstruction.length(), fontHandle);
    int instColor = (coinsCollected >= totalCoins) ? GetColor(100, 255, 255) : GetColor(255, 200, 100);
    DrawStringToHandle(centerX - instWidth / 2, topY + 80, goalInstruction.c_str(), instColor, fontHandle);

    // ゴールアクティブ化時の特別なヒント
    if (showGoalHint && goalHintTimer > 0) {
        // 点滅効果
        int blinkAlpha = (int)(255 * (0.5f + 0.5f * sinf(goalHintTimer * 10)));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, blinkAlpha);

        string hintText = "★ ゴールエリアが開放されました！ ★";
        int hintWidth = GetDrawStringWidthToHandle(hintText.c_str(), (int)hintText.length(), largeFontHandle);
        DrawStringToHandle(centerX - hintWidth / 2, topY + 130, hintText.c_str(), GetColor(255, 215, 0), largeFontHandle);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void BlockAthleticsScene::DrawBackground()
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

void BlockAthleticsScene::DrawBlocks()
{
    for (const auto& block : blocks) {
        if (!block.isActive) continue;

        int screenX = (int)(block.x - cameraX);
        int screenY = (int)block.y;

        // 画面外カリング
        if (screenX < -BLOCK_SIZE || screenX > SCREEN_W) continue;

        int textureHandle = -1;

        switch (block.type) {
        case BLOCK_DIRT:
            textureHandle = dirtHandle;
            break;
        case BLOCK_DIRT_TOP:
            textureHandle = dirtTopHandle;
            break;
        case BLOCK_STONE:
            textureHandle = stoneHandle;
            break;
        case BLOCK_STONE_TOP:
            textureHandle = stoneTopHandle;
            break;
        case BLOCK_CRATE:
            textureHandle = crateHandle;
            break;
        case BLOCK_EXPLOSIVE:
            textureHandle = explosiveCrateHandle;
            break;
        case BLOCK_COIN:
            textureHandle = coinHandle;
            // コインの回転エフェクト
            if (block.animationPhase > 0) {
                float scale = 1.0f + sinf(block.animationPhase) * 0.1f;
                int size = (int)(BLOCK_SIZE * scale);
                int offsetX = (BLOCK_SIZE - size) / 2;
                int offsetY = (BLOCK_SIZE - size) / 2;
                DrawExtendGraph(screenX + offsetX, screenY + offsetY,
                    screenX + offsetX + size, screenY + offsetY + size,
                    textureHandle, TRUE);
                continue;
            }
            break;
        default:
            continue;
        }

        if (textureHandle != -1) {
            DrawExtendGraph(screenX, screenY, screenX + BLOCK_SIZE, screenY + BLOCK_SIZE,
                textureHandle, TRUE);
        }
    }
}

void BlockAthleticsScene::DrawPlayer()
{
    int screenX = (int)(player.x - cameraX);
    int screenY = (int)player.y;

    if (player.graphicHandle != -1) {
        // プレイヤーは左右対称なので常に通常描画
        DrawExtendGraph(screenX, screenY, screenX + BLOCK_SIZE, screenY + BLOCK_SIZE,
            player.graphicHandle, TRUE);
    }
}

void BlockAthleticsScene::DrawEffects()
{
    DrawParticles();
}

void BlockAthleticsScene::DrawParticles()
{
    for (const auto& particle : particles) {
        if (particle.life > 0) {
            int alpha = (int)(255 * (particle.life / particle.maxLife));
            int size = (int)(8 * particle.scale);

            SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);
            DrawCircle((int)(particle.x - cameraX), (int)particle.y, size, particle.color, TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }
}

void BlockAthleticsScene::DrawUI()
{
    // プロフェッショナルなHUDシステム描画
    hudSystem.Draw();

    // スタイリッシュなゲーム情報パネル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
    DrawBox(30, 200, 500, 400, GetColor(10, 10, 30), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // パネルの光る枠線
    for (int i = 0; i < 3; i++) {
        DrawBox(30 - i, 200 - i, 500 + i, 400 + i, GetColor(100 + i * 30, 200 + i * 20, 255), FALSE);
    }

    // タイトル（グラデーション風）
    string title = "BLOCK ATHLETICS CHAMPIONSHIP";
    DrawStringToHandle(40, 210, title.c_str(), GetColor(255, 215, 0), largeFontHandle);

    string subtitle = "Featuring: " + characterName;
    DrawStringToHandle(45, 255, subtitle.c_str(), GetColor(200, 255, 200), fontHandle);

    // 進行状況（プログレスバー付き）
    char progressText[256];

    // クレート破壊
    sprintf_s(progressText, "OBSTACLES CLEARED: %d/%d", cratesDestroyed, totalCrates);
    DrawStringToHandle(40, 290, progressText, GetColor(255, 255, 255), fontHandle);

    // クレートプログレスバー
    float crateProgress = totalCrates > 0 ? (float)cratesDestroyed / totalCrates : 0.0f;
    DrawProgressBar(40, 315, 200, 15, crateProgress, GetColor(255, 100, 100), GetColor(100, 50, 50));

    // タイム表示（大きく目立つ）
    sprintf_s(progressText, "TIME: %.2fs", gameTimer);
    DrawStringToHandle(300, 290, progressText, GetColor(100, 255, 255), largeFontHandle);

    // 完了率表示
    float totalProgress = ((float)coinsCollected / totalCoins + (float)cratesDestroyed / totalCrates) / 2.0f;
    sprintf_s(progressText, "COMPLETION: %.1f%%", totalProgress * 100);
    DrawStringToHandle(300, 340, progressText, GetColor(255, 200, 255), fontHandle);

    // プロフェッショナルな操作説明（日本語）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(30, SCREEN_H - 200, 800, SCREEN_H - 30, GetColor(20, 20, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 操作説明の枠線
    DrawBox(30, SCREEN_H - 200, 800, SCREEN_H - 30, GetColor(100, 150, 255), FALSE);

    DrawStringToHandle(40, SCREEN_H - 190, "【操作方法】", GetColor(255, 255, 255), largeFontHandle);
    DrawStringToHandle(40, SCREEN_H - 150, "◄► 矢印キー: コースを移動", GetColor(200, 255, 200), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 120, "SPACE: パワージャンプ", GetColor(255, 255, 100), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 90, "↓ 下キー: 急降下", GetColor(100, 200, 255), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 60, "目標: 全てのコインを集めてゴールに到達！", GetColor(255, 200, 100), fontHandle);

#ifdef _DEBUG
    // デバッグ時のみのテスト用操作説明
    DrawStringToHandle(40, SCREEN_H - 30, "テスト: 1/2キーでライフ減少/増加", GetColor(150, 150, 150), fontHandle);
#endif

    // 右下にESC情報
    DrawStringToHandle(SCREEN_W - 300, SCREEN_H - 50, "ESC: メニューに戻る", GetColor(150, 150, 150), fontHandle);

    // ゲーム完了時の豪華な表示
    if (currentState == STATE_COMPLETED) {
        // 半透明オーバーレイ
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 240);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 10, 30), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // 大きな完了パネル
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 250);
        DrawBox(SCREEN_W / 2 - 400, SCREEN_H / 2 - 200, SCREEN_W / 2 + 400, SCREEN_H / 2 + 200, GetColor(10, 30, 60), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // 光る枠線
        for (int i = 0; i < 5; i++) {
            int brightness = 100 + i * 30;
            DrawBox(SCREEN_W / 2 - 400 - i, SCREEN_H / 2 - 200 - i, SCREEN_W / 2 + 400 + i, SCREEN_H / 2 + 200 + i,
                GetColor(brightness, brightness + 50, 255), FALSE);
        }

        // 完了メッセージ
        string completedText = "チャンピオンシップ クリア！";
        int textWidth = GetDrawStringWidthToHandle(completedText.c_str(), (int)completedText.length(), largeFontHandle);
        DrawStringToHandle(SCREEN_W / 2 - textWidth / 2, SCREEN_H / 2 - 120, completedText.c_str(), GetColor(255, 215, 0), largeFontHandle);

        // 詳細結果
        char resultText[256];
        sprintf_s(resultText, "クリアタイム: %.2f秒", gameTimer);
        int timeWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
        DrawStringToHandle(SCREEN_W / 2 - timeWidth / 2, SCREEN_H / 2 - 60, resultText, GetColor(100, 255, 255), fontHandle);

        sprintf_s(resultText, "コイン収集: %d/%d (%.1f%%)", coinsCollected, totalCoins,
            (float)coinsCollected / totalCoins * 100);
        int coinWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
        DrawStringToHandle(SCREEN_W / 2 - coinWidth / 2, SCREEN_H / 2 - 20, resultText, GetColor(255, 215, 0), fontHandle);

        sprintf_s(resultText, "障害物クリア: %d/%d", cratesDestroyed, totalCrates);
        int crateWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
        DrawStringToHandle(SCREEN_W / 2 - crateWidth / 2, SCREEN_H / 2 + 20, resultText, GetColor(255, 100, 100), fontHandle);

        // 評価
        string gradeText = "";
        if (coinsCollected == totalCoins && gameTimer < 60.0f) {
            gradeText = "ランク: S+ (パーフェクト！)";
        }
        else if (coinsCollected == totalCoins) {
            gradeText = "ランク: S (素晴らしい！)";
        }
        else if (coinsCollected >= totalCoins * 0.8f) {
            gradeText = "ランク: A (良い！)";
        }
        else {
            gradeText = "ランク: B (がんばった！)";
        }

        int gradeWidth = GetDrawStringWidthToHandle(gradeText.c_str(), (int)gradeText.length(), largeFontHandle);
        DrawStringToHandle(SCREEN_W / 2 - gradeWidth / 2, SCREEN_H / 2 + 80, gradeText.c_str(), GetColor(255, 255, 100), largeFontHandle);

        // 終了指示
        string exitText = "ESCキーでキャラクター選択に戻る";
        int exitWidth = GetDrawStringWidthToHandle(exitText.c_str(), (int)exitText.length(), fontHandle);
        DrawStringToHandle(SCREEN_W / 2 - exitWidth / 2, SCREEN_H / 2 + 140, exitText.c_str(), GetColor(200, 200, 200), fontHandle);
    }
}

// プログレスバー描画関数
void BlockAthleticsScene::DrawProgressBar(int x, int y, int width, int height, float progress, int fillColor, int bgColor)
{
    // 背景
    DrawBox(x, y, x + width, y + height, bgColor, TRUE);

    // 進行度
    int fillWidth = (int)(width * progress);
    if (fillWidth > 0) {
        DrawBox(x, y, x + fillWidth, y + height, fillColor, TRUE);
    }

    // 枠線
    DrawBox(x, y, x + width, y + height, GetColor(255, 255, 255), FALSE);

    // 光沢効果
    if (progress > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 80);
        DrawBox(x + 2, y + 2, x + fillWidth - 2, y + height / 2, GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

bool BlockAthleticsScene::IsBlockCollision(float playerX, float playerY, float playerW, float playerH)
{
    for (const auto& block : blocks) {
        if (!block.isActive) continue;

        // コインは通り抜け可能（クレートは足場として機能する）
        if (block.type == BLOCK_COIN) continue;

        float blockLeft = block.x;
        float blockRight = block.x + BLOCK_SIZE;
        float blockTop = block.y;
        float blockBottom = block.y + BLOCK_SIZE;

        // プレイヤーの境界
        float playerLeft = playerX;
        float playerRight = playerX + playerW;
        float playerTop = playerY;
        float playerBottom = playerY + playerH;

        // 衝突判定（少し余裕を持たせて正確に）
        if (playerRight > blockLeft + 2 && playerLeft < blockRight - 2 &&
            playerBottom > blockTop + 2 && playerTop < blockBottom - 2) {
            return true;
        }
    }
    return false;
}

BlockAthleticsScene::Block* BlockAthleticsScene::GetBlockAt(float x, float y)
{
    for (auto& block : blocks) {
        if (!block.isActive) continue;

        if (x >= block.x && x < block.x + BLOCK_SIZE &&
            y >= block.y && y < block.y + BLOCK_SIZE) {
            return &block;
        }
    }
    return nullptr;
}

string BlockAthleticsScene::GetCharacterDisplayName(int index)
{
    switch (index) {
    case 0: return "Beige Builder";
    case 1: return "Green Constructor";
    case 2: return "Pink Architect";
    case 3: return "Purple Engineer";
    case 4: return "Yellow Athlete";
    default: return "Block Runner";
    }
}

float BlockAthleticsScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

void BlockAthleticsScene::ResetGame()
{
    // ゲーム状態をリセット
    currentState = STATE_PLAYING;
    gameCompleted = false;
    exitRequested = false;
    gameTimer = 0.0f;
    coinsCollected = 0;
    cratesDestroyed = 0;

    // プレイヤーをリセット
    InitializePlayer();

    // ブロックをリセット
    InitializeStage();

    // カメラをリセット
    cameraX = 0.0f;
    previousPlayerX = player.x;

    // エフェクトをクリア
    particles.clear();

    OutputDebugStringA("BlockAthleticsScene: Game reset\n");
}

void BlockAthleticsScene::DestroyBlock(int blockIndex)
{
    Block& block = blocks[blockIndex];

    // ブロックを非アクティブに
    block.isActive = false;

    // 破壊可能なブロックの場合はカウント
    if (block.isBreakable) {
        cratesDestroyed++;
    }

    // 4分割エフェクト生成
    CreateBlockFragments(block.x, block.y, block.type);

    // 破壊音
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HIT);

    // パーティクルエフェクト
    int particleColor = GetColor(139, 69, 19); // デフォルト色
    switch (block.type) {
    case BLOCK_DIRT:
    case BLOCK_DIRT_TOP:
        particleColor = GetColor(139, 69, 19);
        break;
    case BLOCK_STONE:
    case BLOCK_STONE_TOP:
        particleColor = GetColor(128, 128, 128);
        break;
    case BLOCK_CRATE:
        particleColor = GetColor(160, 82, 45);
        break;
    case BLOCK_EXPLOSIVE:
        particleColor = GetColor(255, 100, 0);
        CreateExplosionEffect(block.x + BLOCK_SIZE / 2, block.y + BLOCK_SIZE / 2);
        break;
    }

    CreateParticles(block.x + BLOCK_SIZE / 2, block.y + BLOCK_SIZE / 2, particleColor, 15);

    Particle p;
        p.velocityY = (rand() % 150 - 200) * 0.1f;
    p.life = p.maxLife = 1.5f + (rand() % 150) * 0.01f;
    p.color = GetColor(255, 255,255);
    p.scale = 0.8f + (rand() % 150) * 0.005f;
    particles.push_back(p);
}


// 新しいエフェクト関数群
void BlockAthleticsScene::CreateSparkleEffect(float x, float y)
{
    for (int i = 0; i < 8; i++) {
        Particle p;
        float angle = (float)i * (6.28f / 8);
        p.x = x + cosf(angle) * 20;
        p.y = y + sinf(angle) * 20;
        p.velocityX = cosf(angle) * 5.0f;
        p.velocityY = sinf(angle) * 5.0f - 2.0f;
        p.life = p.maxLife = 2.0f;
        p.color = GetColor(255, 255, 255);
        p.scale = 1.2f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateExplosionEffect(float x, float y)
{
    for (int i = 0; i < 30; i++) {
        Particle p;
        float angle = (float)(rand() % 360) * 0.0174f;
        float speed = 5.0f + (rand() % 100) * 0.1f;
        p.x = x;
        p.y = y;
        p.velocityX = cosf(angle) * speed;
        p.velocityY = sinf(angle) * speed - 3.0f;
        p.life = p.maxLife = 2.5f;

        // ランダムな爆発色
        switch (rand() % 3) {
        case 0: p.color = GetColor(255, 100, 0); break;
        case 1: p.color = GetColor(255, 255, 0); break;
        case 2: p.color = GetColor(255, 0, 0); break;
        }
        p.scale = 1.5f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateDebrisEffect(float x, float y)
{
    for (int i = 0; i < 12; i++) {
        Particle p;
        p.x = x + (rand() % 40 - 20);
        p.y = y + (rand() % 40 - 20);
        p.velocityX = (rand() % 160 - 80) * 0.1f;
        p.velocityY = (rand() % 80 - 120) * 0.1f;
        p.life = p.maxLife = 3.0f;
        p.color = GetColor(101, 67, 33);
        p.scale = 1.0f + (rand() % 50) * 0.01f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateMovementParticles()
{
    // 移動時の控えめな足元パーティクル（頻度制限）
    static int movementCounter = 0;
    movementCounter++;
    if (movementCounter % 8 == 0 && player.onGround) { // 頻度をさらに下げる
        Particle p;
        p.x = player.x + BLOCK_SIZE / 2 + (rand() % 30 - 15); // 範囲を小さく
        p.y = player.y + BLOCK_SIZE - 5;
        p.velocityX = (rand() % 40 - 20) * 0.03f; // 速度を小さく
        p.velocityY = (rand() % 20 - 30) * 0.03f; // 上向きの速度を小さく
        p.life = p.maxLife = 0.5f; // 短い寿命
        p.color = GetColor(180, 180, 180);
        p.scale = 0.4f; // 小さいサイズ
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateJumpParticles()
{
    for (int i = 0; i < 10; i++) {
        Particle p;
        p.x = player.x + BLOCK_SIZE / 2 + (rand() % 60 - 30);
        p.y = player.y + BLOCK_SIZE;
        p.velocityX = (rand() % 120 - 60) * 0.1f;
        p.velocityY = (rand() % 40) * 0.1f;
        p.life = p.maxLife = 1.2f;
        p.color = GetColor(150, 255, 150);
        p.scale = 0.8f;
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateLandingParticles()
{
    for (int i = 0; i < 5; i++) { // パーティクル数を減らす
        Particle p;
        p.x = player.x + BLOCK_SIZE / 2 + (rand() % 50 - 25); // 範囲を小さく
        p.y = player.y + BLOCK_SIZE;
        p.velocityX = (rand() % 80 - 40) * 0.08f; // 速度を控えめに
        p.velocityY = (rand() % 40 - 60) * 0.08f; // 上向きの速度を控えめに
        p.life = p.maxLife = 0.8f; // 短い寿命
        p.color = GetColor(150, 150, 150);
        p.scale = 0.5f; // 小さいサイズ
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateHardLandingParticles()
{
    for (int i = 0; i < 8; i++) { // パーティクル数を減らす
        Particle p;
        p.x = player.x + BLOCK_SIZE / 2 + (rand() % 80 - 40); // 範囲を小さく
        p.y = player.y + BLOCK_SIZE;
        p.velocityX = (rand() % 120 - 60) * 0.08f; // 速度を控えめに
        p.velocityY = (rand() % 60 - 80) * 0.08f; // 上向きの速度を控えめに
        p.life = p.maxLife = 1.0f; // 短い寿命
        p.color = GetColor(180, 130, 80);
        p.scale = 0.7f; // 小さいサイズ
        particles.push_back(p);
    }
}

void BlockAthleticsScene::CreateFastFallParticles()

{

    static int fallCounter = 0;

    fallCounter++;

    if (fallCounter % 3 == 0) { // 頻度制限

        Particle p;

        p.x = player.x + BLOCK_SIZE / 2 + (rand() % 40 - 20);

        p.y = player.y + BLOCK_SIZE / 2;

        p.velocityX = (rand() % 40 - 20) * 0.05f;

        p.velocityY = player.velocityY * 0.3f;

        p.life = p.maxLife = 0.8f;

        p.color = GetColor(100, 150, 255);

        p.scale = 0.6f;

        particles.push_back(p);

    }

}