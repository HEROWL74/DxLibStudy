#include "GameScene.h"
#include <math.h>
using namespace std;

GameScene::GameScene()
    : selectedCharacterIndex(-1)
    , cameraX(0.0f)
    , previousPlayerX(0.0f)
    , exitRequested(false)
    , escPressed(false), escPressedPrev(false)
    , stageSelectPressed(false), stageSelectPressedPrev(false)
    , playerLife(6)
    , playerCoins(0)
    , playerStars(0)
    , currentStageIndex(0)
    , fadeState(FADE_NONE)
    , fadeAlpha(0.0f)
    , fadeTimer(0.0f)
    , showingResult(false)
    , goalReached(false)
    , doorOpened(false)
    , playerEnteringDoor(false)
    , playerInvulnerable(false)
    , invulnerabilityTimer(0.0f)
 
{
    backgroundHandle = -1;
    fontHandle = -1;
    stageChangeRequested = false;
    requestedStageIndex = -1;
} 


GameScene::~GameScene()
{
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
}

void GameScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_GAME);

    // 背景とフォント読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);

    // キャラクター情報設定
    characterName = GetCharacterDisplayName(selectedCharacter);

    // プレイヤー初期化
    gamePlayer.Initialize(selectedCharacter);
    gamePlayer.SetAutoWalking(false);

    // ステージシステム初期化
    currentStageIndex = 0;
    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // カメラ初期位置
    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // **各システム初期化**
    InitializeHUD();
    coinSystem.Initialize();
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.Initialize();
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    resultUI.Initialize();
    goalSystem.Initialize();
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
    doorSystem.Initialize();
    doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
    enemyManager.GenerateEnemiesForStage(currentStageIndex);
    goalSystem.ResetGoal(); // ゴール状態をリセット

    blockSystem.Initialize();
    blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

    // 収集カウントをリセット
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();
    blockSystem.ResetCoinCount(); // **ブロックからのコインもリセット**

    // 状態をリセット
    playerStars = 0;
    playerCoins = 0;
    doorOpened = false;
    playerEnteringDoor = false;
}
void GameScene::Update()
{
    // リザルト表示中の場合はリザルトのみ更新
    if (showingResult) {
        UpdateResult();
        return;
    }

    // 入力更新
    UpdateInput();

    // **遅延自動歩行の処理**
    UpdateDelayedAutoWalk();

    // **プレイヤー更新（自動歩行対応）**
    if (gamePlayer.IsAutoWalking()) {
        UpdatePlayerAutoWalk();
    }
    else {
        float prevX = gamePlayer.GetX();
        float prevY = gamePlayer.GetY();

        gamePlayer.Update(&stageManager);

        float newX = gamePlayer.GetX();
        float newY = gamePlayer.GetY();

        if (blockSystem.CheckCollision(newX, newY, 80.0f, 100.0f)) {
            blockSystem.HandlePlayerCollision(&gamePlayer, newX, newY);
        }
    }

    // **プレイヤーの無敵状態更新**
    UpdatePlayerInvulnerability();

    // **各システム更新**
    float hudCoinIconScreenX = 30 + 80 + 20 + 48 / 2;
    float hudCoinIconScreenY = 30 + 64 + 20 + 48 / 2;
    float hudCoinIconWorldX = hudCoinIconScreenX + cameraX;
    float hudCoinIconWorldY = hudCoinIconScreenY;

    coinSystem.Update(&gamePlayer, hudCoinIconWorldX, hudCoinIconWorldY);
    starSystem.Update(&gamePlayer);
    enemyManager.Update(&gamePlayer, &stageManager);
    blockSystem.Update(&gamePlayer);

    // **敵とプレイヤーの相互作用用更新（自動歩行中は無効化）**
    if (!gamePlayer.IsAutoWalking()) {
        UpdatePlayerEnemyInteractions();
    }

    // **ゴールシステム更新**
    goalSystem.Update(&gamePlayer);

    // **ドアシステム更新**
    doorSystem.Update(&gamePlayer);

   

    // **ドア相互作用用更新**
    UpdateDoorInteraction();

    // **ゴールタッチ後のリザルト表示**
    if (goalSystem.IsGoalTouched() && !goalReached) {
        HandleGoalReached();
    }

    // **フェード処理**
    UpdateFade();

    // カメラ更新
    UpdateCamera();

    // ステージ更新
    stageManager.Update(cameraX);

    // **ゲームロジック更新**
    UpdateGameLogic();

    // **HUD更新**
    UpdateHUD();

    // **ESCキーでタイトルに戻る（修正版）**
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
        OutputDebugStringA("GameScene: ESC pressed - returning to title\n");
    }
}

void GameScene::UpdateDoorInteraction()
{
    // **自動歩行状態のデバッグ情報**
    static int debugCounter = 0;
    debugCounter++;

    if (debugCounter % 60 == 0) { // 1秒ごとにデバッグ出力
        char debugMsg[256];
        sprintf_s(debugMsg, "GameScene: AutoWalk:%s, DoorOpened:%s, PlayerEntering:%s\n",
            gamePlayer.IsAutoWalking() ? "ON" : "OFF",
            doorOpened ? "YES" : "NO",
            doorSystem.IsPlayerEntering() ? "YES" : "NO");
        OutputDebugStringA(debugMsg);
    }

    // ドアが開いていて、プレイヤーがドアに完全に入った場合
    if (doorOpened && doorSystem.IsPlayerFullyEntered() && !showingResult) {
        // **自動歩行モードを解除**
        gamePlayer.SetAutoWalking(false);

        HandlePlayerEnteredDoor();
    }
}

void GameScene::HandleGoalReached()
{
    goalReached = true;

    // ステージ5未満でドアが存在する場合
    if (currentStageIndex < 4 && doorSystem.IsDoorExists()) {
        doorSystem.OpenDoor();
        doorOpened = true;

        // **修正：プレイヤーの状態を安定化してから自動歩行を開始**
        if (gamePlayer.GetState() == Player::JUMPING ||
            gamePlayer.GetState() == Player::FALLING ||
            gamePlayer.GetVelocityY() != 0.0f) {

            // **プレイヤーの速度を完全にリセット（飛び上がりを防止）**
            float currentX = gamePlayer.GetX();
            float currentY = gamePlayer.GetY();

            // 速度を0にリセット
            gamePlayer.SetPosition(currentX, currentY);

            // **段階的な自動歩行開始（遅延フレーム追加）**
            // フレーム遅延を追加して物理演算を安定化
            autoWalkDelayFrames = 10; // 10フレーム後に自動歩行開始
            pendingAutoWalk = true;

            char debugMsg[256];
            sprintf_s(debugMsg, "GameScene: Goal reached during jump/fall - Delayed auto-walk start. VelY was: %.2f\n",
                gamePlayer.GetVelocityY());
            OutputDebugStringA(debugMsg);
        }
        else {
            // **通常状態からの穏やかな自動歩行移行**
            gamePlayer.SetAutoWalking(true);
        }

        // **デバッグ出力**
        OutputDebugStringA("GameScene: Goal reached! Door opened, auto-walk prepared.\n");
    }
    else {
        // ステージ5の場合：直接リザルト表示
        ShowStageResult();
    }
}
// HandlePlayerEnteredDoor関数を修正
void GameScene::HandlePlayerEnteredDoor()
{
    playerEnteringDoor = true;

    // **重要: 自動歩行モードを即座に解除**
    gamePlayer.SetAutoWalking(false);

    ShowStageResult();
}


void GameScene::UpdatePlayerEnemyInteractions()
{
    // **修正: より詳細な敵との衝突処理システム**
    if (enemyManager.CheckPlayerEnemyCollisions(&gamePlayer)) {
        HandlePlayerEnemyCollision();
    }
}


void GameScene::HandlePlayerEnemyCollision()
{
    // 無敵状態の場合は何もしない
    if (playerInvulnerable) return;

    // **敵との衝突を詳細に処理**
    // より詳細な判定はEnemyManager.CheckDetailedPlayerEnemyCollision で行われる

    // **プレイヤーが敵の上から踏んだかどうかの詳細判定**
    bool playerStompedEnemy = CheckIfPlayerStompedEnemy();

    if (!playerStompedEnemy) {
        // **横からの接触によるダメージ**
        HandlePlayerDamage(1); // 1ダメージ（ハーフハート）
    }
    else {
        // **踏みつけ成功時の処理**
        HandleSuccessfulStomp();
    }
}
// **プレイヤーの状態リセット時にドア状態もリセット**
void GameScene::HandlePlayerDamage(int damage)
{
    if (playerInvulnerable) return;

    // **ライフ減少処理**
    int oldLife = playerLife;
    playerLife -= damage;
    // ダメージ時
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HURT);
    if (playerLife < 0) playerLife = 0;

    // **HUDシステムに即座にライフ変更を通知**
    hudSystem.SetCurrentLife(playerLife);

    // 無敵状態を開始
    playerInvulnerable = true;
    invulnerabilityTimer = 0.0f;

    // **デバッグ出力**
    char debugMsg[256];
    sprintf_s(debugMsg, "GameScene: Player took %d damage! Life: %d -> %d\n",
        damage, oldLife, playerLife);
    OutputDebugStringA(debugMsg);

    // プレイヤーが死亡した場合の処理
    if (playerLife <= 0) {
        // リスポーン処理
        gamePlayer.ResetPosition();
        playerLife = 6; // ライフを全回復

        // HUDを更新
        hudSystem.SetCurrentLife(playerLife);

        // 無敵状態をリセット
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        // **ドア状態もリセット（新追加）**
        doorOpened = false;
        playerEnteringDoor = false;
        if (doorSystem.IsDoorExists()) {
            // ドアを閉じた状態に戻す（必要に応じて）
            doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        }

        // ゴール状態もリセット
        goalReached = false;
        goalSystem.ResetGoal();

        // デバッグ出力
        OutputDebugStringA("GameScene: Player respawned with full life!\n");
    }
}

void GameScene::UpdatePlayerInvulnerability()
{
    if (playerInvulnerable) {
        invulnerabilityTimer += 0.016f; // 60FPS想定

        if (invulnerabilityTimer >= INVULNERABILITY_DURATION) {
            playerInvulnerable = false;
            invulnerabilityTimer = 0.0f;
        }
    }
}

// **ステージ全体のリセット時にドア状態を含める**
void GameScene::UpdateGameLogic()
{
    // **コイン収集数をゲーム状態に反映（リアルタイム更新）**
    int newCoinCount = coinSystem.GetCollectedCoinsCount();
    if (newCoinCount != playerCoins) {
        playerCoins = newCoinCount;
        // コイン取得時のサウンドやエフェクトをここに追加可能
    }

    // **星収集数をゲーム状態に反映**
    int newStarCount = starSystem.GetCollectedStarsCount();
    if (newStarCount != playerStars) {
        playerStars = newStarCount;
        // 星取得時のサウンドやエフェクトをここに追加可能
    }

    // ここでゲームロジックを更新
    // 例：ダメージ処理、アイテム取得など

    // **テスト用：キー入力でライフを操作**
    static bool key1Pressed = false, key1PressedPrev = false;
    static bool key2Pressed = false, key2PressedPrev = false;
    static bool key3Pressed = false, key3PressedPrev = false;

    key1PressedPrev = key1Pressed;
    key2PressedPrev = key2Pressed;
    key3PressedPrev = key3Pressed;

    key1Pressed = CheckHitKey(KEY_INPUT_1) != 0;
    key2Pressed = CheckHitKey(KEY_INPUT_2) != 0;
    key3Pressed = CheckHitKey(KEY_INPUT_3) != 0;

    // テスト用操作
    if (key1Pressed && !key1PressedPrev) {
        // 1キーでライフ減少（ハート割れテスト）
        if (playerLife > 0) {
            playerLife--;
            hudSystem.SetCurrentLife(playerLife); // HUDに即座に反映
            OutputDebugStringA("GameScene: Manual life decrease for testing.\n");
        }
    }

    if (key2Pressed && !key2PressedPrev) {
        // 2キーでライフ回復
        if (playerLife < 6) {
            playerLife++;
            hudSystem.SetCurrentLife(playerLife); // HUDに即座に反映
            OutputDebugStringA("GameScene: Manual life increase for testing.\n");
        }
    }

    if (key3Pressed && !key3PressedPrev) {
        // 3キーでコイン・星全配置（テスト用）
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        enemyManager.GenerateEnemiesForStage(currentStageIndex);
        // **ブロックシステムもリセット（新追加）**
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);
        // **ドア状態もリセット（新追加）**
        doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        doorOpened = false;
        playerEnteringDoor = false;
        goalReached = false;
        goalSystem.ResetGoal();

        OutputDebugStringA("GameScene: Regenerated all items, enemies, and door for testing.\n");
    }
}

void GameScene::UpdateHUD()
{
    // HUDシステムを更新
    hudSystem.Update();

    // ゲーム状態をHUDに反映
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.SetCoins(playerCoins);
    hudSystem.SetCollectedStars(playerStars);  // 星数を反映
    hudSystem.SetTotalStars(3);               // 総星数を設定
}

void GameScene::UpdateFade()
{
    const float FADE_SPEED = 0.02f; // フェード速度

    switch (fadeState) {
    case FADE_OUT:
        fadeTimer += 0.016f; // 60FPS想定
        fadeAlpha = fadeTimer * (1.0f / FADE_SPEED);

        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            fadeState = FADE_IN;
            fadeTimer = 0.0f;

            // ステージ切り替え
            StartNextStage();
        }
        break;

    case FADE_IN:
        fadeTimer += 0.016f;
        fadeAlpha = 1.0f - (fadeTimer * (1.0f / FADE_SPEED));

        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fadeState = FADE_NONE;
            fadeTimer = 0.0f;
        }
        break;

    case FADE_NONE:
    default:
        break;
    }
}

void GameScene::StartNextStage()
{
    // 次のステージに移行
    currentStageIndex = (currentStageIndex + 1) % 5;

    // ステージが最高（4番目）の場合の特別処理
    if (currentStageIndex >= 4) {
        // 最高のステージ - 特別な処理をここに追加可能
        // 現在は通常通りステージを読み込み
    }

    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // プレイヤー位置をリセット
    gamePlayer.ResetPosition();

    // **重要: 自動歩行モードを確実にリセット**
    gamePlayer.SetAutoWalking(false);

    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // **ブロックシステムのリセット（新追加）**
    blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

    // コイン、星、ゴール、ドア、敵を再配置
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
    doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
    enemyManager.GenerateEnemiesForStage(currentStageIndex);
    goalSystem.ResetGoal(); // ゴール状態をリセット

    // 収集カウントをリセット
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();
    blockSystem.ResetCoinCount(); // **ブロックからのコインもリセット**

    // 状態をリセット
    playerStars = 0;
    playerCoins = 0;
    doorOpened = false;
    playerEnteringDoor = false;
}

void GameScene::UpdateResult()
{
    resultUI.Update();

    // **リザルトが完全に非表示になったら、通常モードに戻る**
    if (resultUI.IsHidden()) {
        showingResult = false;
    }

    HandleResultButtons();
}

void GameScene::ShowStageResult()
{
    showingResult = true;
    playerStars = starSystem.GetCollectedStarsCount();
    resultUI.ShowResult(playerStars, 3, currentStageIndex + 1);
}

void GameScene::HandleResultButtons()
{
    ResultUISystem::ButtonAction action = resultUI.GetClickedButton();

    switch (action) {
    case ResultUISystem::BUTTON_RETRY:
        // ステージをリトライ
        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **重要: 自動歩行モードを確実にリセット**
        gamePlayer.SetAutoWalking(false);

        // **ブロックシステムをリセット（新追加）**
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

        // システムをリセット
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        enemyManager.GenerateEnemiesForStage(currentStageIndex);
        goalSystem.ResetGoal();

        // 収集カウントをリセット
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();
        blockSystem.ResetCoinCount(); // **ブロックからのコインもリセット**

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;
        doorOpened = false;
        playerEnteringDoor = false;
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        // **新追加：遅延自動歩行システムのリセット**
        pendingAutoWalk = false;
        autoWalkDelayFrames = 0;

        // **リザルトUIを完全リセット**
        resultUI.ResetState();
        break;

    case ResultUISystem::BUTTON_NEXT_STAGE:
        // 次のステージへ
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **重要: 自動歩行モードを確実にリセット**
        gamePlayer.SetAutoWalking(false);

        // **ブロックシステムの新ステージ初期化（新追加）**
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

        // 新ステージのシステム初期化
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        enemyManager.GenerateEnemiesForStage(currentStageIndex);
        goalSystem.ResetGoal();

        // 収集カウントをリセット
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();
        blockSystem.ResetCoinCount(); // **ブロックからのコインもリセット**

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;
        doorOpened = false;
        playerEnteringDoor = false;
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        // **新追加：遅延自動歩行システムのリセット**
        pendingAutoWalk = false;
        autoWalkDelayFrames = 0;

        // **リザルトUIを完全リセット**
        resultUI.ResetState();
        break;

    default:
        break;
    }
}


void GameScene::Draw()
{
    // **背景描画**
    DrawSeamlessBackground();

    // ステージ描画
    stageManager.Draw(cameraX);

    // **ブロック描画（ステージの後、プレイヤーの前）**
    blockSystem.Draw(cameraX);

    // **コイン描画**
    coinSystem.Draw(cameraX);

    // **星描画**
    starSystem.Draw(cameraX);

    // **敵描画**
    enemyManager.Draw(cameraX);

    // **ゴール描画**
    goalSystem.Draw(cameraX);

    // **ドア描画**
    doorSystem.Draw(cameraX);

    blockSystem.Draw(cameraX);

    // **プレイヤー描画（ドアに入っている時は非表示）**
    if (!playerEnteringDoor || !doorSystem.IsPlayerFullyEntered()) {
        // プレイヤー描画（無敵状態時は点滅）
        if (!playerInvulnerable || (int)(invulnerabilityTimer * 10) % 2 == 0) {
            gamePlayer.Draw(cameraX);
        }
    }

    // プレイヤーの影を描画
    if (!playerEnteringDoor || !doorSystem.IsPlayerFullyEntered()) {
        gamePlayer.DrawShadow(cameraX, &stageManager);
    }

    // **HUDシステム描画（最前面）**
    hudSystem.Draw();

    // UI描画（デバッグ情報など）
    if (!showingResult) { // **ポーズ中は非表示**
        DrawUI();
    }

    // **リザルトUI描画**
    resultUI.Draw();

  
 

    // **フェード描画（最前面）**
    DrawFade();
}

void GameScene::DrawSeamlessBackground()
{
    // 背景画像のサイズを取得
    int bgWidth, bgHeight;
    GetGraphSize(backgroundHandle, &bgWidth, &bgHeight);

    // パララックス効果（背景は前景より遅く移動）
    float parallaxSpeed = 0.3f; // 背景の移動速度（0.3倍）
    float bgOffsetX = cameraX * parallaxSpeed;

    // 背景をタイル状に描画するための開始位置を計算
    int startTileX = (int)(bgOffsetX / bgWidth) - 1; // 余裕を持って1枚前から
    int endTileX = startTileX + (SCREEN_W / bgWidth) + 3; // 余裕を持って3枚上まで

    // 背景をシームレスに描画
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
        float drawX = tileX * bgWidth - bgOffsetX;

        // 背景画像を描画
        DrawExtendGraph(
            (int)drawX, 0,
            (int)drawX + bgWidth, SCREEN_H,
            backgroundHandle, TRUE
        );
    }
}
void GameScene::DrawFade()
{
    if (fadeState != FADE_NONE && fadeAlpha > 0.0f) {
        int alpha = (int)(255 * fadeAlpha);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void GameScene::UpdateInput()
{
    // 前フレームの入力を保存
    escPressedPrev = escPressed;
    stageSelectPressedPrev = stageSelectPressed;

    // 現在の入力を取得
    escPressed = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    stageSelectPressed = CheckHitKey(KEY_INPUT_TAB) != 0;

    // **ステージ変更リクエスト（TABキーなど）**
    static bool tabWasPressed = false;
    bool tabPressed = CheckHitKey(KEY_INPUT_TAB) != 0;

    if (tabPressed && !tabWasPressed && !showingResult) {
        stageChangeRequested = true;
        requestedStageIndex = (currentStageIndex + 1) % 5;
    }
    tabWasPressed = tabPressed;
}
void GameScene::UpdateCamera()
{
    float currentPlayerX = gamePlayer.GetX();

    // プレイヤーを画面の少し左寄りに配置（先の視界を確保）
    float targetCameraX = currentPlayerX - SCREEN_W * 0.35f;

    // 範囲制限
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) {
        targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;
    }

    // 非常に滑らかな追従（遅めの設定でブレを防止）
    cameraX = Lerp(cameraX, targetCameraX, CAMERA_FOLLOW_SPEED);

    // 前フレームのプレイヤー位置を更新
    previousPlayerX = currentPlayerX;
}

void GameScene::UpdateCameraSimple()
{
    float currentPlayerX = gamePlayer.GetX();

    // プレイヤーを画面の少し左寄りに配置
    float targetCameraX = currentPlayerX - SCREEN_W * 0.35f;

    // 範囲制限
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) {
        targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;
    }

    // 非常に滑らかな追従（より遅めの設定）
    cameraX = Lerp(cameraX, targetCameraX, 0.03f); // 0.03fでより滑らかに
}

// **修正されたDrawUI関数（ポーズ情報追加）**
void GameScene::DrawUI()
{
#ifdef _DEBUG
    // ゲームタイトル
    string gameTitle = "STAGE ADVENTURE";
    int titleWidth = GetDrawStringWidthToHandle(gameTitle.c_str(), (int)gameTitle.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - titleWidth / 2, 20, gameTitle.c_str(), GetColor(255, 255, 255), fontHandle);

    // 選択されたキャラクター情報表示
    string characterInfo = "Playing as: " + characterName;
    int infoWidth = GetDrawStringWidthToHandle(characterInfo.c_str(), (int)characterInfo.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - infoWidth / 2, 60, characterInfo.c_str(), GetColor(255, 215, 0), fontHandle);

    // 操作説明（半透明背景付き）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(20, SCREEN_H - 260, 700, SCREEN_H - 20, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawStringToHandle(30, SCREEN_H - 250, "Controls:", GetColor(255, 255, 255), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 210, "Left/Right: Move, Space: Jump, Down: Duck/Slide", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 170, "TAB: Change Stage, ESC: Pause Game", GetColor(200, 200, 200), fontHandle); // **修正**

    // **テスト用操作説明（拡張）**
    DrawStringToHandle(30, SCREEN_H - 130, "Test: 1/2: Life -/+, 3: Reset Items & Enemies", GetColor(150, 150, 150), fontHandle);

    // デバッグ情報を強化
    string debugInfo = "=== DEBUG INFO ===";
    DrawStringToHandle(30, SCREEN_H - 240, debugInfo.c_str(), GetColor(255, 255, 0), fontHandle);

    // プレイヤー位置情報
    string posInfo = "Position: (" + to_string((int)gamePlayer.GetX()) + ", " + to_string((int)gamePlayer.GetY()) + ")";
    DrawStringToHandle(30, SCREEN_H - 210, posInfo.c_str(), GetColor(150, 150, 150), fontHandle);

    // **HUD状態情報とカメラデバッグ情報**
    string hudInfo = "Life: " + to_string(playerLife) + "/6, Coins: " + to_string(playerCoins) + ", Stars: " + to_string(playerStars) + "/3";
    DrawStringToHandle(30, SCREEN_H - 100, hudInfo.c_str(), GetColor(100, 200, 255), fontHandle);

    // **敵の状態情報**
    string enemyInfo = "Enemies: " + to_string(enemyManager.GetActiveEnemyCount()) + " active, " + to_string(enemyManager.GetDeadEnemyCount()) + " defeated";
    DrawStringToHandle(30, SCREEN_H - 80, enemyInfo.c_str(), GetColor(255, 150, 150), fontHandle);

    // **ドア状態情報**
    string doorInfo = "Door: ";
    if (doorSystem.IsDoorExists()) {
        if (doorOpened) {
            doorInfo += "OPEN";
        }
        else {
            doorInfo += "CLOSED";
        }
        if (playerEnteringDoor) {
            doorInfo += " (Player Entering)";
        }
    }
    else {
        doorInfo += "NONE (Stage 5)";
    }
    DrawStringToHandle(30, SCREEN_H - 60, doorInfo.c_str(), GetColor(100, 255, 100), fontHandle);



    // **無敵状態表示**
    if (playerInvulnerable) {
        string invulInfo = "INVULNERABLE: " + to_string(INVULNERABILITY_DURATION - invulnerabilityTimer).substr(0, 3) + "s";
        DrawStringToHandle(30, SCREEN_H - 20, invulInfo.c_str(), GetColor(255, 100, 100), fontHandle);
    }

    // 入力状態表示
    string inputInfo = "Input: ";
    if (CheckHitKey(KEY_INPUT_LEFT)) inputInfo += "[LEFT] ";
    if (CheckHitKey(KEY_INPUT_RIGHT)) inputInfo += "[RIGHT] ";
    if (CheckHitKey(KEY_INPUT_SPACE)) inputInfo += "[SPACE] ";
    if (CheckHitKey(KEY_INPUT_DOWN)) inputInfo += "[DOWN] ";
    if (inputInfo == "Input: ") inputInfo += "NONE";
    DrawStringToHandle(30, SCREEN_H - 180, inputInfo.c_str(), GetColor(200, 200, 100), fontHandle);

    // プレイヤーの内部状態
    string stateDetail = "OnGround: " + string(gamePlayer.GetState() != Player::JUMPING && gamePlayer.GetState() != Player::FALLING ? "YES" : "NO");
    DrawStringToHandle(30, SCREEN_H - 150, stateDetail.c_str(), GetColor(100, 200, 100), fontHandle);

    // 現在の状態表示
    string stateInfo = "State: ";
    switch (gamePlayer.GetState()) {
    case Player::IDLE: stateInfo += "IDLE"; break;
    case Player::WALKING: stateInfo += "WALKING"; break;
    case Player::JUMPING: stateInfo += "JUMPING"; break;
    case Player::FALLING: stateInfo += "FALLING"; break;
    case Player::DUCKING: stateInfo += "DUCKING"; break;
    case Player::SLIDING: stateInfo += "SLIDING"; break; // **新追加**
    case Player::HIT: stateInfo += "HIT"; break;
    }
    stateInfo += ", Direction: " + string(gamePlayer.IsFacingRight() ? "Right" : "Left");
    DrawStringToHandle(SCREEN_W - 500, 30, stateInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // カメラ位置表示
    string cameraInfo = "Camera: " + to_string((int)cameraX);
    DrawStringToHandle(SCREEN_W - 500, 70, cameraInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // **ゴールとステージデバッグ情報**
    string stageInfo = "Stage: " + to_string(currentStageIndex + 1) + "/5";
    string goalInfo = "Goal: " + string(goalSystem.IsGoalTouched() ? "TOUCHED!" : "Active") + ", Result: " + string(showingResult ? "SHOWING" : "HIDDEN");
    DrawStringToHandle(30, SCREEN_H - 80, stageInfo.c_str(), GetColor(255, 200, 100), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 60, goalInfo.c_str(), GetColor(100, 255, 200), fontHandle);

    // **F2キーでの敵デバッグ情報切り替えヒント**
    DrawStringToHandle(SCREEN_W - 500, 110, "F2: Toggle Enemy Debug", GetColor(150, 150, 150), fontHandle);
#endif
}

void GameScene::InitializeHUD()
{
    // HUDシステムの初期化
    hudSystem.Initialize();

    // プレイヤーキャラクターをHUDに設定
    hudSystem.SetPlayerCharacter(selectedCharacterIndex);

    // 初期ライフとコインを設定
    hudSystem.SetMaxLife(6);        // 最大ライフ：3ハート × 2 = 6
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.SetCoins(playerCoins);
    hudSystem.SetCollectedStars(playerStars);
    hudSystem.SetTotalStars(3);

    // HUD表示位置を設定（左上から30ピクセル、拡大版）
    hudSystem.SetPosition(30, 30);
    hudSystem.SetVisible(true);
}

std::string GameScene::GetCharacterDisplayName(int index)
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

float GameScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float GameScene::SmoothLerp(float current, float target, float speed)
{
    // SmoothDamp風の滑らかな補間
    float distance = target - current;

    // 距離が非常に小さい場合は直接設定
    if (fabsf(distance) < 0.1f) {
        return target;
    }

    // イージングアウト効果
    float t = 1.0f - powf(1.0f - speed, 60.0f * 0.016f); // 60FPS想定
    return current + distance * t;
}

// **新機能: プレイヤーが敵を踏んだかの詳細判定**
bool GameScene::CheckIfPlayerStompedEnemy()
{
    float playerX = gamePlayer.GetX();
    float playerY = gamePlayer.GetY();
    float playerVelY = gamePlayer.GetVelocityY();

    // **アクティブな敵をチェック**
    const auto& enemies = enemyManager.GetEnemies(); // EnemyManagerのGetEnemies()を使用

    for (const auto& enemy : enemies) {
        if (!enemy || !enemy->IsActive() || enemy->IsDead()) continue;

        float enemyX = enemy->GetX();
        float enemyY = enemy->GetY();

        // **基本的な重なり判定**
        const float PLAYER_WIDTH = 80.0f;
        const float PLAYER_HEIGHT = 100.0f;
        const float ENEMY_WIDTH = 48.0f;
        const float ENEMY_HEIGHT = 56.0f;

        bool isOverlapping = (playerX - PLAYER_WIDTH / 2 < enemyX + ENEMY_WIDTH / 2 &&
            playerX + PLAYER_WIDTH / 2 > enemyX - ENEMY_WIDTH / 2 &&
            playerY - PLAYER_HEIGHT / 2 < enemyY + ENEMY_HEIGHT / 2 &&
            playerY + PLAYER_HEIGHT / 2 > enemyY - ENEMY_HEIGHT / 2);

        if (!isOverlapping) continue;

        // **踏みつけ判定**
        bool isStompFromAbove = (playerVelY > 0 &&
            playerY < enemyY &&
            playerY + PLAYER_HEIGHT / 2 >= enemyY - ENEMY_HEIGHT / 2);

        if (isStompFromAbove) {
            // **敵の種類による処理分岐**
            if (enemy->GetType() == EnemyBase::NORMAL_SLIME) {
                // **NormalSlime: 踏みつけ成功**
                ApplyStompBounce(); // プレイヤーに跳ね返り効果

                // デバッグ出力
                OutputDebugStringA("GameScene: Successfully stomped NormalSlime!\n");
                return true;
            }
            else if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
                // **SpikeSlime: トゲが出ているかでダメージが決まる**
                SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy.get());

                // **トゲが出ている場合は踏んでもダメージ**
                if (spikeSlime->AreSpikesOut()) {
                    // **トゲが出ている場合は踏み失敗（ダメージを受ける）**
                    return false; // 踏みつけ失敗（ダメージを受ける）
                }
                else {
                    // **トゲが出ていない場合は踏みつけ成功（敵がスタン）**
                    ApplyStompBounce(); // プレイヤーに跳ね返り効果

                    // デバッグ出力
                    OutputDebugStringA("GameScene: Successfully stomped SpikeSlime (spikes retracted)!\n");
                    return true;
                }
            }
        }
    }

    return false; // 踏みつけは発生していない
}
// **新機能: 踏みつけ成功時の処理**
void GameScene::HandleSuccessfulStomp()
{
    // **プレイヤーに跳ね返り効果を与える**
    ApplyStompBounce();

    // **効果音やエフェクトの再生**
    // PlaySoundEffect("enemy_stomp");
    // SpawnStompEffect(gamePlayer.GetX(), gamePlayer.GetY());
}

// **新機能: 踏みつけ時の跳ね返り効果**
void GameScene::ApplyStompBounce()
{
    // **プレイヤーに上向きの速度を与える（小さなジャンプ効果）**
    gamePlayer.ApplyStompBounce(-8.0f); // 上向きの速度を設定
}

// **新追加: 自動歩行専用の更新関数**
void GameScene::UpdatePlayerAutoWalk()
{
    // **自動歩行用の物理処理のみ実行**
    gamePlayer.UpdateAutoWalkPhysics(&stageManager);

    // **アニメーション更新**
    gamePlayer.UpdateAnimationOnly();

    // **デバッグ出力（60フレームに1回）**
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 60 == 0) {
        char debugMsg[256];
        sprintf_s(debugMsg, "GameScene: Auto-walking - Player X:%.1f, VelX:%.1f\n",
            gamePlayer.GetX(), gamePlayer.GetVelocityX());
        OutputDebugStringA(debugMsg);
    }
}

// **新機能：遅延自動歩行の処理**
void GameScene::UpdateDelayedAutoWalk()
{
    if (pendingAutoWalk && autoWalkDelayFrames > 0) {
        autoWalkDelayFrames--;

        if (autoWalkDelayFrames <= 0) {
            // **穏やかな自動歩行開始**
            gamePlayer.SetAutoWalking(true);
            pendingAutoWalk = false;

            OutputDebugStringA("GameScene: Delayed auto-walk started!\n");
        }
    }
}
