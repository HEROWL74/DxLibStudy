#include "Player.h"
#include <math.h>
#include <algorithm>
#include <cmath>

Player::Player()
    : x(300.0f)
    , y(200.0f)
    , velocityX(0.0f)
    , velocityY(0.0f)
    , facingRight(true)
    , currentState(FALLING)
    , onGround(false)
    , animationTimer(0.0f)
    , walkAnimFrame(false)
    , bobPhase(0.0f)
    , characterIndex(0)
{
    // スプライトハンドルを初期化
    sprites.front = sprites.idle = sprites.walk_a = sprites.walk_b = -1;
    sprites.jump = sprites.duck = sprites.hit = sprites.climb_a = sprites.climb_b = -1;
}

Player::~Player()
{
    // スプライトの解放
    if (sprites.front != -1) DeleteGraph(sprites.front);
    if (sprites.idle != -1) DeleteGraph(sprites.idle);
    if (sprites.walk_a != -1) DeleteGraph(sprites.walk_a);
    if (sprites.walk_b != -1) DeleteGraph(sprites.walk_b);
    if (sprites.jump != -1) DeleteGraph(sprites.jump);
    if (sprites.duck != -1) DeleteGraph(sprites.duck);
    if (sprites.hit != -1) DeleteGraph(sprites.hit);
    if (sprites.climb_a != -1) DeleteGraph(sprites.climb_a);
    if (sprites.climb_b != -1) DeleteGraph(sprites.climb_b);
}

void Player::Initialize(int characterIndex)
{
    this->characterIndex = characterIndex;
    characterColorName = GetCharacterColorName(characterIndex);

    // キャラクタースプライト読み込み
    LoadCharacterSprites(characterIndex);

    // 初期位置設定
    ResetPosition();
}

void Player::Update(StageManager* stageManager)
{
    // 物理演算更新
    UpdatePhysics(stageManager);

    // 当たり判定処理
    HandleCollisions(stageManager);

    // アニメーション更新
    UpdateAnimation();
}

void Player::Draw(float cameraX)
{
    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // アイドル時の上下揺れ効果
    float bobOffset = (currentState == IDLE) ? sinf(bobPhase) * 2.0f : 0.0f;

    // 画面座標計算
    int screenX = (int)(x - cameraX);
    int screenY = (int)(y + bobOffset);

    // キャラクターサイズ取得
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    // 中央揃えで描画位置調整
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // 左右反転描画
    if (facingRight) {
        DrawGraph(screenX, screenY, currentSprite, TRUE);
    }
    else {
        DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
    }
}

void Player::UpdatePhysics(StageManager* stageManager)
{
    // **改良された水平移動処理（イージング付き）**
    bool leftPressed = CheckHitKey(KEY_INPUT_LEFT) && currentState != DUCKING;
    bool rightPressed = CheckHitKey(KEY_INPUT_RIGHT) && currentState != DUCKING;

    if (leftPressed) {
        // 左方向への加速
        velocityX -= ACCELERATION;
        if (velocityX < -MAX_HORIZONTAL_SPEED) {
            velocityX = -MAX_HORIZONTAL_SPEED;
        }
        facingRight = false;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else if (rightPressed) {
        // 右方向への加速
        velocityX += ACCELERATION;
        if (velocityX > MAX_HORIZONTAL_SPEED) {
            velocityX = MAX_HORIZONTAL_SPEED;
        }
        facingRight = true;
        if (onGround) {
            currentState = WALKING;
        }
    }
    else {
        // キー入力がない場合の自然な減速（摩擦）
        velocityX *= FRICTION;

        // 極遅の速度は0にする（完全停止）
        if (fabsf(velocityX) < 0.1f) {
            velocityX = 0.0f;
        }

        // 地上での状態決定
        if (onGround && currentState != JUMPING && currentState != FALLING) {
            if (fabsf(velocityX) < 0.5f) {
                currentState = IDLE;
            }
            else {
                currentState = WALKING; // まだ滑っている
            }
        }
    }

    // しゃがみ処理
    if (CheckHitKey(KEY_INPUT_DOWN) && onGround) {
        currentState = DUCKING;
        velocityX *= 0.8f; // しゃがみ時はより強い減速
    }

    // ジャンプ処理（ふわっとした感じに調整）
    static bool spaceWasPressed = false;
    bool spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;

    if (spacePressed && !spaceWasPressed && onGround) {
        velocityY = JUMP_POWER;
        currentState = JUMPING;
        onGround = false;
    }
    spaceWasPressed = spacePressed;

    // 重力適用（ふわっとした感じに調整）
    if (!onGround) {
        // ジャンプボタンを離したときの早期落下（より自然な操作感）
        if (currentState == JUMPING && !spacePressed && velocityY < 0) {
            velocityY *= 0.7f; // 上昇を早めに止める
        }

        velocityY += GRAVITY;

        // 空気抵抗によるふわっと感の演出
        if (velocityY < 0) { // 上昇中
            velocityY *= AIR_RESISTANCE;
        }

        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }

        // ジャンプから落下への状態変化
        if (velocityY > 0 && currentState == JUMPING) {
            currentState = FALLING;
        }
    }
}

void Player::HandleCollisions(StageManager* stageManager)
{
    // プレイヤーの実際の衝突判定サイズ
    // タイルサイズ(64px)に対して適切なサイズに設定
    const float COLLISION_WIDTH = 48.0f;   // タイルサイズの3/4
    const float COLLISION_HEIGHT = 60.0f;  // タイルサイズより少し小さく

    // ===== X方向の移動と衝突判定 =====
    float newX = x + velocityX;

    // ステージ境界チェック
    if (newX - COLLISION_WIDTH / 2 < 0) {
        x = COLLISION_WIDTH / 2;
        velocityX = 0.0f;
    }
    else if (newX + COLLISION_WIDTH / 2 > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - COLLISION_WIDTH / 2;
        velocityX = 0.0f;
    }
    else {
        // X方向の詳細な衝突チェック
        if (CheckXCollision(newX, y, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager)) {
            // 壁にぶつかった場合、ピクセル単位で調整
            velocityX = 0.0f;
            newX = AdjustXPosition(x, velocityX > 0, COLLISION_WIDTH, stageManager);
        }
        x = newX;
    }

    // ===== Y方向の移動と衝突判定 =====
    float newY = y + velocityY;

    if (velocityY > 0) {
        // 下方向移動（落下）
        HandleDownwardMovement(newY, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager);
    }
    else if (velocityY < 0) {
        // 上方向移動（ジャンプ）
        HandleUpwardMovement(newY, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager);
    }
    else {
        // Y方向の速度が0の場合、地面チェック
        if (onGround && !IsOnGround(x, y, COLLISION_WIDTH, COLLISION_HEIGHT, stageManager)) {
            onGround = false;
            currentState = FALLING;
        }
        y = newY;
    }

    // 画面外落下の救済
    if (y > 1400) {
        ResetPosition();
    }
}

bool Player::CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager)
{
    // プレイヤーの上部、中央、下部の3点でチェック
    float checkPoints[] = {
        currentY - height / 2 + 8,  // 上部（少し下）
        currentY,                   // 中央
        currentY + height / 2 - 8   // 下部（少し上）
    };

    for (float checkY : checkPoints) {
        if (CheckPointCollision(newX, checkY, width, 4.0f, stageManager)) {
            return true;
        }
    }
    return false;
}

void Player::HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // 足元の複数点でチェック
    float footY = newY + height / 2;
    float leftFoot = x - width / 3;
    float rightFoot = x + width / 3;
    float centerFoot = x;

    // 3点で地面チェック
    bool leftHit = CheckPointCollision(leftFoot, footY, 4.0f, 4.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 4.0f, 4.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 4.0f, 4.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // 地面に着地
        float groundY = FindPreciseGroundY(x, newY, width, stageManager);
        if (groundY != -1) {
            y = groundY - height / 2;
            velocityY = 0.0f;
            onGround = true;

            // 着地後の状態決定
            if (CheckHitKey(KEY_INPUT_DOWN)) {
                currentState = DUCKING;
            }
            else if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                currentState = WALKING;
            }
            else {
                currentState = IDLE;
            }
        }
    }
    else {
        // 自由落下
        y = newY;
        if (onGround) {
            onGround = false;
            if (currentState != JUMPING) {
                currentState = FALLING;
            }
        }
    }
}

void Player::HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // 頭上の複数点でチェック
    float headY = newY - height / 2;
    float leftHead = x - width / 3;
    float rightHead = x + width / 3;
    float centerHead = x;

    // 3点で天井チェック
    bool leftHit = CheckPointCollision(leftHead, headY, 4.0f, 4.0f, stageManager);
    bool centerHit = CheckPointCollision(centerHead, headY, 4.0f, 4.0f, stageManager);
    bool rightHit = CheckPointCollision(rightHead, headY, 4.0f, 4.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // 天井にぶつかった
        float ceilingY = FindPreciseCeilingY(x, newY, width, stageManager);
        if (ceilingY != -1) {
            y = ceilingY + height / 2;
            velocityY = 0.0f;  // 上方向の速度を完全に停止
            currentState = FALLING;
            onGround = false;
        }
    }
    else {
        // 自由移動
        y = newY;
    }
}

bool Player::CheckPointCollision(float centerX, float centerY, float width, float height, StageManager* stageManager)
{
    float left = centerX - width / 2;
    float top = centerY - height / 2;
    return stageManager->CheckCollision(left, top, width, height);
}

float Player::AdjustXPosition(float currentX, bool movingRight, float width, StageManager* stageManager)
{
    // ピクセル単位で調整して、壁にめり込まない位置を見つける
    float adjustedX = currentX;
    float step = movingRight ? -1.0f : 1.0f;

    for (int i = 0; i < 32; i++) { // 最大32ピクセル調整
        adjustedX += step;
        if (!CheckXCollision(adjustedX, y, width, 60.0f, stageManager)) {
            return adjustedX;
        }
    }

    return currentX; // 調整できない場合は元の位置
}

float Player::FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // タイルサイズを考慮した正確な地面位置の検索
    int tileSize = Stage::TILE_SIZE; // 64px

    // プレイヤーの足元周辺のタイルを調べる
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY + 3; // 下方向に3タイル分チェック

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * tileSize;

        // プレイヤーの幅で地面をチェック
        float leftX = playerX - width / 2 + 4;
        float rightX = playerX + width / 2 - 4;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY, 4, 4) ||
            stageManager->CheckCollision(centerX, checkY, 4, 4) ||
            stageManager->CheckCollision(rightX, checkY, 4, 4)) {

            // このタイルの上端が地面
            return checkY;
        }
    }

    return -1; // 地面が見つからない
}

float Player::FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // タイルサイズを考慮した正確な天井位置の検索
    int tileSize = Stage::TILE_SIZE; // 64px

    // プレイヤーの頭上周辺のタイルを調べる
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY - 3; // 上方向に3タイル分チェック

    for (int tileY = startTileY; tileY >= endTileY && tileY >= 0; tileY--) {
        float checkY = (tileY + 1) * tileSize; // タイルの下端

        // プレイヤーの幅で天井をチェック
        float leftX = playerX - width / 2 + 4;
        float rightX = playerX + width / 2 - 4;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY - 4, 4, 4) ||
            stageManager->CheckCollision(centerX, checkY - 4, 4, 4) ||
            stageManager->CheckCollision(rightX, checkY - 4, 4, 4)) {

            // このタイルの下端が天井
            return checkY;
        }
    }

    return -1; // 天井が見つからない
}

bool Player::IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager)
{
    // 地面との接触判定
    float footY = playerY + height / 2 + 2; // 足元より少し下
    float leftFoot = playerX - width / 3;
    float rightFoot = playerX + width / 3;
    float centerFoot = playerX;

    return CheckPointCollision(leftFoot, footY, 4.0f, 4.0f, stageManager) ||
        CheckPointCollision(centerFoot, footY, 4.0f, 4.0f, stageManager) ||
        CheckPointCollision(rightFoot, footY, 4.0f, 4.0f, stageManager);
}

// ===== 既存の関数（互換性のため残す） =====

bool Player::CheckCollision(float checkX, float checkY, StageManager* stageManager)
{
    float collisionWidth = 48.0f;
    float collisionHeight = 60.0f;
    float left = checkX - collisionWidth / 2;
    float top = checkY - collisionHeight / 2;
    return stageManager->CheckCollision(left, top, collisionWidth, collisionHeight);
}

float Player::GetGroundY(float checkX, StageManager* stageManager)
{
    float footWidth = 48.0f;
    return stageManager->GetGroundY(checkX - footWidth / 2, footWidth);
}

bool Player::CheckCollisionRect(float checkX, float checkY, float width, float height, StageManager* stageManager)
{
    float left = checkX - width / 2;
    float top = checkY - height / 2;
    return stageManager->CheckCollision(left, top, width, height);
}

int Player::FindGroundTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager)
{
    float groundY = FindPreciseGroundY(playerX, playerY, playerWidth, stageManager);
    return (groundY != -1) ? (int)groundY : -1;
}

int Player::FindCeilingTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager)
{
    float ceilingY = FindPreciseCeilingY(playerX, playerY, playerWidth, stageManager);
    return (ceilingY != -1) ? (int)ceilingY : -1;
}

// ===== その他の既存関数 =====

void Player::UpdateAnimation()
{
    if (currentState == WALKING) {
        animationTimer += WALK_ANIM_SPEED;
        if (animationTimer >= 1.0f) {
            animationTimer = 0.0f;
            walkAnimFrame = !walkAnimFrame;
        }
    }
    else {
        animationTimer = 0.0f;
        walkAnimFrame = false;
    }

    if (currentState == IDLE) {
        bobPhase += 0.02f;
        if (bobPhase >= 2.0f * 3.14159265359f) {
            bobPhase = 0.0f;
        }
    }
}

int Player::GetCurrentSprite()
{
    switch (currentState) {
    case IDLE: return sprites.idle;
    case WALKING: return walkAnimFrame ? sprites.walk_b : sprites.walk_a;
    case JUMPING: return sprites.jump;
    case FALLING: return sprites.jump;
    case DUCKING: return sprites.duck;
    default: return sprites.idle;
    }
}

void Player::LoadCharacterSprites(int characterIndex)
{
    std::string colorName = GetCharacterColorName(characterIndex);
    std::string basePath = "Sprites/Characters/character_" + colorName + "_";

    sprites.front = LoadGraph((basePath + "front.png").c_str());
    sprites.idle = LoadGraph((basePath + "idle.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.jump = LoadGraph((basePath + "jump.png").c_str());
    sprites.duck = LoadGraph((basePath + "duck.png").c_str());
    sprites.hit = LoadGraph((basePath + "hit.png").c_str());
    sprites.climb_a = LoadGraph((basePath + "climb_a.png").c_str());
    sprites.climb_b = LoadGraph((basePath + "climb_b.png").c_str());
}

std::string Player::GetCharacterColorName(int index)
{
    switch (index) {
    case 0: return "beige";
    case 1: return "green";
    case 2: return "pink";
    case 3: return "purple";
    case 4: return "yellow";
    default: return "beige";
    }
}

void Player::DrawShadow(float cameraX, StageManager* stageManager)
{
    int shadowX = (int)(x - cameraX);
    int shadowY = (int)(y + PLAYER_HEIGHT / 2 + 5);

    int shadowRadiusX = 20;
    int shadowRadiusY = 8;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
    DrawOval(shadowX, shadowY, shadowRadiusX, shadowRadiusY, GetColor(30, 30, 30), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void Player::SetPosition(float newX, float newY)
{
    x = newX;
    y = newY;
}

void Player::ResetPosition()
{
    x = 300.0f;
    y = 200.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    currentState = FALLING;
    onGround = false;
    facingRight = true;
}

void Player::DrawDebugInfo(float cameraX)
{
    const float COLLISION_WIDTH = 48.0f;
    const float COLLISION_HEIGHT = 60.0f;

    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // 衝突判定ボックス表示
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(
        screenX - COLLISION_WIDTH / 2,
        screenY - COLLISION_HEIGHT / 2,
        screenX + COLLISION_WIDTH / 2,
        screenY + COLLISION_HEIGHT / 2,
        GetColor(255, 0, 0), FALSE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // タイルグリッド表示
    int tileSize = Stage::TILE_SIZE;
    int startTileX = (int)((x - cameraX - 200) / tileSize) * tileSize;
    int endTileX = (int)((x - cameraX + 200) / tileSize) * tileSize;
    int startTileY = (int)((y - 200) / tileSize) * tileSize;
    int endTileY = (int)((y + 200) / tileSize) * tileSize;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    for (int tx = startTileX; tx <= endTileX; tx += tileSize) {
        for (int ty = startTileY; ty <= endTileY; ty += tileSize) {
            int screenTileX = tx - (int)cameraX;
            DrawBox(screenTileX, ty, screenTileX + tileSize, ty + tileSize, GetColor(0, 255, 0), FALSE);
        }
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // **詳細情報表示（イージング情報含む）**
    std::string debugText =
        "Pos: (" + std::to_string((int)x) + "," + std::to_string((int)y) + ") " +
        "Vel: (" + std::to_string(velocityX) + "," + std::to_string(velocityY) + ") " +
        "Ground: " + (onGround ? "YES" : "NO");
    DrawString(screenX - 100, screenY - 140, debugText.c_str(), GetColor(255, 255, 0));

    // **速度の視覚化（矢印で表示）**
    if (fabsf(velocityX) > 0.1f || fabsf(velocityY) > 0.1f) {
        int arrowEndX = screenX + (int)(velocityX * 5);
        int arrowEndY = screenY + (int)(velocityY * 5);
        DrawLine(screenX, screenY, arrowEndX, arrowEndY, GetColor(255, 0, 255));
        DrawCircle(arrowEndX, arrowEndY, 3, GetColor(255, 0, 255), TRUE);
    }
}