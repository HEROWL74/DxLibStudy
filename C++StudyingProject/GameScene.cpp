#include "GameScene.h"
#include <math.h>
using namespace std;

GameScene::GameScene()
    : selectedCharacterIndex(-1)
    , playerX(0.0f)
    , playerY(0.0f)
    , velocityX(0.0f)
    , velocityY(0.0f)
    , groundY(800.0f)
    , facingRight(true)
    , currentState(IDLE)
    , onGround(true)
    , animationTimer(0.0f)
    , walkAnimFrame(false)
    , bobPhase(0.0f)
    , exitRequested(false)
    , leftPressed(false), rightPressed(false), downPressed(false), spacePressed(false)
    , leftPressedPrev(false), rightPressedPrev(false), downPressedPrev(false), spacePressedPrev(false)
    , escPressed(false), escPressedPrev(false)
{
    // スプライトハンドルを初期化
    characterSprites.front = -1;
    characterSprites.idle = -1;
    characterSprites.walk_a = -1;
    characterSprites.walk_b = -1;
    characterSprites.jump = -1;
    characterSprites.duck = -1;
    characterSprites.hit = -1;
    characterSprites.climb_a = -1;
    characterSprites.climb_b = -1;
}

GameScene::~GameScene()
{
    DeleteGraph(backgroundHandle);
    DeleteGraph(characterSprites.front);
    DeleteGraph(characterSprites.idle);
    DeleteGraph(characterSprites.walk_a);
    DeleteGraph(characterSprites.walk_b);
    DeleteGraph(characterSprites.jump);
    DeleteGraph(characterSprites.duck);
    DeleteGraph(characterSprites.hit);
    DeleteGraph(characterSprites.climb_a);
    DeleteGraph(characterSprites.climb_b);
    DeleteFontToHandle(fontHandle);
}

void GameScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;

    // 背景とフォント読み込み
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);

    // キャラクター情報設定
    characterColorName = GetCharacterColorName(selectedCharacter);
    characterName = GetCharacterDisplayName(selectedCharacter);

    // 全キャラクタースプライト読み込み
    LoadAllCharacterSprites(selectedCharacter);

    // プレイヤー初期位置設定
    playerX = SCREEN_W / 2.0f;
    playerY = groundY - 100.0f; // 地面より少し上
    velocityX = 0.0f;
    velocityY = 0.0f;
    facingRight = true;
    currentState = IDLE;
    onGround = true;
}

void GameScene::Update()
{
    // 入力更新
    UpdateInput();

    // 物理演算更新
    UpdatePhysics();

    // アニメーション更新
    UpdateAnimation();

    // ESCキーでタイトルに戻る
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
    }
}

void GameScene::Draw()
{
    // 背景描画
    DrawExtendGraph(0, 0, SCREEN_W, SCREEN_H, backgroundHandle, TRUE);

    // ゲームタイトル
    string gameTitle = "GAME WORLD";
    int titleWidth = GetDrawStringWidthToHandle(gameTitle.c_str(), (int)gameTitle.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - titleWidth / 2, 50, gameTitle.c_str(), GetColor(255, 255, 255), fontHandle);

    // 選択されたキャラクター情報表示
    string characterInfo = "Playing as: " + characterName;
    int infoWidth = GetDrawStringWidthToHandle(characterInfo.c_str(), (int)characterInfo.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - infoWidth / 2, 90, characterInfo.c_str(), GetColor(255, 215, 0), fontHandle);

    // 地面の線を描画
    DrawLine(0, (int)groundY, SCREEN_W, (int)groundY, GetColor(100, 100, 100), TRUE);

    // キャラクター描画
    DrawCharacter();

    // 操作説明
    DrawStringToHandle(50, SCREEN_H - 150, "Controls:", GetColor(255, 255, 255), fontHandle);
    DrawStringToHandle(50, SCREEN_H - 110, "Left/Right: Move, Space: Jump, Down: Duck", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(50, SCREEN_H - 70, "Press [ESC] to return to title screen", GetColor(150, 150, 150), fontHandle);

    // デバッグ情報
    string stateInfo = "State: ";
    switch (currentState) {
    case IDLE: stateInfo += "IDLE"; break;
    case WALKING: stateInfo += "WALKING"; break;
    case JUMPING: stateInfo += "JUMPING"; break;
    case DUCKING: stateInfo += "DUCKING"; break;
    }
    stateInfo += ", Direction: " + string(facingRight ? "Right" : "Left");
    DrawStringToHandle(50, 50, stateInfo.c_str(), GetColor(100, 100, 100), fontHandle);
}

void GameScene::UpdateInput()
{
    // 前フレームの入力を保存
    leftPressedPrev = leftPressed;
    rightPressedPrev = rightPressed;
    downPressedPrev = downPressed;
    spacePressedPrev = spacePressed;
    escPressedPrev = escPressed;

    // 現在の入力を取得
    leftPressed = CheckHitKey(KEY_INPUT_LEFT) != 0;
    rightPressed = CheckHitKey(KEY_INPUT_RIGHT) != 0;
    downPressed = CheckHitKey(KEY_INPUT_DOWN) != 0;
    spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;
    escPressed = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
}

void GameScene::UpdatePhysics()
{
    // 水平移動
    velocityX = 0.0f;
    if (leftPressed && currentState != DUCKING) {
        velocityX = -MOVE_SPEED;
        facingRight = false;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else if (rightPressed && currentState != DUCKING) {
        velocityX = MOVE_SPEED;
        facingRight = true;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else if (onGround && !downPressed) {
        currentState = IDLE;
    }

    // しゃがみ
    if (downPressed && onGround) {
        currentState = DUCKING;
        velocityX = 0.0f; // しゃがみ中は移動しない
    }

    // ジャンプ
    if (spacePressed && !spacePressedPrev && onGround) {
        velocityY = JUMP_POWER;
        currentState = JUMPING;
        onGround = false;
    }

    // 重力適用
    if (!onGround) {
        velocityY += GRAVITY;
    }

    // 位置更新
    playerX += velocityX;
    playerY += velocityY;

    // 画面端制限
    if (playerX < 50) playerX = 50;
    if (playerX > SCREEN_W - 50) playerX = SCREEN_W - 50;

    // 地面との当たり判定
    if (playerY >= groundY - 100) {
        playerY = groundY - 100;
        velocityY = 0.0f;
        if (!onGround) {
            onGround = true;
            if (!leftPressed && !rightPressed && !downPressed) {
                currentState = IDLE;
            }
        }
    }
    else {
        onGround = false;
        if (currentState != JUMPING) {
            currentState = JUMPING; // 落下中もJUMPING状態
        }
    }
}

void GameScene::UpdateAnimation()
{
    // 歩行アニメーション
    if (currentState == WALKING) {
        animationTimer += WALK_ANIM_SPEED;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            walkAnimFrame = !walkAnimFrame; // walk_aとwalk_bを切り替え
        }
    }
    else {
        animationTimer = 0.0f;
        walkAnimFrame = false;
    }

    // アイドル時の上下揺れ
    if (currentState == IDLE) {
        bobPhase += 0.03f;
        if (bobPhase >= 2.0f * 3.14159265359f) {
            bobPhase -= 2.0f * 3.14159265359f;
        }
    }
    else {
        bobPhase = 0.0f;
    }
}

int GameScene::GetCurrentSprite()
{
    switch (currentState) {
    case IDLE:
        return characterSprites.idle;
    case WALKING:
        return walkAnimFrame ? characterSprites.walk_b : characterSprites.walk_a;
    case JUMPING:
        return characterSprites.jump;
    case DUCKING:
        return characterSprites.duck;
    default:
        return characterSprites.idle;
    }
}

void GameScene::DrawCharacter()
{
    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // アイドル時の上下揺れ効果
    float bobOffset = (currentState == IDLE) ? sinf(bobPhase) * 3.0f : 0.0f;

    // 描画位置計算
    int drawX = (int)playerX;
    int drawY = (int)(playerY + bobOffset);

    // キャラクターサイズ取得
    int charWidth, charHeight;
    GetGraphSize(currentSprite, &charWidth, &charHeight);

    // 中央揃えのため調整
    drawX -= charWidth / 2;
    drawY -= charHeight / 2;

    // 左右反転描画
    if (facingRight) {
        // 右向き（通常描画）
        DrawGraph(drawX, drawY, currentSprite, TRUE);
    }
    else {
        // 左向き（反転描画）
        DrawTurnGraph(drawX + charWidth, drawY, currentSprite, TRUE);
    }

    // キャラクターの影（簡単な楕円）
    int shadowX = (int)playerX;
    int shadowY = (int)(groundY - 5);
    int shadowRadiusX = charWidth / 3;
    int shadowRadiusY = 8;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawOval(shadowX, shadowY, shadowRadiusX, shadowRadiusY, GetColor(50, 50, 50), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

std::string GameScene::GetCharacterColorName(int index)
{
    switch (index) {
    case BEIGE:  return "beige";
    case GREEN:  return "green";
    case PINK:   return "pink";
    case PURPLE: return "purple";
    case YELLOW: return "yellow";
    default:     return "beige";
    }
}

std::string GameScene::GetCharacterDisplayName(int index)
{
    switch (index) {
    case BEIGE:  return "Beige Knight";
    case GREEN:  return "Green Ranger";
    case PINK:   return "Pink Warrior";
    case PURPLE: return "Purple Mage";
    case YELLOW: return "Yellow Hero";
    default:     return "Unknown Hero";
    }
}

void GameScene::LoadAllCharacterSprites(int characterIndex)
{
    string colorName = GetCharacterColorName(characterIndex);
    string basePath = "Sprites/Characters/character_" + colorName + "_";

    // 全スプライトを読み込み
    characterSprites.front = LoadGraph((basePath + "front.png").c_str());
    characterSprites.idle = LoadGraph((basePath + "idle.png").c_str());
    characterSprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    characterSprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    characterSprites.jump = LoadGraph((basePath + "jump.png").c_str());
    characterSprites.duck = LoadGraph((basePath + "duck.png").c_str());
    characterSprites.hit = LoadGraph((basePath + "hit.png").c_str());
    characterSprites.climb_a = LoadGraph((basePath + "climb_a.png").c_str());
    characterSprites.climb_b = LoadGraph((basePath + "climb_b.png").c_str());
}

float GameScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}