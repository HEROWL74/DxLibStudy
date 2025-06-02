#include "GameScene.h"
#include <math.h>
using namespace std;

GameScene::GameScene()
    : selectedCharacterIndex(-1)
    , cameraX(0.0f)
    , targetCameraX(0.0f)
    , exitRequested(false)
    , escPressed(false), escPressedPrev(false)
    , stageSelectPressed(false), stageSelectPressedPrev(false)
{
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
    stageManager.LoadStage(StageManager::GRASS);

    // カメラ初期位置
    cameraX = 0.0f;
    targetCameraX = 0.0f;
}

void GameScene::Update()
{
    // 入力更新
    UpdateInput();

    // ステージ切り替え
    if (stageSelectPressed && !stageSelectPressedPrev) {
        static int currentStageIndex = 0;
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        // プレイヤー位置をリセット
        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        targetCameraX = 0.0f;
    }

    // プレイヤー更新
    gamePlayer.Update(&stageManager);

    // カメラ更新
    UpdateCamera();

    // ステージ更新
    stageManager.Update(cameraX);

    // ESCキーでタイトルに戻る
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
    }
}

void GameScene::Draw()
{
    // 背景描画（パララックス効果）
    int bgX = -((int)cameraX / 4);
    DrawExtendGraph(bgX, 0, bgX + SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // ステージ描画
    stageManager.Draw(cameraX);

    // プレイヤー描画
    gamePlayer.Draw(cameraX);

    // プレイヤーの影を描画
    gamePlayer.DrawShadow(cameraX, &stageManager);

    // UI描画
    DrawUI();
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
    // プレイヤーを中心にカメラを追従
    targetCameraX = gamePlayer.GetX() - SCREEN_W / 2.0f;

    // カメラの移動範囲を制限
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;

    // 滑らかなカメラ移動
    cameraX = Lerp(cameraX, targetCameraX, CAMERA_LERP);
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
    DrawBox(20, SCREEN_H - 180, 600, SCREEN_H - 20, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawStringToHandle(30, SCREEN_H - 170, "Controls:", GetColor(255, 255, 255), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 130, "Left/Right: Move, Space: Jump, Down: Duck", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 90, "TAB: Change Stage, ESC: Return to title", GetColor(200, 200, 200), fontHandle);

    // デバッグ情報を強化
    string debugInfo = "=== DEBUG INFO ===";
    DrawStringToHandle(30, SCREEN_H - 160, debugInfo.c_str(), GetColor(255, 255, 0), fontHandle);

    // プレイヤー位置情報
    string posInfo = "Position: (" + to_string((int)gamePlayer.GetX()) + ", " + to_string((int)gamePlayer.GetY()) + ")";
    DrawStringToHandle(30, SCREEN_H - 130, posInfo.c_str(), GetColor(150, 150, 150), fontHandle);

    // 入力状態表示
    string inputInfo = "Input: ";
    if (CheckHitKey(KEY_INPUT_LEFT)) inputInfo += "[LEFT] ";
    if (CheckHitKey(KEY_INPUT_RIGHT)) inputInfo += "[RIGHT] ";
    if (CheckHitKey(KEY_INPUT_SPACE)) inputInfo += "[SPACE] ";
    if (CheckHitKey(KEY_INPUT_DOWN)) inputInfo += "[DOWN] ";
    if (inputInfo == "Input: ") inputInfo += "NONE";
    DrawStringToHandle(30, SCREEN_H - 100, inputInfo.c_str(), GetColor(200, 200, 100), fontHandle);

    // プレイヤーの内部状態
    string stateDetail = "OnGround: " + string(gamePlayer.GetState() != Player::JUMPING && gamePlayer.GetState() != Player::FALLING ? "YES" : "NO");
    DrawStringToHandle(30, SCREEN_H - 70, stateDetail.c_str(), GetColor(100, 200, 100), fontHandle);

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