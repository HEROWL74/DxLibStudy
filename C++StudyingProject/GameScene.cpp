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
    , playerLife(6)      // 初期ライフ：3ハート分
    , playerCoins(0)     // 初期コイン数
    , playerStars(0)     // 初期星数
    , currentStageIndex(0) // 初期ステージ
    , fadeState(FADE_NONE) // フェード状態
    , fadeAlpha(0.0f)    // フェード透明度
    , fadeTimer(0.0f)    // フェードタイマー
    , showingResult(false) // リザルト表示状態
    , goalReached(false)   // ゴール到達状態
{
    backgroundHandle = -1;
    fontHandle = -1;
}

GameScene::~GameScene()
{
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
}

void GameScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;

    // 背景とフォント読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);

    // キャラクター情報設定
    characterName = GetCharacterDisplayName(selectedCharacter);

    // プレイヤー初期化
    gamePlayer.Initialize(selectedCharacter);

    // ステージシステム初期化（デフォルトでGrassStage）
    currentStageIndex = 0;
    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // カメラ初期位置
    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // **HUDシステム初期化**
    InitializeHUD();

    // **コインシステム初期化（ステージ特化版）**
    coinSystem.Initialize();
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);

    // **星システム初期化（新機能）**
    starSystem.Initialize();
    starSystem.GenerateStarsForStageIndex(currentStageIndex);

    // **リザルトUI初期化（新機能）**
    resultUI.Initialize();

    // **ゴールシステム初期化**
    goalSystem.Initialize();
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);

    // 状態リセット
    showingResult = false;
    goalReached = false;
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

void GameScene::Update()
{
    // リザルト表示中の場合は、リザルトのみ更新
    if (showingResult) {
        UpdateResult();
        return;
    }

    // 入力更新
    UpdateInput();

    // ステージ切り替え（リザルト非表示時のみ）
    if (stageSelectPressed && !stageSelectPressedPrev && fadeState == FADE_NONE && !showingResult) {
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        // プレイヤー位置をリセット
        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **コイン、星、ゴールを再配置（星システム対応）**
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);

        // 収集カウントをリセット
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **リザルトUIも完全リセット**
        resultUI.ResetState();
    }

    // プレイヤー更新
    gamePlayer.Update(&stageManager);

    // **コインシステム更新（スクリーン座標でHUDのコインアイコン位置を渡す）**
    // HUDコインアイコンの正確なスクリーン座標を計算
    float hudCoinIconScreenX = 30 + 80 + 20 + 48 / 2; // スクリーン座標（カメラの影響なし）
    float hudCoinIconScreenY = 30 + 64 + 20 + 48 / 2; // スクリーン座標（カメラの影響なし）

    // スクリーン座標をワールド座標に変換
    float hudCoinIconWorldX = hudCoinIconScreenX + cameraX;
    float hudCoinIconWorldY = hudCoinIconScreenY; // Y座標はカメラの影響なし

    coinSystem.Update(&gamePlayer, hudCoinIconWorldX, hudCoinIconWorldY);

    // **星システム更新（新機能）**
    starSystem.Update(&gamePlayer);

    // **ゴールシステム更新**
    goalSystem.Update(&gamePlayer);

    // **ゴールタッチ時のリザルト表示（変更）**
    if (goalSystem.IsGoalTouched() && !goalReached) {
        goalReached = true;
        ShowStageResult();
    }

    // **フェード処理**
    UpdateFade();

    // カメラ更新（滑らかなシステムを使用）
    UpdateCamera();

    // ステージ更新
    stageManager.Update(cameraX);

    // **ゲームロジック更新**
    UpdateGameLogic();

    // **HUD更新**
    UpdateHUD();

    // ESCキーでタイトルに戻る
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
    }
}

void GameScene::UpdateGameLogic()
{
    // **コイン収集数をゲーム状態に反映（リアルタイム更新）**
    int newCoinCount = coinSystem.GetCollectedCoinsCount();
    if (newCoinCount != playerCoins) {
        playerCoins = newCoinCount;
        // コイン獲得時のサウンドやエフェクトをここに追加可能
    }

    // **星収集数をゲーム状態に反映（新機能）**
    int newStarCount = starSystem.GetCollectedStarsCount();
    if (newStarCount != playerStars) {
        playerStars = newStarCount;
        // 星獲得時のサウンドやエフェクトをここに追加可能
    }

    // ここでゲームロジックを更新
    // 例：ダメージ処理、アイテム獲得など

    // **テスト用：キー入力でライフを調整**
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
        // 1キーでライフ減少
        playerLife--;
        if (playerLife < 0) playerLife = 0;
    }

    if (key2Pressed && !key2PressedPrev) {
        // 2キーでライフ回復
        playerLife++;
        if (playerLife > 6) playerLife = 6;
    }

    if (key3Pressed && !key3PressedPrev) {
        // 3キーでコイン・星全配置（テスト用）
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
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
        // 現在は通常通りステージを読み込む
    }

    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // プレイヤー位置をリセット
    gamePlayer.ResetPosition();
    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // コイン、星、ゴールを再配置
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
    goalSystem.ResetGoal(); // ゴール状態をリセット

    // 収集カウントをリセット
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();

    // 状態をリセット
    playerStars = 0;
    playerCoins = 0;
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

        // システムをリセット
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        goalSystem.ResetGoal();

        // 収集カウントをリセット
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **リザルトUIを完全にリセット**
        resultUI.ResetState();
        break;

    case ResultUISystem::BUTTON_NEXT_STAGE:
        // 次のステージへ
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // 新ステージのシステム初期化
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        goalSystem.ResetGoal();

        // 収集カウントをリセット
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // 状態を完全リセット
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **リザルトUIを完全にリセット**
        resultUI.ResetState();
        break;

    default:
        break;
    }
}

void GameScene::Draw()
{
    // **背景描画（シームレスなタイリング）**
    DrawSeamlessBackground();

    // ステージ描画
    stageManager.Draw(cameraX);

    // **コイン描画**
    coinSystem.Draw(cameraX);

    // **星描画（新機能）**
    starSystem.Draw(cameraX);

    // **ゴール描画**
    goalSystem.Draw(cameraX);

    // プレイヤー描画
    gamePlayer.Draw(cameraX);

    // プレイヤーの影を描画
    gamePlayer.DrawShadow(cameraX, &stageManager);

    // **HUDシステム描画（最前面）**
    hudSystem.Draw();

    // UI描画（デバッグ情報など）
    if (!showingResult) {  // リザルト表示中はデバッグ情報を隠す
        DrawUI();
    }

    // **リザルトUI描画（新機能）**
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
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
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

void GameScene::DrawUI()
{
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
    DrawBox(20, SCREEN_H - 220, 700, SCREEN_H - 20, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawStringToHandle(30, SCREEN_H - 210, "Controls:", GetColor(255, 255, 255), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 170, "Left/Right: Move, Space: Jump, Down: Duck", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 130, "TAB: Change Stage, ESC: Return to title", GetColor(200, 200, 200), fontHandle);

    // **テスト用操作説明（拡張）**
    DrawStringToHandle(30, SCREEN_H - 90, "Test: 1/2: Life -/+, 3: Reset Items", GetColor(150, 150, 150), fontHandle);

    // デバッグ情報を強化
    string debugInfo = "=== DEBUG INFO ===";
    DrawStringToHandle(30, SCREEN_H - 200, debugInfo.c_str(), GetColor(255, 255, 0), fontHandle);

    // プレイヤー位置情報
    string posInfo = "Position: (" + to_string((int)gamePlayer.GetX()) + ", " + to_string((int)gamePlayer.GetY()) + ")";
    DrawStringToHandle(30, SCREEN_H - 170, posInfo.c_str(), GetColor(150, 150, 150), fontHandle);

    // **HUD状態情報とカメラデバッグ情報**
    string hudInfo = "Life: " + to_string(playerLife) + "/6, Coins: " + to_string(playerCoins) + ", Stars: " + to_string(playerStars) + "/3";
    DrawStringToHandle(30, SCREEN_H - 60, hudInfo.c_str(), GetColor(100, 200, 255), fontHandle);

    // 入力状態表示
    string inputInfo = "Input: ";
    if (CheckHitKey(KEY_INPUT_LEFT)) inputInfo += "[LEFT] ";
    if (CheckHitKey(KEY_INPUT_RIGHT)) inputInfo += "[RIGHT] ";
    if (CheckHitKey(KEY_INPUT_SPACE)) inputInfo += "[SPACE] ";
    if (CheckHitKey(KEY_INPUT_DOWN)) inputInfo += "[DOWN] ";
    if (inputInfo == "Input: ") inputInfo += "NONE";
    DrawStringToHandle(30, SCREEN_H - 140, inputInfo.c_str(), GetColor(200, 200, 100), fontHandle);

    // プレイヤーの内部状態
    string stateDetail = "OnGround: " + string(gamePlayer.GetState() != Player::JUMPING && gamePlayer.GetState() != Player::FALLING ? "YES" : "NO");
    DrawStringToHandle(30, SCREEN_H - 110, stateDetail.c_str(), GetColor(100, 200, 100), fontHandle);

    // 現在の状態表示
    string stateInfo = "State: ";
    switch (gamePlayer.GetState()) {
    case Player::IDLE: stateInfo += "IDLE"; break;
    case Player::WALKING: stateInfo += "WALKING"; break;
    case Player::JUMPING: stateInfo += "JUMPING"; break;
    case Player::FALLING: stateInfo += "FALLING"; break;
    case Player::DUCKING: stateInfo += "DUCKING"; break;
    }
    stateInfo += ", Direction: " + string(gamePlayer.IsFacingRight() ? "Right" : "Left");
    DrawStringToHandle(SCREEN_W - 500, 30, stateInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // カメラ位置表示
    string cameraInfo = "Camera: " + to_string((int)cameraX);
    DrawStringToHandle(SCREEN_W - 500, 70, cameraInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // **ゴールとステージデバッグ情報**
    string stageInfo = "Stage: " + to_string(currentStageIndex + 1) + "/5";
    string goalInfo = "Goal: " + string(goalSystem.IsGoalTouched() ? "TOUCHED!" : "Active") + ", Result: " + string(showingResult ? "SHOWING" : "HIDDEN");
    DrawStringToHandle(30, SCREEN_H - 40, stageInfo.c_str(), GetColor(255, 200, 100), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 20, goalInfo.c_str(), GetColor(100, 255, 200), fontHandle);
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