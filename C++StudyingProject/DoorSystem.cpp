#include "DoorSystem.h"
#include <math.h>
#include <algorithm>

DoorSystem::DoorSystem()
    : doorX(0), doorY(0)
    , doorState(DOOR_CLOSED)
    , doorExists(false)
    , openingProgress(0.0f)
    , enteringProgress(0.0f)
    , animationTimer(0.0f)
    , playerStartX(0.0f)
    , playerTargetX(0.0f)
    , playerWalkSpeed(PLAYER_WALK_SPEED)
{
    // テクスチャハンドルを初期化
    doorTextures.closedBottom = doorTextures.closedTop = -1;
    doorTextures.openBottom = doorTextures.openTop = -1;
}

DoorSystem::~DoorSystem()
{
    // テクスチャの解放
    if (doorTextures.closedBottom != -1) DeleteGraph(doorTextures.closedBottom);
    if (doorTextures.closedTop != -1) DeleteGraph(doorTextures.closedTop);
    if (doorTextures.openBottom != -1) DeleteGraph(doorTextures.openBottom);
    if (doorTextures.openTop != -1) DeleteGraph(doorTextures.openTop);
}

void DoorSystem::Initialize()
{
    LoadTextures();
    ClearDoor();
}

void DoorSystem::LoadTextures()
{
    // ドアテクスチャを読み込み
    doorTextures.closedBottom = LoadGraph("Sprites/Tiles/door_closed.png");
    doorTextures.closedTop = LoadGraph("Sprites/Tiles/door_closed_top.png");
    doorTextures.openBottom = LoadGraph("Sprites/Tiles/door_open.png");
    doorTextures.openTop = LoadGraph("Sprites/Tiles/door_open_top.png");
}

void DoorSystem::Update(Player* player)
{
    if (!doorExists || !player) return;

    // アニメーション更新
    UpdateDoorAnimation();

    // プレイヤー進入処理
    if (doorState == DOOR_OPEN) {
        // ドアが開いていてプレイヤーが近くにいる場合
        if (CheckPlayerNearDoor(player)) {
            StartPlayerEntering(player);
        }
    }
    else if (doorState == DOOR_PLAYER_ENTERING) {
        UpdatePlayerEntering(player);
    }
}

void DoorSystem::UpdateDoorAnimation()
{
    animationTimer += 0.016f; // 60FPS想定

    switch (doorState) {
    case DOOR_OPENING:
        openingProgress += 0.016f / OPENING_DURATION;
        if (openingProgress >= 1.0f) {
            openingProgress = 1.0f;
            doorState = DOOR_OPEN;
        }
        break;

    case DOOR_PLAYER_ENTERING:
        enteringProgress += 0.016f / ENTERING_DURATION;
        if (enteringProgress >= 1.0f) {
            enteringProgress = 1.0f;
        }
        break;

    default:
        break;
    }
}
void DoorSystem::UpdatePlayerEntering(Player* player)
{
    if (!player) return;

    // **自動歩行が有効か確認**
    if (!player->IsAutoWalking()) {
        // 自動歩行が無効になっている場合は再有効化
        player->SetAutoWalking(true);
        OutputDebugStringA("DoorSystem: Re-enabled auto walking\n");
    }

    // **ドアまでの距離をチェック**
    float currentX = player->GetX();
    float targetX = doorX + DOOR_WIDTH / 2; // ドアの中央
    float distance = abs(currentX - targetX);

    // **デバッグ出力**
    char debugMsg[256];
    sprintf_s(debugMsg, "DoorSystem: Player X:%.1f, Target X:%.1f, Distance:%.1f\n",
        currentX, targetX, distance);
    OutputDebugStringA(debugMsg);

    // **ドアに十分近づいた場合**
    if (distance <= 30.0f) { // 判定距離を30ピクセルに拡大
        // **自動歩行を停止**
        player->SetAutoWalking(false);

        // **プレイヤーをドアの中央に配置（Y座標は変更しない）**
        player->SetPosition(targetX, player->GetY());

        // ドア進入完了
        enteringProgress = 1.0f;

        OutputDebugStringA("DoorSystem: Player reached door center!\n");
    }
}


bool DoorSystem::CheckPlayerNearDoor(Player* player)
{
    if (!player) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();

    // **ドア検出範囲を拡大**
    float doorCenterX = doorX + DOOR_WIDTH / 2;
    float doorCenterY = doorY + DOOR_HEIGHT / 2;

    // **X方向とY方向を個別にチェック**
    float distanceX = abs(playerX - doorCenterX);
    float distanceY = abs(playerY - doorCenterY);

    // **ドア検出条件を緩和**
    bool nearX = distanceX <= DETECTION_WIDTH * 2.0f; // X方向の検出範囲を拡大
    bool nearY = distanceY <= DOOR_HEIGHT * 1.5f;     // Y方向の検出範囲

    bool isNear = nearX && nearY;

    // **デバッグ出力**
    if (isNear) {
        char debugMsg[256];
        sprintf_s(debugMsg, "DoorSystem: Player near door! X:%.1f, Y:%.1f, DoorX:%.1f, DoorY:%.1f\n",
            playerX, playerY, doorCenterX, doorCenterY);
        OutputDebugStringA(debugMsg);
    }

    return isNear;
}

void DoorSystem::StartPlayerEntering(Player* player)
{
    if (!player) return;

    doorState = DOOR_PLAYER_ENTERING;
    enteringProgress = 0.0f;
    playerStartX = player->GetX();
    playerTargetX = doorX + DOOR_WIDTH / 2;
    animationTimer = 0.0f;
}

void DoorSystem::Draw(float cameraX)
{
    if (!doorExists) return;

    DrawDoorSprites(cameraX);
    DrawDoorEffects(cameraX);
}

void DoorSystem::DrawDoorSprites(float cameraX)
{
    // 画面座標に変換
    int screenX = (int)(doorX - cameraX);
    int screenY = (int)doorY;

    // 画面外なら描画しない
    if (screenX < -DOOR_WIDTH || screenX > 1920 + DOOR_WIDTH) return;

    // ドアの状態に応じてテクスチャを選択
    int bottomTexture = -1;
    int topTexture = -1;

    if (doorState == DOOR_CLOSED) {
        bottomTexture = doorTextures.closedBottom;
        topTexture = doorTextures.closedTop;
    }
    else {
        // 開いている状態（DOOR_OPENING, DOOR_OPEN, DOOR_PLAYER_ENTERING）
        bottomTexture = doorTextures.openBottom;
        topTexture = doorTextures.openTop;
    }

    // 開放アニメーション効果
    float scaleEffect = 1.0f;
    int alphaEffect = 255;

    if (doorState == DOOR_OPENING) {
        // 開放時の演出
        scaleEffect = 1.0f + openingProgress * 0.1f; // 軽い拡大効果
        alphaEffect = 255;
    }

    // 下部ドア描画
    if (bottomTexture != -1) {
        int drawWidth = (int)(DOOR_WIDTH * scaleEffect);
        int drawHeight = (int)(DOOR_WIDTH * scaleEffect); // 64x64想定
        int offsetX = (drawWidth - DOOR_WIDTH) / 2;
        int offsetY = (drawHeight - DOOR_WIDTH) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaEffect);
        DrawExtendGraph(
            screenX - offsetX, screenY + 64 - offsetY,
            screenX - offsetX + drawWidth, screenY + 64 - offsetY + drawHeight,
            bottomTexture, TRUE
        );
    }

    // 上部ドア描画
    if (topTexture != -1) {
        int drawWidth = (int)(DOOR_WIDTH * scaleEffect);
        int drawHeight = (int)(DOOR_WIDTH * scaleEffect);
        int offsetX = (drawWidth - DOOR_WIDTH) / 2;
        int offsetY = (drawHeight - DOOR_WIDTH) / 2;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alphaEffect);
        DrawExtendGraph(
            screenX - offsetX, screenY - offsetY,
            screenX - offsetX + drawWidth, screenY - offsetY + drawHeight,
            topTexture, TRUE
        );
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void DoorSystem::DrawDoorEffects(float cameraX)
{
    int screenX = (int)(doorX - cameraX);
    int screenY = (int)doorY;

    // 開放時のエフェクト
    if (doorState == DOOR_OPENING && openingProgress < 1.0f) {
        // 光るエフェクト
        float glowIntensity = sinf(animationTimer * 8.0f) * 0.3f + 0.7f;
        int glowAlpha = (int)(120 * glowIntensity * (1.0f - openingProgress));

        SetDrawBlendMode(DX_BLENDMODE_ADD, glowAlpha);

        // ドア周辺の光
        for (int i = 0; i < 3; i++) {
            int offset = (i + 1) * 8;
            DrawBox(
                screenX - offset, screenY - offset,
                screenX + (int)DOOR_WIDTH + offset, screenY + (int)DOOR_HEIGHT + offset,
                GetColor(255, 255, 100), FALSE
            );
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // プレイヤー進入時のエフェクト
    if (doorState == DOOR_PLAYER_ENTERING) {
        // やわらかい光のエフェクト
        float enterGlow = sinf(enteringProgress * 3.14159f) * 0.8f;
        int enterAlpha = (int)(80 * enterGlow);

        SetDrawBlendMode(DX_BLENDMODE_ADD, enterAlpha);
        DrawCircle(screenX + DOOR_WIDTH / 2, screenY + DOOR_HEIGHT / 2, 60,
            GetColor(100, 200, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

#ifdef _DEBUG
    // デバッグ用：ドア検出範囲を表示
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawCircle(screenX + DOOR_WIDTH / 2, screenY + DOOR_HEIGHT / 2,
        (int)DETECTION_WIDTH, GetColor(0, 255, 255), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

void DoorSystem::SetDoor(float x, float y)
{
    doorX = x;
    doorY = y;
    doorExists = true;
    doorState = DOOR_CLOSED;
    openingProgress = 0.0f;
    enteringProgress = 0.0f;
    animationTimer = 0.0f;
}

void DoorSystem::ClearDoor()
{
    doorExists = false;
    doorState = DOOR_CLOSED;
    openingProgress = 0.0f;
    enteringProgress = 0.0f;
    animationTimer = 0.0f;
}

void DoorSystem::OpenDoor()
{
    if (!doorExists || doorState != DOOR_CLOSED) return;

    doorState = DOOR_OPENING;
    animationTimer = 0.0f;
    openingProgress = 0.0f;
}

bool DoorSystem::IsPlayerFullyEntered() const
{
    return doorState == DOOR_PLAYER_ENTERING && enteringProgress >= 1.0f;
}

// **修正: ドア配置時の地面レベル検出を改善**
void DoorSystem::PlaceDoorForStage(int stageIndex, StageManager* stageManager)
{
    ClearDoor();

    // ステージ5（インデックス4）以外にドアを配置
    if (stageIndex >= 4) {
        // ステージ5には配置しない
        return;
    }

    // **ステージごとのドア配置位置を個別に設定**
    float doorWorldX;

    switch (stageIndex) {
    case 0: // ステージ1（Grass Stage）
        // 旗の前に配置（旗の位置から少し近前）
        doorWorldX = Stage::STAGE_WIDTH - 500; // 旗より近前
        break;
    case 1: // ステージ2（Stone Stage）
        doorWorldX = Stage::STAGE_WIDTH - 350;
        break;
    case 2: // ステージ3（Sand Stage）
        doorWorldX = Stage::STAGE_WIDTH - 400;
        break;
    case 3: // ステージ4（Snow Stage）
        doorWorldX = Stage::STAGE_WIDTH - 380;
        break;
    default:
        doorWorldX = Stage::STAGE_WIDTH - 400;
        break;
    }

    // **改良された地面検索で正確な地面レベルを取得**
    float groundLevel = FindGroundLevelAccurate(doorWorldX, stageManager);

    if (groundLevel > 0) {
        // **ドアの底部が地面と同じ高さになるように配置**
        // ドアは2タイル分の高さ（128px）なので、上タイルの位置を計算
        float doorWorldY = groundLevel - DOOR_HEIGHT; // 上タイルが地面より128px上に
        SetDoor(doorWorldX, doorWorldY);

        // **デバッグ出力**
        char debugMsg[256];
        sprintf_s(debugMsg, "DoorSystem: Placed door at Stage %d - X:%.1f, Y:%.1f, Ground:%.1f\n",
            stageIndex + 1, doorWorldX, doorWorldY, groundLevel);
        OutputDebugStringA(debugMsg);
    }
}

float DoorSystem::FindGroundLevel(float x, StageManager* stageManager)
{
    // 指定された位置での地面レベルを探索
    // 上から下に向かってタイルをチェック
    for (int y = 0; y < Stage::STAGE_HEIGHT; y += Stage::TILE_SIZE) {
        if (stageManager->CheckCollision(x, y, 1, 1)) {
            return (float)y; // 最初に見つかったタイルの位置を返す
        }
    }
    return Stage::GROUND_LEVEL; // デフォルトの地面レベル
}

// **新追加: より正確な地面検索関数**
float DoorSystem::FindGroundLevelAccurate(float x, StageManager* stageManager)
{
    const int TILE_SIZE = 64; // Stage::TILE_SIZE
    const int MAX_SEARCH_TILES = 20; // 最大20タイル下まで検索

    // ドアの幅の範囲で地面を検索（より安定した配置のため）
    float searchPositions[] = {
        x,                    // 中央
        x + DOOR_WIDTH / 4,   // 右寄り
        x - DOOR_WIDTH / 4    // 左寄り
    };

    // 各位置で地面を検索
    for (float searchX : searchPositions) {
        // 上から下に向かって地面を検索
        for (int tileY = 0; tileY < MAX_SEARCH_TILES; tileY++) {
            float checkY = tileY * TILE_SIZE;

            // この位置に固体ブロックがあるかチェック
            if (stageManager->CheckCollision(searchX, checkY, 8, 8)) {
                // 地面発見：このタイルの上端を返す
                return checkY;
            }
        }
    }

    // 地面が見つからない場合はデフォルト値
    return 12 * TILE_SIZE; // タイル12個分の高さ（768px）
}

float DoorSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}