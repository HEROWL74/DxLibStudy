#include "GoalSystem.h"
#include <math.h>

GoalSystem::GoalSystem()
    : goalX(0), goalY(0)
    , currentColor(FLAG_BLUE)
    , goalState(GOAL_ACTIVE)
    , goalExists(false)
    , animationTimer(0.0f)
    , currentFrame(false)
    , goalTouchTimer(0.0f)
    , glowIntensity(0.0f)
    , bobPhase(0.0f)
{
    // テクスチャハンドルを初期化
    flagTextures.blue_a = flagTextures.blue_b = -1;
    flagTextures.yellow_a = flagTextures.yellow_b = -1;
    flagTextures.green_a = flagTextures.green_b = -1;
    flagTextures.red_a = flagTextures.red_b = -1;
}

GoalSystem::~GoalSystem()
{
    // テクスチャの解放
    if (flagTextures.blue_a != -1) DeleteGraph(flagTextures.blue_a);
    if (flagTextures.blue_b != -1) DeleteGraph(flagTextures.blue_b);
    if (flagTextures.yellow_a != -1) DeleteGraph(flagTextures.yellow_a);
    if (flagTextures.yellow_b != -1) DeleteGraph(flagTextures.yellow_b);
    if (flagTextures.green_a != -1) DeleteGraph(flagTextures.green_a);
    if (flagTextures.green_b != -1) DeleteGraph(flagTextures.green_b);
    if (flagTextures.red_a != -1) DeleteGraph(flagTextures.red_a);
    if (flagTextures.red_b != -1) DeleteGraph(flagTextures.red_b);
}

void GoalSystem::Initialize()
{
    LoadTextures();
    ClearGoal();
}

void GoalSystem::LoadTextures()
{
    // 旗のテクスチャを読み込み
    flagTextures.blue_a = LoadGraph("Sprites/Tiles/flag_blue_a.png");
    flagTextures.blue_b = LoadGraph("Sprites/Tiles/flag_blue_b.png");
    flagTextures.yellow_a = LoadGraph("Sprites/Tiles/flag_yellow_a.png");
    flagTextures.yellow_b = LoadGraph("Sprites/Tiles/flag_yellow_b.png");
    flagTextures.green_a = LoadGraph("Sprites/Tiles/flag_green_a.png");
    flagTextures.green_b = LoadGraph("Sprites/Tiles/flag_green_b.png");
    flagTextures.red_a = LoadGraph("Sprites/Tiles/flag_red_a.png");
    flagTextures.red_b = LoadGraph("Sprites/Tiles/flag_red_b.png");
}

void GoalSystem::Update(Player* player)
{
    if (!goalExists || !player) return;

    // アニメーション更新
    UpdateAnimation();

    // エフェクト更新
    UpdateEffects();

    // プレイヤーとの衝突判定
    if (goalState == GOAL_ACTIVE && CheckPlayerCollision(player)) {
        goalState = GOAL_TOUCHED;
        goalTouchTimer = 0.0f;
    }

    // ゴールタッチ後の処理
    if (goalState == GOAL_TOUCHED) {
        goalTouchTimer += 0.016f; // 60FPS想定

        if (goalTouchTimer >= GOAL_TOUCH_DURATION) {
            goalState = GOAL_COMPLETED;
        }
    }
}

void GoalSystem::Draw(float cameraX)
{
    if (!goalExists) return;

    // ゴールの旗を描画
    DrawGoalFlag(cameraX);

    // エフェクトを描画
    DrawGoalEffects(cameraX);
}

void GoalSystem::UpdateAnimation()
{
    // ゆっくりとした旗のアニメーション
    animationTimer += ANIMATION_SPEED;
    if (animationTimer >= 1.0f) {
        animationTimer = 0.0f;
        currentFrame = !currentFrame; // フレームを切り替え
    }

    // 上下浮遊効果
    bobPhase += BOB_SPEED;
    if (bobPhase >= 2.0f * 3.14159265f) {
        bobPhase = 0.0f;
    }
}

void GoalSystem::UpdateEffects()
{
    // グロー効果の更新
    switch (goalState) {
    case GOAL_ACTIVE:
        // 通常時のゆっくりとしたグロー
        glowIntensity = 0.3f + sinf(bobPhase * 2.0f) * 0.2f;
        break;

    case GOAL_TOUCHED:
    {
        // タッチ時の強いグロー効果
        float progress = goalTouchTimer / GOAL_TOUCH_DURATION;
        glowIntensity = 0.8f + sinf(progress * 20.0f) * 0.2f; // 点滅効果
    }
    break;

    case GOAL_COMPLETED:
        glowIntensity = 1.0f;
        break;
    }
}

bool GoalSystem::CheckPlayerCollision(Player* player)
{
    float playerX = player->GetX();
    float playerY = player->GetY();

    // ゴールとプレイヤーの距離をチェック
    float distance = GetDistance(playerX, playerY, goalX, goalY);

    // ゴール判定範囲内かチェック
    return distance <= (GOAL_DETECTION_WIDTH / 2);
}

void GoalSystem::DrawGoalFlag(float cameraX)
{
    // 画面座標に変換
    int screenX = (int)(goalX - cameraX);
    int screenY = (int)(goalY + sinf(bobPhase) * BOB_AMPLITUDE); // 浮遊効果

    // 画面外なら描画しない
    if (screenX < -FLAG_WIDTH || screenX > 1920 + FLAG_WIDTH) return;

    // 現在のフレームのテクスチャを取得
    int flagTexture = GetCurrentFlagTexture();
    if (flagTexture == -1) return;

    // ゴールタッチ時の特殊効果
    if (goalState == GOAL_TOUCHED) {
        float progress = goalTouchTimer / GOAL_TOUCH_DURATION;
        float scale = 1.0f + sinf(progress * 10.0f) * 0.1f; // 振動効果

        int scaledWidth = (int)(FLAG_WIDTH * scale);
        int scaledHeight = (int)(FLAG_HEIGHT * scale);
        int offsetX = (scaledWidth - FLAG_WIDTH) / 2;
        int offsetY = (scaledHeight - FLAG_HEIGHT) / 2;

        DrawExtendGraph(
            screenX - offsetX, screenY - offsetY,
            screenX + scaledWidth - offsetX, screenY + scaledHeight - offsetY,
            flagTexture, TRUE
        );
    }
    else {
        // 通常描画
        DrawExtendGraph(
            screenX, screenY,
            screenX + (int)FLAG_WIDTH, screenY + (int)FLAG_HEIGHT,
            flagTexture, TRUE
        );
    }
}

void GoalSystem::DrawGoalEffects(float cameraX)
{
    int screenX = (int)(goalX - cameraX);
    int screenY = (int)(goalY + sinf(bobPhase) * BOB_AMPLITUDE);

    // グロー効果
    if (glowIntensity > 0.01f) {
        int glowAlpha = (int)(glowIntensity * 150);
        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);

        // 複数のレイヤーでグロー効果
        for (int i = 0; i < 3; i++) {
            int offset = (i + 1) * 8;
            DrawBox(
                screenX - offset, screenY - offset,
                screenX + (int)FLAG_WIDTH + offset, screenY + (int)FLAG_HEIGHT + offset,
                GetColor(255, 255, 150), FALSE
            );
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // デバッグ用：ゴール判定範囲を表示
#ifdef _DEBUG
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawCircle(screenX + FLAG_WIDTH / 2, screenY + FLAG_HEIGHT / 2,
        (int)(GOAL_DETECTION_WIDTH / 2), GetColor(0, 255, 255), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

int GoalSystem::GetCurrentFlagTexture()
{
    // 現在の色とフレームに応じてテクスチャを返す
    switch (currentColor) {
    case FLAG_BLUE:
        return currentFrame ? flagTextures.blue_b : flagTextures.blue_a;
    case FLAG_YELLOW:
        return currentFrame ? flagTextures.yellow_b : flagTextures.yellow_a;
    case FLAG_GREEN:
        return currentFrame ? flagTextures.green_b : flagTextures.green_a;
    case FLAG_RED:
        return currentFrame ? flagTextures.red_b : flagTextures.red_a;
    default:
        return flagTextures.blue_a;
    }
}

void GoalSystem::SetGoal(float x, float y, FlagColor color)
{
    goalX = x;
    goalY = y;
    currentColor = color;
    goalExists = true;
    goalState = GOAL_ACTIVE;
    goalTouchTimer = 0.0f;
    animationTimer = 0.0f;
    bobPhase = 0.0f;
}

void GoalSystem::ClearGoal()
{
    goalExists = false;
    goalState = GOAL_ACTIVE;
    goalTouchTimer = 0.0f;
}

void GoalSystem::PlaceGoalForStage(int stageIndex, StageManager* stageManager)
{
    ClearGoal();

    // 最後のステージ（4番目）にはゴールを配置しない
    if (stageIndex >= 4) {
        return;
    }

    // ステージに応じた色と位置を設定
    FlagColor stageColors[] = {
        FLAG_BLUE,    // GrassStage
        FLAG_YELLOW,  // StoneStage  
        FLAG_GREEN,   // SandStage
        FLAG_RED      // SnowStage
    };

    // ステージの終端近くにゴールを配置
    float goalWorldX = Stage::STAGE_WIDTH - 400; // ステージの右端から400px手前
    float groundLevel = FindGroundLevel(goalWorldX, stageManager);

    if (groundLevel > 0) {
        // ブロックの上に配置（旗の底がブロックの上に来るように）
        float goalWorldY = groundLevel - FLAG_HEIGHT;
        SetGoal(goalWorldX, goalWorldY, stageColors[stageIndex % 4]);
    }
}

float GoalSystem::FindGroundLevel(float x, StageManager* stageManager)
{
    // 指定された位置での地面レベルを検索
    // 上から下に向かってタイルをチェック
    for (int y = 0; y < Stage::STAGE_HEIGHT; y += Stage::TILE_SIZE) {
        if (stageManager->CheckCollision(x, y, 1, 1)) {
            return (float)y; // 最初に見つかったタイルの位置を返す
        }
    }
    return Stage::GROUND_LEVEL; // デフォルトの地面レベル
}

float GoalSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}