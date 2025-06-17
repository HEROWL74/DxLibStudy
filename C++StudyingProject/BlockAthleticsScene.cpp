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
    ,clearStateTimer(0.0f)
    ,clearAnimTimer(0.0f)
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

    // **破片テクスチャの解放を追加**
    for (int blockType = 0; blockType < 7; blockType++) {
        for (int fragmentIndex = 0; fragmentIndex < 4; fragmentIndex++) {
            if (fragmentHandles[blockType][fragmentIndex] != -1) {
                DeleteGraph(fragmentHandles[blockType][fragmentIndex]);
                fragmentHandles[blockType][fragmentIndex] = -1;
            }
        }
    }
}

void BlockAthleticsScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    characterName = GetCharacterDisplayName(selectedCharacter);

    // テクスチャ読み込み
    LoadTextures();

    // **重要: 破片テクスチャの読み込みを追加**
    LoadFragmentTextures();

    // 破片ハンドル配列を初期化
    for (int i = 0; i < 7; i++) {
        for (int j = 0; j < 4; j++) {
            fragmentHandles[i][j] = -1;
        }
    }

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
    hudSystem.SetCurrentLife(6);
    hudSystem.SetPosition(30, 30);
    hudSystem.SetVisible(true);

    // ゲーム状態リセット
    currentState = STATE_PLAYING;
    gameCompleted = false;
    exitRequested = false;
    gameTimer = 0.0f;
    coinsCollected = 0;
    cratesDestroyed = 0;
    playerLife = 6;
    ResetClearTimers();

    // カメラ初期化
    cameraX = 0.0f;
    previousPlayerX = player.x;

    // エフェクト初期化
    particles.clear();
    blockFragments.clear(); // 破片配列もクリア

    // BGM開始
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_GAME);

    OutputDebugStringA("BlockAthleticsScene: Initialized successfully with fragment system\n");
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

        // DerivationGraphを使用して破片テクスチャを作成
        fragmentHandles[blockType][i] = DerivationGraph(
            srcX, srcY, fragmentWidth, fragmentHeight, originalHandle
        );

        if (fragmentHandles[blockType][i] != -1) {
            char debugMsg[128];
            sprintf_s(debugMsg, "BlockAthleticsScene: Created fragment %d for block type %d (handle: %d)\n",
                i, blockType, fragmentHandles[blockType][i]);
            OutputDebugStringA(debugMsg);
        }
        else {
            char errorMsg[128];
            sprintf_s(errorMsg, "BlockAthleticsScene: Failed to create fragment %d for block type %d\n",
                i, blockType);
            OutputDebugStringA(errorMsg);
        }
    }
}
void BlockAthleticsScene::InitializeStage()
{
    blocks.clear();
    totalCoins = 0;
    totalCrates = 0;

    // ===== セクション1: 広々スタートエリア =====
    // 超安全なスタート地点（広い平地）
    for (int x = 0; x < 12; x++) {
        Block dirtBlock = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(dirtBlock);
        Block dirtTopBlock = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(dirtTopBlock);
    }

    // スタートエリアの地上コイン（安全に歩いて取得）
    for (int i = 0; i < 4; i++) {
        Block coin = { BLOCK_COIN, BLOCK_SIZE * (2 + i * 2), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(coin);
        totalCoins++;
    }

    // ===== ギミック1: 近距離コイン階段（ジャンプ可能距離） =====
    // 各段の距離を1.5ブロック間隔に縮める
    for (int i = 0; i < 4; i++) {
        int baseX = 14 + i * 2; // 4ブロック間隔から2ブロック間隔に短縮

        // 幅2ブロックの安全な足場
        for (int w = 0; w < 2; w++) {
            Block stairBase = { BLOCK_STONE, (float)((baseX + w) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (2 + i), true, false, 0.0f, false };
            blocks.push_back(stairBase);
            Block stairTop = { BLOCK_STONE_TOP, (float)((baseX + w) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (3 + i), true, false, 0.0f, false };
            blocks.push_back(stairTop);
        }

        // 中央にコイン配置
        Block stairCoin = { BLOCK_COIN, (float)(baseX * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (4 + i), true, true, 0.0f, false };
        blocks.push_back(stairCoin);
        totalCoins++;
    }

    // ===== セクション2: 接続平地エリア =====
    // 階段から連続する平地
    for (int x = 22; x < 30; x++) {
        Block dirtTop = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(dirtTop);
        Block dirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(dirt);
    }

    // 平地に散らばったコイン
    for (int i = 0; i < 3; i++) {
        Block groundCoin = { BLOCK_COIN, BLOCK_SIZE * (24 + i * 2), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(groundCoin);
        totalCoins++;
    }

    // ===== ギミック2: 近接宝箱エリア =====
    // 地上からすぐにアクセス可能な宝箱
    for (int x = 31; x < 35; x++) {
        for (int y = 1; y <= 2; y++) {
            if (x == 31 || x == 34 || y == 1) { // L字型の壁
                Block treasureWall = { BLOCK_CRATE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (1 + y), true, true, 0.0f, true };
                blocks.push_back(treasureWall);
                totalCrates++;
            }
        }
    }

    // 宝箱へのなだらかなアクセス
    Block accessStep = { BLOCK_DIRT_TOP, (float)(30 * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
    blocks.push_back(accessStep);

    // 宝箱内のコイン
    for (int i = 0; i < 2; i++) {
        Block treasureCoin = { BLOCK_COIN, (float)((32 + i) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(treasureCoin);
        totalCoins++;
    }

    // ===== ギミック3: 密集ジグザグ道（1ブロック間隔） =====
    // ジャンプで確実に届く1ブロック間隔
    for (int i = 0; i < 6; i++) {
        int x = 37 + i; // 1ブロック間隔
        int y = 2 + (i % 2); // 高さ差も1ブロックのみ

        // 幅2ブロックの安全な足場
        for (int w = 0; w < 2; w++) {
            Block zigzagPlatform = { BLOCK_STONE_TOP, (float)((x + w) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * y, true, false, 0.0f, false };
            blocks.push_back(zigzagPlatform);
        }

        // 中央にコイン
        Block zigzagCoin = { BLOCK_COIN, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (y + 1), true, true, 0.0f, false };
        blocks.push_back(zigzagCoin);
        totalCoins++;
    }

    // ===== セクション3: 休憩平地 =====
    for (int x = 45; x < 55; x++) {
        Block restArea = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(restArea);
        Block restDirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(restDirt);
    }

    // 休憩エリアのボーナスコイン
    for (int i = 0; i < 4; i++) {
        Block restCoin = { BLOCK_COIN, BLOCK_SIZE * (47 + i * 2), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(restCoin);
        totalCoins++;
    }

    // ===== ギミック4: コンパクト迷路（1ブロック間隔） =====
    // 迷路のサイズを縮小し、通路を広く
    for (int x = 57; x < 65; x++) {
        for (int y = 2; y < 4; y++) {
            // 1ブロックおきに壁を配置（十分な通路確保）
            bool isWall = ((x - 57) % 2 == 0 && y == 3) ||
                ((x - 57) % 2 == 1 && y == 2);

            if (isWall) {
                Block mazeWall = { BLOCK_CRATE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * y, true, true, 0.0f, true };
                blocks.push_back(mazeWall);
                totalCrates++;
            }
        }
    }

    // 迷路上の簡単アクセスルート
    for (int x = 57; x < 65; x++) {
        Block safeRoute = { BLOCK_STONE_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 5, true, false, 0.0f, false };
        blocks.push_back(safeRoute);
    }

    // 迷路内のコイン
    Block mazeCoin1 = { BLOCK_COIN, BLOCK_SIZE * 59, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(mazeCoin1);
    totalCoins++;
    Block mazeCoin2 = { BLOCK_COIN, BLOCK_SIZE * 63, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(mazeCoin2);
    totalCoins++;
    Block routeCoin = { BLOCK_COIN, BLOCK_SIZE * 61, STAGE_HEIGHT - BLOCK_SIZE * 6, true, true, 0.0f, false };
    blocks.push_back(routeCoin);
    totalCoins++;

    // ===== ギミック5: 低いコインタワー（密集階段） =====
    // 中央の塔（低め、3段）
    for (int y = 1; y <= 3; y++) {
        Block towerStone = { BLOCK_STONE, BLOCK_SIZE * 70, STAGE_HEIGHT - BLOCK_SIZE * y, true, false, 0.0f, false };
        blocks.push_back(towerStone);
    }
    Block towerTop = { BLOCK_STONE_TOP, BLOCK_SIZE * 70, STAGE_HEIGHT - BLOCK_SIZE * 4, true, false, 0.0f, false };
    blocks.push_back(towerTop);

    // 簡単な階段（隣接配置）
    for (int i = 0; i < 3; i++) {
        Block step = { BLOCK_DIRT_TOP, (float)((68 + i) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (2 + i), true, false, 0.0f, false };
        blocks.push_back(step);

        // 各段にコイン
        Block stepCoin = { BLOCK_COIN, (float)((68 + i) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * (3 + i), true, true, 0.0f, false };
        blocks.push_back(stepCoin);
        totalCoins++;
    }

    // 塔の頂上コイン
    Block towerBonusCoin = { BLOCK_COIN, BLOCK_SIZE * 70, STAGE_HEIGHT - BLOCK_SIZE * 5, true, true, 0.0f, false };
    blocks.push_back(towerBonusCoin);
    totalCoins++;

    // ===== ギミック6: 隣接ブリッジ =====
    // 連続する安全な橋
    for (int x = 73; x < 78; x++) {
        // 幅2ブロックの安全な橋
        for (int w = 0; w < 2; w++) {
            Block bridge = { BLOCK_CRATE, (float)((x + w * 0.5f) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, true };
            blocks.push_back(bridge);
            totalCrates++;
        }

        // 下に安全ネット
        for (int w = 0; w < 3; w++) {
            Block safetyNet = { BLOCK_DIRT_TOP, (float)((x + w * 0.33f) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
            blocks.push_back(safetyNet);
        }
    }

    // 橋の中央にコイン
    Block bridgeCoin = { BLOCK_COIN, BLOCK_SIZE * 75, STAGE_HEIGHT - BLOCK_SIZE * 4, true, true, 0.0f, false };
    blocks.push_back(bridgeCoin);
    totalCoins++;

    // ===== ギミック7: 小さなピラミッド（隣接設計） =====
    // 3段の小さなピラミッド
    for (int level = 0; level < 3; level++) {
        for (int x = 0; x <= (2 - level); x++) {
            Block pyramidStone = { BLOCK_STONE, (float)((82 + x) * BLOCK_SIZE),
                                 STAGE_HEIGHT - BLOCK_SIZE * (2 + level), true, false, 0.0f, false };
            blocks.push_back(pyramidStone);

            // コイン（各段）
            Block pyramidCoin = { BLOCK_COIN, (float)((82 + x) * BLOCK_SIZE),
                                STAGE_HEIGHT - BLOCK_SIZE * (3 + level), true, true, 0.0f, false };
            blocks.push_back(pyramidCoin);
            totalCoins++;
        }
    }

    // ===== セクション4: 最終平地エリア =====
    for (int x = 87; x < 95; x++) {
        Block finalGround = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(finalGround);
        Block finalDirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(finalDirt);
    }

    // 最終エリアのボーナスコイン
    for (int i = 0; i < 3; i++) {
        Block finalCoin = { BLOCK_COIN, BLOCK_SIZE * (89 + i * 2), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(finalCoin);
        totalCoins++;
    }

    // ===== ギミック8: 簡単アクセス隠し部屋 =====
    // 地上からすぐアクセス可能
    for (int x = 97; x < 101; x++) {
        for (int y = 2; y < 3; y++) {
            if (x == 97 || x == 100) {
                if (!(x == 98)) { // 広い入口
                    Block secretWall = { BLOCK_CRATE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * y, true, true, 0.0f, true };
                    blocks.push_back(secretWall);
                    totalCrates++;
                }
            }
        }
    }

    // 隠し部屋の中に豪華なコイン
    for (int i = 0; i < 3; i++) {
        Block secretCoin = { BLOCK_COIN, (float)((98 + i) * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
        blocks.push_back(secretCoin);
        totalCoins++;
    }

    // ===== ゴール前の連続エリア =====
    for (int x = 103; x < 110; x++) {
        Block preGoal = { BLOCK_DIRT_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 2, true, false, 0.0f, false };
        blocks.push_back(preGoal);
        Block preGoalDirt = { BLOCK_DIRT, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE, true, false, 0.0f, false };
        blocks.push_back(preGoalDirt);
    }

    // 最後のお祝いコイン
    Block celebrationCoin = { BLOCK_COIN, BLOCK_SIZE * 106, STAGE_HEIGHT - BLOCK_SIZE * 3, true, true, 0.0f, false };
    blocks.push_back(celebrationCoin);
    totalCoins++;

    // ===== ゴールエリアの台座（隣接配置） =====
    for (int x = 112; x < 118; x++) {
        for (int y = 1; y <= 2; y++) {
            Block goalStone = { BLOCK_STONE, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * y, true, false, 0.0f, false };
            blocks.push_back(goalStone);
        }
        Block goalTop = { BLOCK_STONE_TOP, (float)(x * BLOCK_SIZE), STAGE_HEIGHT - BLOCK_SIZE * 3, true, false, 0.0f, false };
        blocks.push_back(goalTop);
    }

    // ステージサイズ設定（大幅短縮）
    STAGE_WIDTH = BLOCK_SIZE * 120;

    OutputDebugStringA("BlockAthleticsScene: Accessible jump-friendly course initialized\n");

    char debugMsg[256];
    sprintf_s(debugMsg, "BlockAthleticsScene: Accessible course stats - Coins: %d, Crates: %d, Length: %d blocks\n",
        totalCoins, totalCrates, STAGE_WIDTH / BLOCK_SIZE);
    OutputDebugStringA(debugMsg);
}

void BlockAthleticsScene::InitializeGoalArea()
{
    // ゴールエリアの設定（アクセス可能な設計に調整）
    goalArea.x = STAGE_WIDTH - BLOCK_SIZE * 8;   // より近いゴールエリア
    goalArea.y = STAGE_HEIGHT - BLOCK_SIZE * 4;  // 低めの高さ
    goalArea.width = BLOCK_SIZE * 8;             // 幅8ブロック分
    goalArea.height = BLOCK_SIZE * 4;            // 高さは4ブロック分
    goalArea.isActive = false;  // 初期状態では非アクティブ
    goalArea.animationPhase = 0.0f;
    goalArea.goalTextureHandle = -1;
    goalArea.flagTextureHandle = -1;

    // ゴールの表示状態
    showGoalHint = false;
    goalHintTimer = 0.0f;

    OutputDebugStringA("BlockAthleticsScene: Jump-accessible goal area initialized\n");
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
    player.jumpPower = -20.0f;  // ふわっとしたジャンプ力に調整
    player.moveSpeed = 6.0f;   // 移動速度はそのまま
}

void BlockAthleticsScene::Update()
{
    if (currentState == STATE_PLAYING) {
        gameTimer += 0.016f;
    }

    UpdateInput();

    if (currentState == STATE_PLAYING) {
        UpdatePlayer();
        UpdateBlocks();
        UpdateCollisions();
        UpdateBlockDestruction();
        UpdateGoalArea();
    }
    else if (currentState == STATE_COMPLETED) {
        // **メンバ変数タイマーを使用**
        clearStateTimer += 0.016f;
        clearAnimTimer += 0.016f;

        // 15秒後に自動復帰
        if (clearStateTimer >= 15.0f) {
            exitRequested = true;
            ResetClearTimers();
            OutputDebugStringA("BlockAthleticsScene: Auto-return after 15 seconds\n");
        }

        // ESCで即座に復帰
        if (escPressed && !escPressedPrev) {
            exitRequested = true;
            ResetClearTimers();
            OutputDebugStringA("BlockAthleticsScene: Manual return with ESC\n");
        }

        // エフェクトは継続
        UpdateEffects();
        UpdateBlockFragments();
    }

    UpdateCamera();
    UpdateEffects();
    UpdateBlockFragments();

    hudSystem.SetCurrentLife(playerLife);

    // プレイ中のESC処理
    if (escPressed && !escPressedPrev && currentState == STATE_PLAYING) {
        exitRequested = true;
        OutputDebugStringA("BlockAthleticsScene: ESC pressed during play - exiting\n");
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
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

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
                // **修正: クリア処理を改善**
                CompleteGame();
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
        if (fragment.life <= 0) continue;

        int alpha = (int)(255 * (fragment.life / fragment.maxLife));
        int size = (int)(BLOCK_SIZE / 2 * fragment.scale); // 1/2サイズの破片

        int screenX = (int)(fragment.x - cameraX);
        int screenY = (int)fragment.y;

        // 画面外カリング
        if (screenX < -size || screenX > SCREEN_W + size) continue;

        // 透明度設定
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // テクスチャがある場合は回転描画
        if (fragment.graphicHandle != -1) {
            float centerX = screenX + size / 2;
            float centerY = screenY + size / 2;

            // 回転描画
            DrawRotaGraph3((int)centerX, (int)centerY,
                size / 2, size / 2,  // 中心座標（相対）
                fragment.scale, fragment.scale,  // X,Y拡大率
                fragment.rotation,  // 回転角
                fragment.graphicHandle, TRUE);
        }
        else {
            // テクスチャがない場合は色付きの矩形で代用
            DrawBox(screenX, screenY, screenX + size, screenY + size,
                fragment.color, TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
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

        // **改善: より寛容な当たり判定**
        float blockCenterX = blocks[i].x + BLOCK_SIZE / 2;
        float blockCenterY = blocks[i].y + BLOCK_SIZE / 2;
        float playerCenterX = player.x + BLOCK_SIZE / 2;
        float playerCenterY = player.y + BLOCK_SIZE / 2;

        // 距離による円形の当たり判定（より取りやすく）
        float distance = sqrtf((playerCenterX - blockCenterX) * (playerCenterX - blockCenterX) +
            (playerCenterY - blockCenterY) * (playerCenterY - blockCenterY));

        // **当たり判定範囲を拡大（ブロックサイズの80%）**
        float collisionRange = BLOCK_SIZE * 0.8f;

        if (distance <= collisionRange) {
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
    // 破片テクスチャが正しく読み込まれているかチェック
    bool hasValidFragments = false;
    for (int i = 0; i < 4; i++) {
        if (fragmentHandles[blockType][i] != -1) {
            hasValidFragments = true;
            break;
        }
    }

    if (!hasValidFragments) {
        // 破片テクスチャがない場合は通常のパーティクルエフェクトで代用
        int particleColor = GetColor(139, 69, 19);
        switch (blockType) {
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
            break;
        case BLOCK_COIN:
            particleColor = GetColor(255, 215, 0);
            break;
        }
        CreateParticles(x + BLOCK_SIZE / 2, y + BLOCK_SIZE / 2, particleColor, 20);
        return;
    }

    // 4つの破片を生成（2x2の格子状に分割）
    for (int i = 0; i < 4; i++) {
        if (fragmentHandles[blockType][i] == -1) continue;

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
        float baseSpeed = 5.0f + (rand() % 300) * 0.01f;
        float randomAngle = ((rand() % 120) - 60) * 0.0174f; // ±60度のランダム

        float cosA = cosf(randomAngle);
        float sinA = sinf(randomAngle);

        fragment.velocityX = (directionX * cosA - directionY * sinA) * baseSpeed;
        fragment.velocityY = (directionX * sinA + directionY * cosA) * baseSpeed - 4.0f; // 上向き成分

        // 回転設定
        fragment.rotation = (float)(rand() % 360) * 0.0174f;
        fragment.rotationSpeed = ((rand() % 600) - 300) * 0.01f;

        // ライフ設定
        fragment.life = fragment.maxLife = 2.5f + (rand() % 200) * 0.01f;

        // 見た目設定
        fragment.scale = 0.8f + (rand() % 40) * 0.01f;
        fragment.fragmentType = i;
        fragment.originalBlockType = blockType;
        fragment.graphicHandle = fragmentHandles[blockType][i];

        // 色情報（テクスチャがない場合のフォールバック）
        switch (blockType) {
        case BLOCK_DIRT:
        case BLOCK_DIRT_TOP:
            fragment.color = GetColor(139, 69, 19);
            break;
        case BLOCK_STONE:
        case BLOCK_STONE_TOP:
            fragment.color = GetColor(128, 128, 128);
            break;
        case BLOCK_CRATE:
            fragment.color = GetColor(160, 82, 45);
            break;
        case BLOCK_EXPLOSIVE:
            fragment.color = GetColor(255, 100, 0);
            break;
        case BLOCK_COIN:
            fragment.color = GetColor(255, 215, 0);
            break;
        default:
            fragment.color = GetColor(200, 200, 200);
            break;
        }

        blockFragments.push_back(fragment);
    }

    // デバッグログ
    char debugMsg[128];
    sprintf_s(debugMsg, "BlockAthleticsScene: Created %d fragments for block type %d\n",
        4, blockType);
    OutputDebugStringA(debugMsg);
}

void BlockAthleticsScene::CollectCoin(int blockIndex)
{
    blocks[blockIndex].isActive = false;
    coinsCollected++;

    float coinX = blocks[blockIndex].x + BLOCK_SIZE / 2;
    float coinY = blocks[blockIndex].y + BLOCK_SIZE / 2;

    // ギミック判定とエフェクト
    string gimmickType = DetectGimmickType(coinX, coinY);

    if (gimmickType != "normal") {
        // 特別なギミックコインの場合
        CreateGimmickClearEffect(coinX, coinY, 3);

        // ギミック完了チェック
        if (IsGimmickCompleted(gimmickType)) {
            CreateGimmickDiscoveryEffect(coinX, coinY, gimmickType);
        }
    }
    else {
        // 通常のコイン収集エフェクト
        CreateParticles(coinX, coinY, GetColor(255, 215, 0), 15);
        CreateSparkleEffect(coinX, coinY);
    }

    SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

    char debugMsg[128];
    sprintf_s(debugMsg, "BlockAthleticsScene: Coin collected (%s)! %d/%d\n",
        gimmickType.c_str(), coinsCollected, totalCoins);
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
        hudSystem.SetCurrentLife(playerLife);

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

    // ライフが0になったらゲームリセット
    if (playerLife <= 0) {
        // **修正: 完全なゲームリセットを実行**
        ResetGame();

        OutputDebugStringA("BlockAthleticsScene: Game Over! Full game reset executed\n");
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

void BlockAthleticsScene::DrawGimmickProgress()
{
    // ギミック進行状況パネル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(SCREEN_W - 400, 100, SCREEN_W - 50, 300, GetColor(20, 20, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawBox(SCREEN_W - 400, 100, SCREEN_W - 50, 300, GetColor(100, 150, 255), FALSE);

    DrawStringToHandle(SCREEN_W - 390, 110, "ギミック攻略状況", GetColor(255, 255, 255), fontHandle);

    // 主要ギミックの完了状況
    string gimmicks[] = {
        "coin_stairs", "treasure_box", "explosive_maze",
        "coin_tower", "secret_room"
    };
    string gimmickNames[] = {
        "コイン階段", "宝箱", "爆発迷路",
        "コインタワー", "隠し部屋"
    };

    for (int i = 0; i < 5; i++) {
        bool completed = IsGimmickCompleted(gimmicks[i]);
        int color = completed ? GetColor(100, 255, 100) : GetColor(255, 200, 100);
        string status = completed ? " ✓" : " ...";

        string displayText = gimmickNames[i] + status;
        DrawStringToHandle(SCREEN_W - 380, 140 + i * 25, displayText.c_str(), color, fontHandle);
    }

    // 全ギミック完了ボーナス表示
    int completedCount = 0;
    for (int i = 0; i < 5; i++) {
        if (IsGimmickCompleted(gimmicks[i])) completedCount++;
    }

    if (completedCount == 5) {
        DrawStringToHandle(SCREEN_W - 390, 270, "全ギミック制覇！", GetColor(255, 215, 0), fontHandle);
    }
    else {
        char progressText[64];
        sprintf_s(progressText, "制覇: %d/5", completedCount);
        DrawStringToHandle(SCREEN_W - 390, 270, progressText, GetColor(200, 200, 200), fontHandle);
    }
}

void BlockAthleticsScene::DrawUI()
{
    // プロフェッショナルなHUDシステム描画
    hudSystem.Draw();

    // スタイリッシュなゲーム情報パネル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 220);
    DrawBox(30, 200, 600, 450, GetColor(10, 10, 30), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // パネルの光る枠線
    for (int i = 0; i < 3; i++) {
        DrawBox(30 - i, 200 - i, 600 + i, 450 + i, GetColor(100 + i * 30, 200 + i * 20, 255), FALSE);
    }

    // タイトル（グラデーション風）
    string title = "GIMMICK ATHLETICS COURSE";
    DrawStringToHandle(40, 210, title.c_str(), GetColor(255, 215, 0), largeFontHandle);

    string subtitle = "Featuring: " + characterName;
    DrawStringToHandle(45, 255, subtitle.c_str(), GetColor(200, 255, 200), fontHandle);

    // 進行状況とギミック情報
    char progressText[256];

    // コイン収集（強調表示）
    sprintf_s(progressText, "TREASURES FOUND: %d/%d", coinsCollected, totalCoins);
    int coinColor = (coinsCollected >= totalCoins) ? GetColor(100, 255, 100) : GetColor(255, 215, 0);
    DrawStringToHandle(40, 290, progressText, coinColor, fontHandle);

    // コインプログレスバー
    float coinProgress = totalCoins > 0 ? (float)coinsCollected / totalCoins : 0.0f;
    DrawProgressBar(40, 315, 250, 15, coinProgress, GetColor(255, 215, 0), GetColor(100, 100, 50));

    // ギミック破壊状況
    sprintf_s(progressText, "GIMMICKS CLEARED: %d/%d", cratesDestroyed, totalCrates);
    DrawStringToHandle(40, 345, progressText, GetColor(255, 100, 100), fontHandle);

    // ギミックプログレスバー
    float gimmickProgress = totalCrates > 0 ? (float)cratesDestroyed / totalCrates : 0.0f;
    DrawProgressBar(40, 370, 250, 15, gimmickProgress, GetColor(255, 100, 100), GetColor(100, 50, 50));

    // タイム表示（大きく目立つ）
    sprintf_s(progressText, "TIME: %.2fs", gameTimer);
    DrawStringToHandle(350, 290, progressText, GetColor(100, 255, 255), largeFontHandle);

    // ベストタイム表示
    if (bestTime > 0.0f) {
        sprintf_s(progressText, "BEST: %.2fs", bestTime);
        DrawStringToHandle(350, 330, progressText, GetColor(255, 215, 0), fontHandle);
    }

    // 完了率表示
    float totalProgress = ((float)coinsCollected / totalCoins + (float)cratesDestroyed / totalCrates) / 2.0f;
    sprintf_s(progressText, "COMPLETION: %.1f%%", totalProgress * 100);
    DrawStringToHandle(350, 370, progressText, GetColor(255, 200, 255), fontHandle);

    // ギミック解説パネル
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
    DrawBox(30, SCREEN_H - 250, 900, SCREEN_H - 30, GetColor(20, 20, 40), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 操作説明の枠線
    DrawBox(30, SCREEN_H - 250, 900, SCREEN_H - 30, GetColor(100, 150, 255), FALSE);

    DrawStringToHandle(40, SCREEN_H - 240, "【優しいギミックコース攻略法】", GetColor(255, 255, 255), largeFontHandle);

    // ギミック説明（2列レイアウト）- より親しみやすい説明
    // 左列
    DrawStringToHandle(40, SCREEN_H - 200, "◆ 幅広コイン階段: ゆったり段々コインを登って収集", GetColor(255, 215, 0), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 175, "◆ 地上宝箱: 地面から簡単アクセスでお宝ゲット", GetColor(255, 100, 100), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 150, "◆ 安全ジグザグ道: 幅広足場で安心して進める", GetColor(150, 255, 150), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 125, "◆ 複数ルート迷路: 3つの安全な道から選択可能", GetColor(255, 150, 0), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 100, "◆ 低めコインタワー: 幅広階段で楽々登頂", GetColor(100, 200, 255), fontHandle);

    // 右列
    DrawStringToHandle(480, SCREEN_H - 200, "◆ 安全ブリッジ: 幅広橋で安心して渡れる", GetColor(200, 150, 100), fontHandle);
    DrawStringToHandle(480, SCREEN_H - 175, "◆ 低いピラミッド: 4段の優しいピラミッド探索", GetColor(255, 255, 100), fontHandle);
    DrawStringToHandle(480, SCREEN_H - 150, "◆ 大きな隠し部屋: 見つけやすい秘密の入口", GetColor(150, 255, 255), fontHandle);
    DrawStringToHandle(480, SCREEN_H - 125, "◆ 広い休憩エリア: たくさんの平地で安心休憩", GetColor(255, 150, 255), fontHandle);
    DrawStringToHandle(480, SCREEN_H - 100, "◆ 豪華ゴール台座: 低く広い最終ゴールエリア", GetColor(255, 215, 0), fontHandle);

    // 基本操作（下部）- 優しい説明に変更
    DrawStringToHandle(40, SCREEN_H - 75, "操作: ◄►のんびり移動 SPACE:ふわっとジャンプ ↓:ソフト急降下 B:ブロック破壊", GetColor(200, 255, 200), fontHandle);
    DrawStringToHandle(40, SCREEN_H - 50, "目標: リラックスしてコインを集めて、楽しくゴールを目指そう！", GetColor(255, 200, 100), fontHandle);

#ifdef _DEBUG
    // デバッグ時のみのテスト用操作説明
    DrawStringToHandle(650, SCREEN_H - 75, "テスト: 1/2キーでライフ減少/増加", GetColor(150, 150, 150), fontHandle);
#endif

    // 右下にESC情報
    DrawStringToHandle(SCREEN_W - 300, SCREEN_H - 50, "ESC: メニューに戻る", GetColor(150, 150, 150), fontHandle);

    // **ギミック進行状況表示を追加**
    DrawGimmickProgress();

    // ゲーム完了時の豪華な表示
    if (currentState == STATE_COMPLETED) {
        DrawGameClearScreen();
    }
}

void BlockAthleticsScene::DrawGameClearScreen()
{

    //メンバ関数のタイマーを使用する
    int overlayAlpha = (int)(200 + sinf(clearAnimTimer * 2) * 40);
    // 半透明オーバーレイ（アニメーション付き）
    static float clearAnimTimer = 0.0f;
    clearAnimTimer += 0.016f;

  
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, overlayAlpha);
    DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 10, 30), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 大きな完了パネル（脈打つエフェクト）
    float panelScale = 1.0f + sinf(clearAnimTimer * 3) * 0.05f;
    int panelWidth = (int)(800 * panelScale);
    int panelHeight = (int)(400 * panelScale);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 250);
    DrawBox(SCREEN_W / 2 - panelWidth / 2, SCREEN_H / 2 - panelHeight / 2,
        SCREEN_W / 2 + panelWidth / 2, SCREEN_H / 2 + panelHeight / 2,
        GetColor(10, 30, 60), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // 光る枠線（虹色エフェクト）
    for (int i = 0; i < 5; i++) {
        int r = (int)(128 + sinf(clearAnimTimer + i * 0.5f) * 127);
        int g = (int)(128 + sinf(clearAnimTimer + i * 0.5f + 2.1f) * 127);
        int b = (int)(128 + sinf(clearAnimTimer + i * 0.5f + 4.2f) * 127);

        DrawBox(SCREEN_W / 2 - panelWidth / 2 - i, SCREEN_H / 2 - panelHeight / 2 - i,
            SCREEN_W / 2 + panelWidth / 2 + i, SCREEN_H / 2 + panelHeight / 2 + i,
            GetColor(r, g, b), FALSE);
    }

    // 完了メッセージ（波打つエフェクト）
    string completedText = "チャンピオンシップ クリア！";
    int textWidth = GetDrawStringWidthToHandle(completedText.c_str(), (int)completedText.length(), largeFontHandle);

    for (int i = 0; i < 3; i++) {
        int offsetY = (int)(sinf(clearAnimTimer * 4 + i) * 5);
        DrawStringToHandle(SCREEN_W / 2 - textWidth / 2 + i, SCREEN_H / 2 - 120 + offsetY,
            completedText.c_str(), GetColor(255, 215, 0), largeFontHandle);
    }

    // 詳細結果
    char resultText[256];
    sprintf_s(resultText, "クリアタイム: %.2f秒", gameTimer);
    int timeWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - timeWidth / 2, SCREEN_H / 2 - 60, resultText, GetColor(100, 255, 255), fontHandle);

    // ベストタイム更新表示
    if (gameTimer <= bestTime || bestTime == gameTimer) {
        string newRecordText = "★ NEW RECORD! ★";
        int recordWidth = GetDrawStringWidthToHandle(newRecordText.c_str(), (int)newRecordText.length(), largeFontHandle);

        int blinkAlpha = (int)(255 * (0.5f + 0.5f * sinf(clearAnimTimer * 8)));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, blinkAlpha);
        DrawStringToHandle(SCREEN_W / 2 - recordWidth / 2, SCREEN_H / 2 - 30,
            newRecordText.c_str(), GetColor(255, 0, 0), largeFontHandle);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    sprintf_s(resultText, "コイン収集: %d/%d (%.1f%%)", coinsCollected, totalCoins,
        (float)coinsCollected / totalCoins * 100);
    int coinWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - coinWidth / 2, SCREEN_H / 2 + 20, resultText, GetColor(255, 215, 0), fontHandle);

    sprintf_s(resultText, "障害物クリア: %d/%d", cratesDestroyed, totalCrates);
    int crateWidth = GetDrawStringWidthToHandle(resultText, strlen(resultText), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - crateWidth / 2, SCREEN_H / 2 + 50, resultText, GetColor(255, 100, 100), fontHandle);

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
    DrawStringToHandle(SCREEN_W / 2 - gradeWidth / 2, SCREEN_H / 2 + 100, gradeText.c_str(), GetColor(255, 255, 100), largeFontHandle);

    // **修正: 残り時間を計算して表示**
    static float displayClearTimer = 0.0f;
    displayClearTimer += 0.016f;

    float remainingTime = 15.0f - displayClearTimer;
    if (remainingTime < 0) remainingTime = 0;

    // 終了指示（残り時間付き）
    char exitText[256];
    sprintf_s(exitText, "ESCキーでキャラクター選択に戻る（自動復帰まで %.1f秒）", remainingTime);

    int blinkAlpha = (int)(255 * (0.7f + 0.3f * sinf(clearAnimTimer * 4)));
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, blinkAlpha);
    int exitWidth = GetDrawStringWidthToHandle(exitText, strlen(exitText), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - exitWidth / 2, SCREEN_H / 2 + 160, exitText, GetColor(200, 200, 200), fontHandle);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **特別メッセージ（クリア達成を祝福）**
    if (clearAnimTimer < 3.0f) {
        string celebrationText = "🎉 素晴らしいパフォーマンス！ 🎉";
        int celebWidth = GetDrawStringWidthToHandle(celebrationText.c_str(), (int)celebrationText.length(), largeFontHandle);

        int celebAlpha = (int)(255 * (1.0f - clearAnimTimer / 3.0f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, celebAlpha);
        DrawStringToHandle(SCREEN_W / 2 - celebWidth / 2, SCREEN_H / 2 + 200,
            celebrationText.c_str(), GetColor(255, 255, 0), largeFontHandle);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
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
    playerLife = 6; // ライフをフル回復

    // プレイヤーをリセット
    InitializePlayer();

    // **重要: ブロックを完全に再生成**
    InitializeStage();

    // **重要: ゴールエリアも再初期化**
    InitializeGoalArea();

    // カメラをリセット
    cameraX = 0.0f;
    previousPlayerX = player.x;

    // エフェクトをクリア
    particles.clear();
    blockFragments.clear();

    // HUD更新
    hudSystem.SetCurrentLife(playerLife);

    OutputDebugStringA("BlockAthleticsScene: Game reset with full regeneration\n");
}

void BlockAthleticsScene::DestroyBlock(int blockIndex)
{
    Block& block = blocks[blockIndex];

    float blockX = block.x + BLOCK_SIZE / 2;
    float blockY = block.y + BLOCK_SIZE / 2;

    // ギミック判定
    string gimmickType = DetectGimmickType(blockX, blockY);

    // ブロックを非アクティブに
    block.isActive = false;

    // 破壊可能なブロックの場合はカウント
    if (block.isBreakable) {
        cratesDestroyed++;
    }

    // ギミック別の特別エフェクト
    if (gimmickType == "treasure_box") {
        CreateGimmickDiscoveryEffect(blockX, blockY, "Treasure Box Opened!");
        // 宝箱オープン音（コイン音を代用）
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);
    }
    else if (gimmickType == "explosive_maze" || gimmickType == "explosive_tower") {
        CreateExplosionEffect(blockX, blockY);
        // 爆発音
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_HIT);
    }
    else if (gimmickType == "secret_room") {
        CreateGimmickDiscoveryEffect(blockX, blockY, "Secret Room Discovered!");
    }
    else {
        // 通常の破壊エフェクト
        switch (block.type) {
        case BLOCK_EXPLOSIVE:
            CreateExplosionEffect(blockX, blockY);
            break;
        case BLOCK_CRATE:
            CreateDebrisEffect(blockX, blockY);
            break;
        default:
            CreateParticles(blockX, blockY, GetColor(150, 150, 150), 12);
            break;
        }
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_HIT);
    }

    // 破片エフェクト生成
    CreateBlockFragments(block.x, block.y, block.type);

    char debugMsg[128];
    sprintf_s(debugMsg, "BlockAthleticsScene: Block destroyed in %s gimmick at (%f, %f)\n",
        gimmickType.c_str(), block.x, block.y);
    OutputDebugStringA(debugMsg);
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

void BlockAthleticsScene::CompleteGame()
{
    currentState = STATE_COMPLETED;
    gameCompleted = true;

    // **タイマーをリセット**
    ResetClearTimers();

    // 豪華なクリアエフェクト
    CreateGameClearEffects();

    // クリア音
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

    // ベストタイム更新チェック
    if (bestTime == 0.0f || gameTimer < bestTime) {
        bestTime = gameTimer;
        OutputDebugStringA("BlockAthleticsScene: NEW BEST TIME!\n");
    }

    OutputDebugStringA("BlockAthleticsScene: GOAL REACHED! Game completed!\n");
}

// **新規追加: 豪華なクリアエフェクト**
void BlockAthleticsScene::CreateGameClearEffects()
{
    float centerX = goalArea.x + goalArea.width / 2;
    float centerY = goalArea.y + goalArea.height / 2;

    // 1. 大量の金色パーティクル（円形に広がる）
    for (int i = 0; i < 100; i++) {
        float angle = (float)i * 0.0628f; // 360度を100分割
        float radius = 50.0f + (rand() % 100);
        float speed = 3.0f + (rand() % 400) * 0.01f;

        Particle p;
        p.x = centerX + cosf(angle) * 30;
        p.y = centerY + sinf(angle) * 30;
        p.velocityX = cosf(angle) * speed;
        p.velocityY = sinf(angle) * speed - 2.0f;
        p.life = p.maxLife = 4.0f + (rand() % 200) * 0.01f;
        p.color = GetColor(255, 215, 0);
        p.scale = 1.2f + (rand() % 80) * 0.01f;
        particles.push_back(p);
    }

    // 2. 白いスパークル（上向きに噴出）
    for (int i = 0; i < 60; i++) {
        Particle p;
        p.x = centerX + (rand() % 200 - 100);
        p.y = centerY + (rand() % 50);
        p.velocityX = (rand() % 200 - 100) * 0.05f;
        p.velocityY = -5.0f - (rand() % 300) * 0.01f;
        p.life = p.maxLife = 5.0f;
        p.color = GetColor(255, 255, 255);
        p.scale = 0.8f + (rand() % 60) * 0.01f;
        particles.push_back(p);
    }

    // 3. 虹色の花火効果
    for (int wave = 0; wave < 3; wave++) {
        for (int i = 0; i < 20; i++) {
            float angle = (float)i * 0.314f; // 20分割
            float speed = 4.0f + wave * 2.0f;

            Particle p;
            p.x = centerX;
            p.y = centerY;
            p.velocityX = cosf(angle) * speed;
            p.velocityY = sinf(angle) * speed - 1.0f;
            p.life = p.maxLife = 3.0f + wave * 0.5f;

            // 虹色
            switch (i % 6) {
            case 0: p.color = GetColor(255, 0, 0); break;   // 赤
            case 1: p.color = GetColor(255, 127, 0); break; // オレンジ
            case 2: p.color = GetColor(255, 255, 0); break; // 黄
            case 3: p.color = GetColor(0, 255, 0); break;   // 緑
            case 4: p.color = GetColor(0, 0, 255); break;   // 青
            case 5: p.color = GetColor(127, 0, 255); break; // 紫
            }

            p.scale = 1.0f + wave * 0.3f;
            particles.push_back(p);
        }
    }

    OutputDebugStringA("BlockAthleticsScene: Game clear effects created!\n");
}

void BlockAthleticsScene::ResetClearTimers()
{
    clearStateTimer = 0.0f;
    clearAnimTimer = 0.0f;
}

// ギミック発見時の特別エフェクト
void BlockAthleticsScene::CreateGimmickDiscoveryEffect(float x, float y, const string& gimmickName)
{
    // 発見エフェクト（虹色の輪）
    for (int i = 0; i < 20; i++) {
        float angle = (float)i * 0.314f;
        Particle p;
        p.x = x;
        p.y = y;
        p.velocityX = cosf(angle) * 6.0f;
        p.velocityY = sinf(angle) * 6.0f - 2.0f;
        p.life = p.maxLife = 3.0f;

        // 虹色
        switch (i % 6) {
        case 0: p.color = GetColor(255, 0, 0); break;   // 赤
        case 1: p.color = GetColor(255, 127, 0); break; // オレンジ
        case 2: p.color = GetColor(255, 255, 0); break; // 黄
        case 3: p.color = GetColor(0, 255, 0); break;   // 緑
        case 4: p.color = GetColor(0, 0, 255); break;   // 青
        case 5: p.color = GetColor(127, 0, 255); break; // 紫
        }

        p.scale = 1.5f;
        particles.push_back(p);
    }

    // 特別なサウンド
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

    char debugMsg[256];
    sprintf_s(debugMsg, "BlockAthleticsScene: Discovered gimmick: %s\n", gimmickName.c_str());
    OutputDebugStringA(debugMsg);
}

// ギミッククリア時のエフェクト
void BlockAthleticsScene::CreateGimmickClearEffect(float x, float y, int coinCount)
{
    // 成功エフェクト（金色の爆発）
    for (int i = 0; i < 30; i++) {
        Particle p;
        float angle = (float)(rand() % 360) * 0.0174f;
        float speed = 4.0f + (rand() % 300) * 0.02f;

        p.x = x + (rand() % 60 - 30);
        p.y = y + (rand() % 60 - 30);
        p.velocityX = cosf(angle) * speed;
        p.velocityY = sinf(angle) * speed - 3.0f;
        p.life = p.maxLife = 2.5f + (rand() % 100) * 0.01f;
        p.color = GetColor(255, 215, 0);
        p.scale = 1.0f + (rand() % 50) * 0.02f;
        particles.push_back(p);
    }

    // ボーナスで白いキラキラも追加
    for (int i = 0; i < coinCount * 5; i++) {
        Particle p;
        p.x = x + (rand() % 120 - 60);
        p.y = y + (rand() % 120 - 60);
        p.velocityX = (rand() % 100 - 50) * 0.1f;
        p.velocityY = -2.0f - (rand() % 200) * 0.02f;
        p.life = p.maxLife = 4.0f;
        p.color = GetColor(255, 255, 255);
        p.scale = 0.8f + (rand() % 40) * 0.01f;
        particles.push_back(p);
    }
}

string BlockAthleticsScene::DetectGimmickType(float x, float y)
{
    int blockX = (int)(x / BLOCK_SIZE);

    // 各ギミックの位置範囲をチェック
    if (blockX >= 10 && blockX <= 14) return "coin_stairs";
    if (blockX >= 17 && blockX <= 19) return "treasure_box";
    if (blockX >= 22 && blockX <= 37) return "zigzag_road";
    if (blockX >= 40 && blockX <= 48) return "explosive_maze";
    if (blockX >= 53 && blockX <= 57) return "coin_tower";
    if (blockX >= 62 && blockX <= 70) return "crate_bridge";
    if (blockX >= 75 && blockX <= 78) return "coin_pyramid";
    if (blockX >= 85 && blockX <= 90) return "secret_room";
    if (blockX >= 95 && blockX <= 105) return "double_decker";
    if (blockX >= 110 && blockX <= 113) return "explosive_tower";

    return "normal";
}

// ギミック完了判定
bool BlockAthleticsScene::IsGimmickCompleted(const string& gimmickType)
{
    // 各ギミックエリアのコインがすべて収集されたかチェック
    // 簡易実装：エリア内のアクティブなコインブロックをカウント

    int startX = 0, endX = 0;

    if (gimmickType == "coin_stairs") { startX = 10; endX = 14; }
    else if (gimmickType == "treasure_box") { startX = 17; endX = 19; }
    else if (gimmickType == "zigzag_road") { startX = 22; endX = 37; }
    else if (gimmickType == "explosive_maze") { startX = 40; endX = 48; }
    else if (gimmickType == "coin_tower") { startX = 53; endX = 57; }
    else if (gimmickType == "crate_bridge") { startX = 62; endX = 70; }
    else if (gimmickType == "coin_pyramid") { startX = 75; endX = 78; }
    else if (gimmickType == "secret_room") { startX = 85; endX = 90; }
    else if (gimmickType == "double_decker") { startX = 95; endX = 105; }
    else if (gimmickType == "explosive_tower") { startX = 110; endX = 113; }
    else return false;

    // エリア内のアクティブなコインをカウント
    int activeCoins = 0;
    for (const auto& block : blocks) {
        if (block.type == BLOCK_COIN && block.isActive) {
            int blockX = (int)(block.x / BLOCK_SIZE);
            if (blockX >= startX && blockX <= endX) {
                activeCoins++;
            }
        }
    }

    return (activeCoins == 0); // エリア内のコインがすべてなくなったら完了
}