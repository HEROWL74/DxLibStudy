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
    try {
        blockSystem.Initialize();
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);
    }
    catch (...) {
        OutputDebugStringA("GameScene: BlockSystem initialization failed\n");
    }
   

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
    if (showingResult) {
        UpdateResult();
        return;
    }

    UpdateInput();
    UpdateDelayedAutoWalk();

    if (gamePlayer.IsAutoWalking()) {
        UpdatePlayerAutoWalk();
    }
    else {
        // **修正: 処理順序を変更**

        gamePlayer.UpdatePhysics(&stageManager);        // 1. 物理演算（位置更新なし）
        gamePlayer.HandleCollisions(&stageManager);     // 2. ステージ衝突判定と位置更新
        blockSystem.Update(&gamePlayer);
        blockSystem.CheckAndResolvePlayerCollisions(&gamePlayer); // 3. ブロック判定で最終調整
       
     

        gamePlayer.UpdateAnimation();                   // 4. アニメーション更新
    }

    // 残りの更新処理...
    UpdatePlayerInvulnerability();
    UpdateCoins();
    UpdatePlayerEnemyInteractions();

    // 他のシステム更新
    float hudCoinIconWorldX = 30 + 80 + 20 + 48 / 2 + cameraX;
    float hudCoinIconWorldY = 30 + 64 + 20 + 48 / 2;

    coinSystem.Update(&gamePlayer, hudCoinIconWorldX, hudCoinIconWorldY);
    starSystem.Update(&gamePlayer);
    enemyManager.Update(&gamePlayer, &stageManager);

    goalSystem.Update(&gamePlayer);
    doorSystem.Update(&gamePlayer);
    UpdateDoorInteraction();

    if (goalSystem.IsGoalTouched() && !goalReached) {
        HandleGoalReached();
    }

    UpdateFade();
    UpdateCamera();
    stageManager.Update(cameraX);
    UpdateGameLogic();
    UpdateHUD();

    if (escPressed && !escPressedPrev) {
        exitRequested = true;
        OutputDebugStringA("GameScene: ESC pressed - returning to title\n");
    }
}

void GameScene::UpdateCoins()
{
    int fieldCoins = coinSystem.GetCollectedCoinsCount();
    int blockCoins = blockSystem.GetCoinsFromBlocks();
    int totalCoins = fieldCoins + blockCoins;

    if (totalCoins != playerCoins) {
        playerCoins = totalCoins;
        hudSystem.SetCoins(playerCoins);

        char debugMsg[128];
        sprintf_s(debugMsg, "GameScene: Coins updated to %d (Field:%d + Blocks:%d)\n",
            totalCoins, fieldCoins, blockCoins);
        OutputDebugStringA(debugMsg);
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
    // **修正: 無敵状態チェック**
    if (playerInvulnerable) {
        return; // 無敵中は衝突判定をスキップ
    }

    // **修正: EnemyManager の詳細な衝突判定を使用**
    const auto& enemies = enemyManager.GetEnemies();

    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive() && !enemy->IsDead()) {
            // 詳細な衝突判定を実行
            if (enemyManager.CheckDetailedPlayerEnemyCollision(&gamePlayer, enemy.get())) {

                float playerX = gamePlayer.GetX();
                float playerY = gamePlayer.GetY();
                float playerVelY = gamePlayer.GetVelocityY();
                float enemyX = enemy->GetX();
                float enemyY = enemy->GetY();

                // 踏みつけ判定
                bool isStompFromAbove = (
                    playerVelY > 1.0f &&
                    playerY < enemyY - 15.0f &&
                    playerY + 50.0f >= enemyY - 28.0f
                    );

                if (isStompFromAbove) {
                    // **踏みつけ処理はEnemyManagerに任せる**
                    enemyManager.HandleStompInteraction(&gamePlayer, enemy.get());
                }
                else {
                    // **横からの衝突 - GameScene で直接ダメージ処理**

                    // ダメージ量を決定
                    int damageAmount = 1;
                    if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
                        SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy.get());
                        if (spikeSlime->AreSpikesOut()) {
                            damageAmount = 2;
                        }
                    }

                    // ノックバック方向を計算
                    float knockbackDirection = (playerX > enemyX) ? 1.0f : -1.0f;

                    // **重要: GameScene の HandlePlayerDamage を呼び出し**
                    HandlePlayerDamage(damageAmount);

                    // **Player クラスにもダメージとノックバックを適用**
                    gamePlayer.TakeDamage(damageAmount, knockbackDirection);

                    // **敵固有の衝突処理も呼び出し**
                    enemy->OnPlayerCollision(&gamePlayer);

                    char debugMsg[256];
                    sprintf_s(debugMsg, "GameScene: Enemy collision! Damage: %d, Life: %d -> %d\n",
                        damageAmount, playerLife + damageAmount, playerLife);
                    OutputDebugStringA(debugMsg);

                    // 一度の衝突処理で終了
                    return;
                }
            }
        }
    }
}
void GameScene::TakeDamageFromEnemy(int damage, float knockbackDirection)
{
    // **無敵状態チェック**
    if (playerInvulnerable) {
        OutputDebugStringA("GameScene: Player is invulnerable - no damage!\n");
        return;
    }

    // **ライフ減少処理**
    int oldLife = playerLife;
    playerLife -= damage;

    // **ダメージ音を確実に再生**
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HURT);

    if (playerLife < 0) playerLife = 0;

    // **HUDシステムに即座にライフ変更を通知**
    hudSystem.SetCurrentLife(playerLife);

    // **重要: Player::TakeDamage を呼び出してダメージ状態にする**
    gamePlayer.TakeDamage(damage, knockbackDirection);

    // **無敵状態を開始**
    playerInvulnerable = true;
    invulnerabilityTimer = 0.0f;

    // **デバッグ出力**
    char debugMsg[256];
    sprintf_s(debugMsg, "GameScene: Player took %d damage from enemy! Life: %d -> %d, Knockback: %.2f\n",
        damage, oldLife, playerLife, knockbackDirection);
    OutputDebugStringA(debugMsg);

    // **プレイヤーが死亡した場合の処理**
    if (playerLife <= 0) {
        // リスポーン処理
        gamePlayer.ResetPosition();
        playerLife = 6; // ライフを全回復

        // HUDを更新
        hudSystem.SetCurrentLife(playerLife);

        // 無敵状態をリセット
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        // **各種状態をリセット**
        doorOpened = false;
        playerEnteringDoor = false;
        goalReached = false;
        goalSystem.ResetGoal();

        OutputDebugStringA("GameScene: Player respawned with full life!\n");
    }
}


// GameScene.cpp の HandlePlayerDamage関数を修正

void GameScene::HandlePlayerDamage(int damage)
{
    if (playerInvulnerable) {
        OutputDebugStringA("GameScene: Player is invulnerable - no damage!\n");
        return;
    }

    // **ライフ減少処理**
    int oldLife = playerLife;
    playerLife -= damage;

    // **ダメージ音を確実に再生**
    SoundManager::GetInstance().PlaySE(SoundManager::SFX_HURT);

    if (playerLife < 0) playerLife = 0;

    // **重要: HUDシステムに即座にライフ変更を通知（ダメージアニメーション付き）**
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.NotifyDamage(); // 確実にアニメーション開始

    // **ノックバック方向を計算**
    float knockbackDirection = 0.0f;
    const auto& enemies = enemyManager.GetEnemies();
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive() && !enemy->IsDead()) {
            float enemyX = enemy->GetX();
            float playerX = gamePlayer.GetX();

            if (abs(enemyX - playerX) < 100.0f) {
                knockbackDirection = (playerX > enemyX) ? 1.0f : -1.0f;
                break;
            }
        }
    }

    // **重要: Player::TakeDamage を呼び出してダメージ状態にする**
    gamePlayer.TakeDamage(damage, knockbackDirection);

    // **無敵状態を開始**
    playerInvulnerable = true;
    invulnerabilityTimer = 0.0f;

    // **詳細なデバッグ出力**
    char debugMsg[256];
    sprintf_s(debugMsg, "GameScene: DAMAGE APPLIED! Life: %d -> %d, Damage: %d, Sound: YES, HUD: UPDATED\n",
        oldLife, playerLife, damage);
    OutputDebugStringA(debugMsg);

    // **プレイヤーが死亡した場合の処理**
    if (playerLife <= 0) {
        OutputDebugStringA("GameScene: Player died - respawning!\n");

        // リスポーン処理
        gamePlayer.ResetPosition();
        playerLife = 6; // ライフを全回復

        // HUDを更新
        hudSystem.SetCurrentLife(playerLife);

        // 無敵状態をリセット
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        // **各種状態をリセット**
        doorOpened = false;
        playerEnteringDoor = false;
        goalReached = false;
        goalSystem.ResetGoal();

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

void GameScene::UpdateGameLogic()
{
    // 星収集数をゲーム状態に反映
    int newStarCount = starSystem.GetCollectedStarsCount();
    if (newStarCount != playerStars) {
        playerStars = newStarCount;
        hudSystem.SetCollectedStars(playerStars);
    }

    // **テスト用キー入力の処理**
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
        if (playerLife > 0) {
            playerLife--;
            hudSystem.SetCurrentLife(playerLife);
            OutputDebugStringA("GameScene: Manual life decrease for testing.\n");
        }
    }

    if (key2Pressed && !key2PressedPrev) {
        if (playerLife < 6) {
            playerLife++;
            hudSystem.SetCurrentLife(playerLife);
            OutputDebugStringA("GameScene: Manual life increase for testing.\n");
        }
    }

    if (key3Pressed && !key3PressedPrev) {
        // **全システムリセット**
        ResetAllSystems();
        OutputDebugStringA("GameScene: All systems reset including block coins.\n");
    }
}


void GameScene::UpdateHUD()
{
    hudSystem.Update();

    // **現在の状態をHUDに反映**
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.SetCoins(playerCoins);
    hudSystem.SetCollectedStars(playerStars);
    hudSystem.SetTotalStars(3);

    // **デバッグ: ライフ状態を定期的に出力**
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 180 == 0) { // 3秒ごと
        char debugMsg[256];
        sprintf_s(debugMsg, "GameScene: HUD Status - Life: %d/6, Coins: %d, Stars: %d/3, Invulnerable: %s\n",
            playerLife, playerCoins, playerStars, playerInvulnerable ? "YES" : "NO");
        OutputDebugStringA(debugMsg);
    }
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

    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // プレイヤー位置をリセット
    gamePlayer.ResetPosition();
    gamePlayer.SetAutoWalking(false);

    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // **修正: ブロックシステムのリセットを追加**
    blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

    // その他のシステムも再配置
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
    doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
    enemyManager.GenerateEnemiesForStage(currentStageIndex);
    goalSystem.ResetGoal();

    // **修正: 収集カウントをリセット（ブロックからのコインも含む）**
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();
    blockSystem.ResetCoinCount();  // **ブロックからのコインをリセット**

    // 状態をリセット
    playerStars = 0;
    playerCoins = 0;  // **これも0にリセット**
    doorOpened = false;
    playerEnteringDoor = false;

    // **HUDも即座に更新**
    hudSystem.SetCoins(0);
    hudSystem.SetCollectedStars(0);

    OutputDebugStringA("GameScene: Started next stage with full system reset including blocks\n");
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
        gamePlayer.SetAutoWalking(false);
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **修正: ブロックシステムもリセット**
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

        // システムをリセット
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        enemyManager.GenerateEnemiesForStage(currentStageIndex);
        goalSystem.ResetGoal();

        // **修正: 収集カウントをリセット（ブロックからのコインも含む）**
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();
        blockSystem.ResetCoinCount();

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;
        doorOpened = false;
        playerEnteringDoor = false;
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        pendingAutoWalk = false;
        autoWalkDelayFrames = 0;

        // **HUDを即座に更新**
        hudSystem.SetCoins(0);
        hudSystem.SetCollectedStars(0);

        resultUI.ResetState();
        OutputDebugStringA("GameScene: Stage retried with full reset including blocks\n");
        break;

    case ResultUISystem::BUTTON_NEXT_STAGE:
        // 次のステージへ
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        gamePlayer.ResetPosition();
        gamePlayer.SetAutoWalking(false);
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **修正: 新ステージのブロックシステム初期化**
        blockSystem.GenerateBlocksForStageIndex(currentStageIndex);

        // 新ステージのシステム初期化
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);
        enemyManager.GenerateEnemiesForStage(currentStageIndex);
        goalSystem.ResetGoal();

        // **修正: 収集カウントをリセット（ブロックからのコインも含む）**
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();
        blockSystem.ResetCoinCount();

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;
        doorOpened = false;
        playerEnteringDoor = false;
        playerInvulnerable = false;
        invulnerabilityTimer = 0.0f;

        pendingAutoWalk = false;
        autoWalkDelayFrames = 0;

        // **HUDを即座に更新**
        hudSystem.SetCoins(0);
        hudSystem.SetCollectedStars(0);

        resultUI.ResetState();
        OutputDebugStringA("GameScene: Advanced to next stage with full initialization including blocks\n");
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

void GameScene::ResetAllSystems()
{
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    enemyManager.GenerateEnemiesForStage(currentStageIndex);
    blockSystem.GenerateBlocksForStageIndex(currentStageIndex);
    doorSystem.PlaceDoorForStage(currentStageIndex, &stageManager);

    // カウントもリセット
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();
    blockSystem.ResetCoinCount();

    doorOpened = false;
    playerEnteringDoor = false;
    goalReached = false;
    goalSystem.ResetGoal();

    // プレイヤーの状態もリセット
    playerStars = 0;
    playerCoins = 0;
    hudSystem.SetCoins(0);
    hudSystem.SetCollectedStars(0);
}

