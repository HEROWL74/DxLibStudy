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

    // 安全な位置制約
    float safeY = y;
    if (safeY < -200) safeY = -200;    // 上限制約
    if (safeY > 1500) safeY = 1500;    // 下限制約

    // アイドル時の上下揺れ効果（安全な範囲で）
    float bobOffset = 0.0f;
    if (currentState == IDLE && onGround) {
        bobOffset = sinf(bobPhase) * 2.0f; // 揺れ幅を制限
    }

    // 安全な画面座標計算
    int screenX = (int)(x - cameraX);
    int screenY = (int)(safeY + bobOffset);

    // 画面外に出すぎた場合の制限
    if (screenX < -500 || screenX > 2500) return; // 描画をスキップ
    if (screenY < -500 || screenY > 1500) return; // 描画をスキップ

    // キャラクターサイズ取得
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    // 安全なサイズ制限
    if (spriteWidth <= 0 || spriteHeight <= 0) return;
    if (spriteWidth > 1000 || spriteHeight > 1000) return; // 異常なサイズを防ぐ

    // 中央揃えで描画位置調整
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // 最終的な安全チェック
    if (screenX < -1000 || screenX > 3000) return;
    if (screenY < -1000 || screenY > 2000) return;

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
    // 改良された水平移動処理（イージング付き）
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

        // 極低の速度は0にする（完全停止）
        if (fabsf(velocityX) < 0.1f) {
            velocityX = 0.0f;
        }

        // 地上での状態決定
        if (onGround && currentState != JUMPING && currentState != FALLING) {
            if (fabsf(velocityX) < 0.5f) {
                currentState = IDLE;
            }
            else {
                currentState = WALKING; // まだ動いている
            }
        }
    }

    // しゃがみ処理
    if (CheckHitKey(KEY_INPUT_DOWN) && onGround) {
        currentState = DUCKING;
        velocityX *= 0.8f; // しゃがみ時はより強い減速
    }

    // 修正：ジャンプ処理（連続ジャンプバグ修正）
    static bool spaceWasPressedLastFrame = false;
    bool spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;

    // ジャンプは地上にいて、かつスペースキーが新しく押された時のみ
    if (spacePressed && !spaceWasPressedLastFrame && onGround && currentState != JUMPING) {
        velocityY = JUMP_POWER;
        currentState = JUMPING;
        onGround = false;
    }

    // 前フレームのスペースキー状態を記録
    spaceWasPressedLastFrame = spacePressed;

    // 修正：重力適用（安全な範囲制限付き）
    if (!onGround) {
        // ジャンプボタンを離した時の早期落下（より自然な操作感）
        if (currentState == JUMPING && !spacePressed && velocityY < 0) {
            velocityY *= 0.7f; // 上昇を早めに止める
        }

        velocityY += GRAVITY;

        // 安全な速度制限
        if (velocityY < -25.0f) {  // 上昇速度の制限
            velocityY = -25.0f;
        }
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }

        // ジャンプから落下への状態変化
        if (velocityY > 0 && currentState == JUMPING) {
            currentState = FALLING;
        }
    }
    else {
        // 地上にいる時は上昇速度をリセット
        if (velocityY < 0) {
            velocityY = 0;
        }
    }
}

void Player::HandleCollisions(StageManager* stageManager)
{
    // プレイヤーの当たり判定サイズ（128x128プレイヤー用に最適化）
    const float COLLISION_WIDTH = 80.0f;   // プレイヤーの実際の幅（少し小さめに）
    const float COLLISION_HEIGHT = 100.0f; // プレイヤーの実際の高さ（少し小さめに）

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
        currentY - height / 2 + 16,  // 上部（少し下）
        currentY,                    // 中央
        currentY + height / 2 - 16   // 下部（少し上）
    };

    for (float checkY : checkPoints) {
        if (CheckPointCollision(newX, checkY, width, 8.0f, stageManager)) {
            return true;
        }
    }
    return false;
}

void Player::HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // 足元の複数点でチェック（128x128プレイヤー用に調整）
    float footY = newY + height / 2;
    float leftFoot = x - width / 3;
    float rightFoot = x + width / 3;
    float centerFoot = x;

    // 3点で地面チェック
    bool leftHit = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

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
    bool leftHit = CheckPointCollision(leftHead, headY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerHead, headY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightHead, headY, 8.0f, 8.0f, stageManager);

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

    for (int i = 0; i < 64; i++) { // 最大64ピクセル調整（1タイル分）
        adjustedX += step;
        if (!CheckXCollision(adjustedX, y, width, 100.0f, stageManager)) {
            return adjustedX;
        }
    }

    return currentX; // 調整できない場合は元の位置
}

float Player::FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // タイルサイズを考慮した正確な地面位置の探索
    int tileSize = Stage::TILE_SIZE; // 64px

    // プレイヤーの足元周辺のタイルを調べる
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY + 4; // 下方向に4タイル分チェック

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * tileSize;

        // プレイヤーの幅で地面をチェック
        float leftX = playerX - width / 2 + 8;
        float rightX = playerX + width / 2 - 8;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY, 8, 8) ||
            stageManager->CheckCollision(centerX, checkY, 8, 8) ||
            stageManager->CheckCollision(rightX, checkY, 8, 8)) {

            // このタイルの上端が地面
            return checkY;
        }
    }

    return -1; // 地面が見つからない
}

float Player::FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager)
{
    // タイルサイズを考慮した正確な天井位置の探索
    int tileSize = Stage::TILE_SIZE; // 64px

    // プレイヤーの頭上周辺のタイルを調べる
    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY - 4; // 上方向に4タイル分チェック

    for (int tileY = startTileY; tileY >= endTileY && tileY >= 0; tileY--) {
        float checkY = (tileY + 1) * tileSize; // タイルの下端

        // プレイヤーの幅で天井をチェック
        float leftX = playerX - width / 2 + 8;
        float rightX = playerX + width / 2 - 8;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY - 8, 8, 8) ||
            stageManager->CheckCollision(centerX, checkY - 8, 8, 8) ||
            stageManager->CheckCollision(rightX, checkY - 8, 8, 8)) {

            // このタイルの下端が天井
            return checkY;
        }
    }

    return -1; // 天井が見つからない
}

bool Player::IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager)
{
    // 地面との接触判定
    float footY = playerY + height / 2 + 4; // 足元よりわずかに下
    float leftFoot = playerX - width / 3;
    float rightFoot = playerX + width / 3;
    float centerFoot = playerX;

    return CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);
}

// ===== 改良された影描画システム =====

void Player::DrawShadow(float cameraX, StageManager* stageManager)
{
    // プレイヤーの足元位置を正確に計算
    float playerFootX = x;
    float playerFootY = y + (PLAYER_HEIGHT / 2) - 8; // 足元より少し上（より自然な位置）

    // 最も近い地面を探索（最適化された検索）
    float groundY = FindOptimalGroundForShadow(playerFootX, playerFootY, stageManager);

    // 地面が見つからない場合は影を描画しない
    if (groundY == -1) return;

    // プレイヤーと地面の距離を計算
    float distanceToGround = groundY - playerFootY;

    // 距離が負の場合（地面より下にいる場合）や遠すぎる場合は影を描画しない
    if (distanceToGround < 0 || distanceToGround > MAX_SHADOW_DISTANCE) return;

    // 影の描画位置（地面上）
    int shadowX = (int)(playerFootX - cameraX);
    int shadowY = (int)groundY - 2; // 地面に少しめり込ませて自然に

    // 画面外チェック（最適化）
    if (shadowX < -100 || shadowX > 1920 + 100) return;
    if (shadowY < -50 || shadowY > 1080 + 50) return;

    // === 影のサイズと透明度の詳細計算 ===

    // 正規化された距離（0.0〜1.0）
    float normalizedDistance = min(distanceToGround / MAX_SHADOW_DISTANCE, 1.0f);

    // プレイヤーの状態による影サイズの調整
    float stateMultiplier = 1.0f;
    switch (currentState) {
    case DUCKING:
        stateMultiplier = 1.2f; // しゃがみ時は少し大きく
        break;
    case JUMPING:
    case FALLING:
        stateMultiplier = 0.9f; // ジャンプ/落下時は少し小さく
        break;
    default:
        stateMultiplier = 1.0f;
        break;
    }

    // 距離による縮小率の計算（より自然な縮小カーブ）
    float distanceMultiplier = 1.0f - powf(normalizedDistance, 1.5f) * 0.8f; // 0.2〜1.0の範囲

    // 最終的な影のサイズ
    float finalSizeMultiplier = stateMultiplier * distanceMultiplier;
    int shadowRadiusX = (int)(BASE_SHADOW_SIZE_X * finalSizeMultiplier);
    int shadowRadiusY = (int)(BASE_SHADOW_SIZE_Y * finalSizeMultiplier);

    // 影の透明度計算（距離とプレイヤー状態を考慮）
    float alphaMultiplier = 1.0f - powf(normalizedDistance, 0.8f) * 0.7f;
    int shadowAlpha = (int)(BASE_SHADOW_ALPHA * alphaMultiplier);

    // 最小サイズと透明度の制限
    if (shadowRadiusX < 6 || shadowRadiusY < 2 || shadowAlpha < 20) return;

    // === 影の詳細描画 ===

    // 影の色（距離に応じてより薄く）
    float colorIntensity = 50.0f * (1.0f - normalizedDistance * 0.6f);
    int shadowColor = GetColor((int)colorIntensity, (int)colorIntensity, (int)colorIntensity);

    // ソフトシャドウ効果（複数レイヤーで描画）
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, shadowAlpha);

    // メインの影
    DrawOval(shadowX, shadowY, shadowRadiusX, shadowRadiusY, shadowColor, TRUE);

    // より濃い中心部（リアリスティックな影）
    if (distanceToGround < MAX_SHADOW_DISTANCE * 0.5f) {
        int innerAlpha = shadowAlpha / 3;
        int innerRadiusX = shadowRadiusX * 2 / 3;
        int innerRadiusY = shadowRadiusY * 2 / 3;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, innerAlpha);
        DrawOval(shadowX, shadowY, innerRadiusX, innerRadiusY, GetColor(20, 20, 20), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // === デバッグ情報 ===
#ifdef _DEBUG
    if (CheckHitKey(KEY_INPUT_F1)) {
        DrawShadowDebugInfo(cameraX, shadowX, shadowY, distanceToGround, normalizedDistance);
    }
#endif
}

void Player::DrawShadowDebugInfo(float cameraX, int shadowX, int shadowY, float distanceToGround, float normalizedDistance)
{
    int playerScreenX = (int)(x - cameraX);
    int playerScreenY = (int)y;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);

    // プレイヤーから影への接続線
    DrawLine(playerScreenX, playerScreenY, shadowX, shadowY, GetColor(255, 255, 0));

    // プレイヤーの足元位置
    int footY = playerScreenY + PLAYER_HEIGHT / 2;
    DrawCircle(playerScreenX, footY, 3, GetColor(0, 255, 0), TRUE);

    // 距離情報の表示
    std::string distInfo = "Ground Dist: " + std::to_string((int)distanceToGround) + "px";
    std::string normalInfo = "Normalized: " + std::to_string(normalizedDistance).substr(0, 4);
    std::string stateInfo = "State: ";

    switch (currentState) {
    case IDLE: stateInfo += "IDLE"; break;
    case WALKING: stateInfo += "WALKING"; break;
    case JUMPING: stateInfo += "JUMPING"; break;
    case FALLING: stateInfo += "FALLING"; break;
    case DUCKING: stateInfo += "DUCKING"; break;
    }

    DrawString(playerScreenX + 15, playerScreenY - 60, distInfo.c_str(), GetColor(255, 255, 0));
    DrawString(playerScreenX + 15, playerScreenY - 40, normalInfo.c_str(), GetColor(255, 255, 0));
    DrawString(playerScreenX + 15, playerScreenY - 20, stateInfo.c_str(), GetColor(255, 255, 0));

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

float Player::FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    // まず高速な基本検索を試行
    float basicGround = FindNearestGroundForShadow(playerX, playerY, stageManager);

    // 基本検索で見つからない場合、より詳細な検索
    if (basicGround == -1) {
        basicGround = FindPreciseGroundForShadow(playerX, playerY, stageManager);
    }

    // それでも見つからない場合、適応的検索
    if (basicGround == -1) {
        basicGround = FindAdaptiveGroundForShadow(playerX, playerY, stageManager);
    }

    return basicGround;
}

float Player::FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const int TILE_SIZE = Stage::TILE_SIZE; // 64px
    const float SEARCH_WIDTH = 60.0f;       // プレイヤーの幅（少し小さめ）
    const int MAX_SEARCH_TILES = 15;        // 最大15タイル下まで探索

    // プレイヤーの足元から下方向に地面を探索
    int startTileY = (int)(playerY / TILE_SIZE);
    int endTileY = startTileY + MAX_SEARCH_TILES;

    // 最も近い地面を探す
    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * TILE_SIZE;

        // プレイヤーの中心と左右で地面をチェック
        bool groundFound = false;

        // 3点でチェック（中央、左、右）
        float checkPositions[] = {
            playerX,                    // 中央
            playerX - SEARCH_WIDTH / 2,   // 左
            playerX + SEARCH_WIDTH / 2    // 右
        };

        for (float checkX : checkPositions) {
            if (stageManager->CheckCollision(checkX, checkY, 4, 4)) {
                groundFound = true;
                break;
            }
        }

        if (groundFound) {
            return checkY; // このタイルの上端が地面
        }
    }

    return -1; // 地面が見つからない
}

float Player::FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const float SEARCH_WIDTH = 50.0f; // より狭い検索幅
    const int MAX_SEARCH_DISTANCE = 960; // 15タイル分（15 * 64px）
    const int SEARCH_STEP = 8; // 8ピクセル単位で検索（高速化）

    // ピクセル単位で下方向に探索
    for (int searchY = (int)playerY; searchY < playerY + MAX_SEARCH_DISTANCE; searchY += SEARCH_STEP) {
        // 5点で地面チェック（より詳細）
        for (int i = 0; i < 5; i++) {
            float checkX = playerX - SEARCH_WIDTH / 2 + (SEARCH_WIDTH * i / 4);

            if (stageManager->CheckCollision(checkX, searchY, 4, 4)) {
                // より正確な地面位置を求める（タイルの上端）
                int tileY = searchY / Stage::TILE_SIZE;
                return (float)(tileY * Stage::TILE_SIZE);
            }
        }
    }

    return -1; // 地面が見つからない
}

float Player::FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const float SEARCH_RADIUS = 30.0f; // プレイヤーの足回りの検索半径
    const int SEARCH_POINTS = 6;       // 円周上の検索点数
    const int MAX_SEARCH_DISTANCE = 480; // 7.5タイル分

    float nearestGroundY = -1;
    float minDistance = (float)(MAX_SEARCH_DISTANCE + 1);

    // プレイヤーの足元周辺の円周上で地面を探索
    for (int i = 0; i < SEARCH_POINTS; i++) {
        float angle = (2.0f * 3.14159265f * i) / SEARCH_POINTS;
        float searchStartX = playerX + cosf(angle) * SEARCH_RADIUS;

        // この位置から下方向に地面を探索
        for (int searchY = (int)playerY; searchY < playerY + MAX_SEARCH_DISTANCE; searchY += 12) {
            if (stageManager->CheckCollision(searchStartX, searchY, 4, 4)) {
                float distance = searchY - playerY;
                if (distance < minDistance) {
                    minDistance = distance;
                    nearestGroundY = searchY;
                }
                break; // この方向での探索は完了
            }
        }
    }

    // タイルの上端に調整
    if (nearestGroundY != -1) {
        int tileY = (int)(nearestGroundY / Stage::TILE_SIZE);
        nearestGroundY = (float)(tileY * Stage::TILE_SIZE);
    }

    return nearestGroundY;
}

// ===== 既存の関数（互換性のため残す） =====

bool Player::CheckCollision(float checkX, float checkY, StageManager* stageManager)
{
    float collisionWidth = 80.0f;
    float collisionHeight = 100.0f;
    float left = checkX - collisionWidth / 2;
    float top = checkY - collisionHeight / 2;
    return stageManager->CheckCollision(left, top, collisionWidth, collisionHeight);
}

float Player::GetGroundY(float checkX, StageManager* stageManager)
{
    float footWidth = 80.0f;
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
    const float COLLISION_WIDTH = 80.0f;
    const float COLLISION_HEIGHT = 100.0f;

    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // 安全な範囲での描画のみ
    if (screenX < -200 || screenX > 2200) return;
    if (screenY < -200 || screenY > 1300) return;

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

    // 詳細情報表示（安全な範囲で）
    std::string debugText =
        "Pos: (" + std::to_string((int)x) + "," + std::to_string((int)y) + ") " +
        "Vel: (" + std::to_string((int)(velocityX * 10)) + "," + std::to_string((int)(velocityY * 10)) + ") " +
        "Ground: " + (onGround ? "YES" : "NO");

    DrawString(screenX - 100, screenY - 140, debugText.c_str(), GetColor(255, 255, 0));

    // 速度の視覚化（制限付き）
    if (fabsf(velocityX) > 0.1f || fabsf(velocityY) > 0.1f) {
        // 速度ベクトルの表示（安全な長さに制限）
        int arrowEndX = screenX + (int)(velocityX * 3); // スケールダウン
        int arrowEndY = screenY + (int)(velocityY * 3); // スケールダウン

        // 矢印の長さを制限
        int maxArrowLength = 50;
        int arrowLengthX = arrowEndX - screenX;
        int arrowLengthY = arrowEndY - screenY;
        int arrowLength = (int)sqrtf(arrowLengthX * arrowLengthX + arrowLengthY * arrowLengthY);

        if (arrowLength > maxArrowLength) {
            float scale = (float)maxArrowLength / arrowLength;
            arrowEndX = screenX + (int)(arrowLengthX * scale);
            arrowEndY = screenY + (int)(arrowLengthY * scale);
        }

        DrawLine(screenX, screenY, arrowEndX, arrowEndY, GetColor(255, 0, 255));
        DrawCircle(arrowEndX, arrowEndY, 3, GetColor(255, 0, 255), TRUE);
    }
}