#include "Player.h"
#include <math.h>
#include <algorithm>
#include <cmath>
#include "SoundManager.h"

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
    , wasStomping(false)
    , stompCooldown(0.0f)
    , slidingTimer(0.0f)          // **新追加**
    , slidingSpeed(0.0f)          // **新追加**
    , wasRunningWhenSlideStarted(false) // **新追加**
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
    // **踏みつけクールダウンの更新**
    if (stompCooldown > 0.0f) {
        stompCooldown -= 0.016f; // 60FPS想定
        if (stompCooldown <= 0.0f) {
            wasStomping = false;
        }
    }

    // **ダメージ状態の更新**
    UpdateDamageState();

    // **無敵時間の更新**
    UpdateInvulnerability();

    // 物理演算更新
    UpdatePhysics(stageManager);

    // 当たり判定処理
    HandleCollisions(stageManager);

    // アニメーション更新
    UpdateAnimation();
}

// **Draw関数の修正: スライディング時の特別な描画効果**
void Player::Draw(float cameraX)
{
    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // 安全な位置制約
    float safeY = y;
    if (safeY < -200) safeY = -200;
    if (safeY > 1500) safeY = 1500;

    // アイドル時の上下揺れ効果（安全な範囲で）
    float bobOffset = 0.0f;
    if (currentState == IDLE && onGround) {
        bobOffset = sinf(bobPhase) * 2.0f;
    }

    // **スライディング時の特別な効果**
    float slidingOffset = 0.0f;
    if (currentState == SLIDING) {
        // 微細な振動効果でスライディングの勢いを表現
        float slidingProgress = GetSlidingProgress();
        float vibrationIntensity = (1.0f - slidingProgress) * 1.5f;
        slidingOffset = sinf(slidingTimer * 40.0f) * vibrationIntensity;
    }

    // **ダメージ時の自然なノックバック振動効果**
    float hitShake = 0.0f;
    if (currentState == HIT && hitTimer < HIT_DURATION * 0.7f) {
        // より自然な振動パターン
        float shakeProgress = hitTimer / (HIT_DURATION * 0.7f);
        float shakeIntensity = (1.0f - shakeProgress) * 2.5f; // 徐々に減衰
        hitShake = sinf(hitTimer * 35.0f) * shakeIntensity;
    }

    // 安全な画面座標計算
    int screenX = (int)(x - cameraX + hitShake + slidingOffset);
    int screenY = (int)(safeY + bobOffset);

    // 画面外チェック
    if (screenX < -500 || screenX > 2500) return;
    if (screenY < -500 || screenY > 1500) return;

    // キャラクターサイズ取得
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    if (spriteWidth <= 0 || spriteHeight <= 0) return;
    if (spriteWidth > 1000 || spriteHeight > 1000) return;

    // 中央揃えで描画位置調整
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // 最終的な安全チェック
    if (screenX < -1000 || screenX > 3000) return;
    if (screenY < -1000 || screenY > 2000) return;

    // **無敵時間中の点滅効果**
    bool shouldDraw = true;
    if (invulnerabilityTimer > 0.0f) {
        // 点滅間隔を調整（速い点滅）
        float blinkSpeed = 12.0f;
        shouldDraw = ((int)(invulnerabilityTimer * blinkSpeed) % 2 == 0);
    }

    if (shouldDraw) {
        // **スライディング時の特別な色効果**
        if (currentState == SLIDING) {
            float slidingProgress = GetSlidingProgress();
            int blueIntensity = 255 + (int)(50 * sinf(slidingTimer * 8.0f) * (1.0f - slidingProgress));
            if (blueIntensity > 255) blueIntensity = 255;
            SetDrawBright(255, 255, blueIntensity); // 青みがかった効果
        }

        // **ダメージ時の赤色効果（より自然に）**
        else if (currentState == HIT && hitTimer < HIT_DURATION * 0.4f) {
            float flashProgress = hitTimer / (HIT_DURATION * 0.4f);
            int redIntensity = (int)(255 * (1.0f - flashProgress));
            SetDrawBright(255, 255 - redIntensity / 2, 255 - redIntensity); // 赤みがかった色
        }

        // 左右反転描画
        if (facingRight) {
            DrawGraph(screenX, screenY, currentSprite, TRUE);
        }
        else {
            DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
        }

        // 色をリセット
        if (currentState == SLIDING || (currentState == HIT && hitTimer < HIT_DURATION * 0.4f)) {
            SetDrawBright(255, 255, 255);
        }
    }

    // **スライディング時の煙エフェクト（オプション）**
    if (currentState == SLIDING && onGround) {
        DrawSlidingEffect(cameraX);
    }
}


void Player::UpdatePhysics(StageManager* stageManager)
{
    // **自動歩行モード専用の処理**
    if (isAutoWalking) {
        UpdateAutoWalkPhysics(stageManager);
        return; // 通常の物理処理をスキップ
    }

    // **ダメージ状態中は入力を制限**
    bool canControl = (currentState != HIT || hitTimer > HIT_DURATION * 0.6f);

    // **スライディング状態の更新**
    if (currentState == SLIDING) {
        UpdateSliding();

        // スライディング終了判定
        if (slidingTimer >= SLIDING_DURATION || slidingSpeed <= SLIDING_MIN_SPEED) {
            EndSliding();
        }

        // スライディング中の重力適用
        if (!onGround) {
            velocityY += GRAVITY;
            if (velocityY > MAX_FALL_SPEED) {
                velocityY = MAX_FALL_SPEED;
            }
        }

        return; // 通常の移動処理をスキップ
    }

    // **改良された水平移動処理（段階的加速度システム）**
    bool leftPressed = canControl && CheckHitKey(KEY_INPUT_LEFT);
    bool rightPressed = canControl && CheckHitKey(KEY_INPUT_RIGHT);
    bool downPressed = canControl && CheckHitKey(KEY_INPUT_DOWN);

    // **スライディング開始判定（修正版）**
    static bool downWasPressedLastFrame = false;
    bool downJustPressed = downPressed && !downWasPressedLastFrame;

    // スライディング条件：下キーを押した瞬間 + 地上 + 十分な速度
    if (downJustPressed && onGround && fabsf(velocityX) > 4.0f && currentState != DUCKING) {
        StartSliding();
        downWasPressedLastFrame = downPressed;
        return; // スライディング開始後は通常処理をスキップ
    }
    downWasPressedLastFrame = downPressed;

    // **段階的加速度システム（リアルな移動感）**
    float currentSpeed = fabsf(velocityX);
    float targetSpeed = 0.0f;
    float accelerationRate = ACCELERATION;

    if (leftPressed && !downPressed) { // 下キー押下中は通常移動しない
        targetSpeed = -MAX_HORIZONTAL_SPEED;

        // 速度に応じた加速度調整
        if (currentSpeed < 2.0f) {
            accelerationRate = ACCELERATION * 1.5f; // 低速時は素早く加速
        }
        else if (currentSpeed < 5.0f) {
            accelerationRate = ACCELERATION; // 中速時は標準加速
        }
        else {
            accelerationRate = ACCELERATION * 0.7f; // 高速時は緩やか
        }

        velocityX -= accelerationRate;
        if (velocityX < targetSpeed) {
            velocityX = targetSpeed;
        }
        facingRight = false;

        if (onGround && currentState != HIT) {
            currentState = (currentSpeed > 1.0f) ? WALKING : IDLE;
        }
    }
    else if (rightPressed && !downPressed) { // 下キー押下中は通常移動しない
        targetSpeed = MAX_HORIZONTAL_SPEED;

        // 速度に応じた加速度調整
        if (currentSpeed < 2.0f) {
            accelerationRate = ACCELERATION * 1.5f; // 低速時は素早く加速
        }
        else if (currentSpeed < 5.0f) {
            accelerationRate = ACCELERATION; // 中速時は標準加速
        }
        else {
            accelerationRate = ACCELERATION * 0.7f; // 高速時は緩やか
        }

        velocityX += accelerationRate;
        if (velocityX > targetSpeed) {
            velocityX = targetSpeed;
        }
        facingRight = true;

        if (onGround && currentState != HIT) {
            currentState = (currentSpeed > 1.0f) ? WALKING : IDLE;
        }
    }
    else {
        // **改良された減速システム**
        float currentFriction = FRICTION;

        if (currentState == HIT && knockbackDecay > 0.0f) {
            // ノックバック中は段階的に減衰
            float decayStage = 1.0f - knockbackDecay;
            if (decayStage < 0.3f) {
                currentFriction = 0.95f; // 初期は緩やか
            }
            else if (decayStage < 0.7f) {
                currentFriction = 0.88f; // 中期は標準的
            }
            else {
                currentFriction = 0.82f; // 後期は早めに止める
            }
        }
        else {
            // 通常時の速度に応じた摩擦調整
            if (currentSpeed > 6.0f) {
                currentFriction = 0.92f; // 高速時はゆっくり減速
            }
            else if (currentSpeed > 3.0f) {
                currentFriction = 0.88f; // 中速時は標準減速
            }
            else {
                currentFriction = 0.82f; // 低速時は早めに停止
            }
        }

        velocityX *= currentFriction;

        // 完全停止の閾値
        if (fabsf(velocityX) < 0.3f) {
            velocityX = 0.0f;
        }

        // 状態の更新
        if (onGround && currentState != JUMPING && currentState != FALLING && currentState != HIT) {
            if (fabsf(velocityX) < 0.5f) {
                currentState = IDLE;
            }
            else {
                currentState = WALKING;
            }
        }
    }

    // **しゃがみ処理（修正版）**
    if (canControl && downPressed && onGround && currentState != HIT && currentState != SLIDING) {
        // 移動中でない場合のみしゃがみ可能
        if (fabsf(velocityX) < 1.0f && !leftPressed && !rightPressed) {
            currentState = DUCKING;
            velocityX *= 0.6f; // より強い減速
        }
        // 移動中の場合は何もしない（スライディング判定は上で処理済み）
    }

    // **ジャンプ処理（ダメージ中は無効）**
    static bool spaceWasPressedLastFrame = false;
    bool spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;

    if (canControl && spacePressed && !spaceWasPressedLastFrame && onGround &&
        currentState != JUMPING && currentState != HIT && currentState != SLIDING) {

        // 速度に応じたジャンプ力調整
        float jumpPower = JUMP_POWER;
        if (fabsf(velocityX) > 6.0f) {
            jumpPower *= 1.1f; // 高速移動中はより高いジャンプ
        }

        velocityY = jumpPower;
        currentState = JUMPING;
        onGround = false;
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_JUMP);
    }

    spaceWasPressedLastFrame = spacePressed;

    // **重力適用**
    if (!onGround) {
        // 可変ジャンプ（スペースキーを離すと早く落下）
        if (currentState == JUMPING && !spacePressed && velocityY < 0) {
            velocityY *= 0.65f; // より強い減衰
        }

        velocityY += GRAVITY;

        // 上昇速度制限
        if (velocityY < -25.0f) {
            velocityY = -25.0f;
        }
        // 落下速度制限
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }

        // ジャンプから落下への状態変更
        if (velocityY > 0 && currentState == JUMPING) {
            currentState = FALLING;
        }
    }
    else {
        // 地上では下向きの速度をリセット
        if (velocityY > 0) {
            velocityY = 0;
        }
    }
}

// **HandleCollisions関数の修正: スライディング時の当たり判定調整**
void Player::HandleCollisions(StageManager* stageManager)
{
    // **スライディング時は当たり判定の高さを調整**
    const float COLLISION_WIDTH = 80.0f;
    float collisionHeight = GetSlidingCollisionHeight(); // スライディング対応

    // ===== X方向の移動と衝突判定 =====
    float newX = x + velocityX;

    // ステージ境界チェック
    if (newX - COLLISION_WIDTH / 2 < 0) {
        x = COLLISION_WIDTH / 2;
        velocityX = 0.0f;

        // **スライディング中に壁にぶつかった場合は強制終了**
        if (currentState == SLIDING) {
            EndSliding();
        }
    }
    else if (newX + COLLISION_WIDTH / 2 > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - COLLISION_WIDTH / 2;
        velocityX = 0.0f;

        // **スライディング中に壁にぶつかった場合は強制終了**
        if (currentState == SLIDING) {
            EndSliding();
        }
    }
    else {
        // X方向の詳細な衝突チェック
        if (CheckXCollision(newX, y, COLLISION_WIDTH, collisionHeight, stageManager)) {
            // 壁にぶつかった場合、ピクセル単位で調整
            velocityX = 0.0f;
            newX = AdjustXPosition(x, velocityX > 0, COLLISION_WIDTH, stageManager);

            // **スライディング中に壁にぶつかった場合は強制終了**
            if (currentState == SLIDING) {
                EndSliding();
            }
        }
        x = newX;
    }

    // ===== Y方向の移動と衝突判定 =====
    float newY = y + velocityY;

    if (velocityY > 0) {
        // 下方向移動（落下）
        HandleDownwardMovement(newY, COLLISION_WIDTH, collisionHeight, stageManager);
    }
    else if (velocityY < 0) {
        // 上方向移動（ジャンプ）
        HandleUpwardMovement(newY, COLLISION_WIDTH, collisionHeight, stageManager);
    }
    else {
        // Y方向の速度が0の場合、地面チェック
        if (onGround && !IsOnGround(x, y, COLLISION_WIDTH, collisionHeight, stageManager)) {
            onGround = false;
            if (currentState != SLIDING) { // スライディング中は状態変更しない
                currentState = FALLING;
            }
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

// **修正されたHandleDownwardMovement: スライディング対応**
void Player::HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    // **スライディング中は特別な当たり判定高さを使用**
    float collisionHeight = (currentState == SLIDING) ? GetSlidingCollisionHeight() : height;

    // 足元の複数点でチェック（128x128プレイヤー用に調整）
    float footY = newY + collisionHeight / 2;
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
            // **スライディング中は正しい位置に配置**
            y = groundY - collisionHeight / 2;
            velocityY = 0.0f;
            onGround = true;

            // 着地後の状態決定
            if (currentState == SLIDING) {
                // スライディング中は状態を維持
                return;
            }
            else if (CheckHitKey(KEY_INPUT_DOWN)) {
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
            if (currentState != JUMPING && currentState != SLIDING) {
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

// **GetCurrentSprite関数の修正: HITスプライト対応**
int Player::GetCurrentSprite()
{
    switch (currentState) {
    case IDLE: return sprites.idle;
    case WALKING: return walkAnimFrame ? sprites.walk_b : sprites.walk_a;
    case JUMPING: return sprites.jump;
    case FALLING: return sprites.jump;
    case DUCKING: return sprites.duck;
    case SLIDING: return sprites.duck; // **スライディング時はしゃがみスプライトを使用**
    case HIT: return sprites.hit;
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

// **ResetPosition関数の修正: スライディング状態もリセット**
void Player::ResetPosition()
{
    x = 300.0f;
    y = 200.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    currentState = FALLING;
    onGround = false;
    facingRight = true;

    // ダメージ関連の状態もリセット
    hitTimer = 0.0f;
    invulnerabilityTimer = 0.0f;
    knockbackDecay = 0.0f;

    // **スライディング状態もリセット**
    slidingTimer = 0.0f;
    slidingSpeed = 0.0f;
    wasRunningWhenSlideStarted = false;
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

// **踏みつけ小ジャンプ効果（マリオ風）**
void Player::ApplyStompBounce(float bounceVelocity)
{
    // デフォルト値を使用してマリオ風の小ジャンプ
    velocityY = (bounceVelocity != -8.0f) ? bounceVelocity : -12.0f; // より高いジャンプ
    currentState = JUMPING;
    onGround = false;
    wasStomping = true;
    stompCooldown = STOMP_COOLDOWN_DURATION;

    // **踏みつけ効果音の再生（オプション）**
    // PlaySoundEffect("player_stomp_bounce");

    // **デバッグ出力**
    OutputDebugStringA("Player: Stomp bounce applied!\n");
}

// **新追加: ダメージ状態更新**
void Player::UpdateDamageState()
{
    if (currentState == HIT) {
        hitTimer += 0.016f; // 60FPS想定

        // ダメージ状態の終了
        if (hitTimer >= HIT_DURATION) {
            if (onGround) {
                if (fabsf(velocityX) > 0.5f) {
                    currentState = WALKING;
                }
                else {
                    currentState = IDLE;
                }
            }
            else {
                currentState = FALLING;
            }
            hitTimer = 0.0f;
        }

        // ノックバック減衰の更新
        if (knockbackDecay > 0.0f) {
            knockbackDecay -= 0.016f;
            if (knockbackDecay <= 0.0f) {
                knockbackDecay = 0.0f;
            }
        }
    }
}

// **新追加: 無敵時間更新**
void Player::UpdateInvulnerability()
{
    if (invulnerabilityTimer > 0.0f) {
        invulnerabilityTimer -= 0.016f; // 60FPS想定
        if (invulnerabilityTimer <= 0.0f) {
            invulnerabilityTimer = 0.0f;
        }
    }
}

// **新追加: ダメージ処理（改良版）**
void Player::TakeDamage(int damage, float knockbackDirection)
{
    if (invulnerabilityTimer > 0.0f) return; // 無敵時間中はダメージを受けない

    // ダメージ状態に変更
    currentState = HIT;
    hitTimer = 0.0f;
    invulnerabilityTimer = INVULNERABILITY_DURATION;
    knockbackDecay = 1.0f;

    // **改良されたノックバック効果**
    if (knockbackDirection != 0.0f) {
        // ノックバック強度を調整
        float knockbackForce = KNOCKBACK_FORCE;

        // 地上にいる場合はより強いノックバック
        if (onGround) {
            knockbackForce *= 1.2f;
            velocityY = KNOCKBACK_VERTICAL; // 少し浮かせる
            onGround = false;
        }
        else {
            // 空中の場合は水平方向のみ
            knockbackForce *= 0.8f;
        }

        velocityX = knockbackDirection * knockbackForce;

        // ノックバック方向に応じて向きを変更
        facingRight = (knockbackDirection > 0.0f);
    }

    // **デバッグ出力**
    OutputDebugStringA("Player: Taking damage with improved knockback!\n");
}

// **新追加: 重力のみ適用（自動歩行用）**
void Player::ApplyGravityOnly(StageManager* stageManager)
{
    // 重力適用
    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }

    // Y方向の位置更新
    y += velocityY;

    // 地面との衝突判定のみ
    HandleGroundCollisionOnly(stageManager);
}

// **新追加: 地面衝突のみ処理（自動歩行用）**
void Player::HandleGroundCollisionOnly(StageManager* stageManager)
{
    const float COLLISION_WIDTH = 80.0f;
    const float COLLISION_HEIGHT = 100.0f;

    // 足元の複数点でチェック
    float footY = y + COLLISION_HEIGHT / 2;
    float leftFoot = x - COLLISION_WIDTH / 3;
    float rightFoot = x + COLLISION_WIDTH / 3;
    float centerFoot = x;

    // 3点で地面チェック
    bool leftHit = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // 地面に着地
        float groundY = FindPreciseGroundY(x, y, COLLISION_WIDTH, stageManager);
        if (groundY != -1) {
            y = groundY - COLLISION_HEIGHT / 2;
            velocityY = 0.0f;
            onGround = true;

            // 自動歩行中は歩行アニメーションに設定
            if (isAutoWalking) {
                currentState = WALKING;
            }
        }
    }
    else {
        // 自由落下
        if (onGround) {
            onGround = false;
            if (currentState != JUMPING) {
                currentState = FALLING;
            }
        }
    }
}

// **新追加: アニメーションのみ更新（自動歩行用）**
void Player::UpdateAnimationOnly()
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

// **新しい関数: UpdateAutoWalkPhysics を完全に修正**
void Player::UpdateAutoWalkPhysics(StageManager* stageManager)
{
    // **自動歩行中は穏やかな右方向移動**
    const float AUTO_WALK_SPEED = 3.5f; // 速度を大幅に減少（6.0f → 3.5f）

    // **段階的な加速（急激な動きを防止）**
    if (fabsf(velocityX) < AUTO_WALK_SPEED) {
        velocityX += 0.3f; // 徐々に加速
    }
    velocityX = min(velocityX, AUTO_WALK_SPEED); // 最大速度制限

    facingRight = true; // 右向き

    // **重力適用（地上にいる場合は無効化）**
    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }
    else {
        // **地上にいる場合は垂直速度を完全に0に**
        if (velocityY > 0) {
            velocityY = 0;
        }
    }

    // **位置更新（X方向とY方向）**
    x += velocityX;
    y += velocityY;

    // **ステージ境界チェック**
    if (x + 40.0f > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - 40.0f;
        velocityX = 0.0f;
    }

    // **衝突判定処理（完全版）**
    HandleCollisions(stageManager);

    // **状態設定（落下中でない限り歩行アニメーション）**
    if (onGround) {
        currentState = WALKING;
    }
    else if (velocityY > 0) {
        currentState = FALLING;
    }
    else {
        currentState = JUMPING;
    }
}

// **修正されたスライディング開始**
void Player::StartSliding()
{
    currentState = SLIDING;
    slidingTimer = 0.0f;

    // 現在の速度を基にスライディング初期速度を設定
    float currentSpeed = fabsf(velocityX);
    slidingSpeed = max(currentSpeed, SLIDING_INITIAL_SPEED);

    // 非常に高速だった場合はさらに強化
    if (currentSpeed > 7.0f) {
        slidingSpeed = currentSpeed * 1.2f; // 20%ボーナス
    }

    wasRunningWhenSlideStarted = (currentSpeed > 6.0f);

    // 現在の方向を維持してスライディング速度を設定
    if (facingRight) {
        velocityX = slidingSpeed;
    }
    else {
        velocityX = -slidingSpeed;
    }

    // **スライディング効果音の再生（オプション）**
    // SoundManager::GetInstance().PlaySE(SoundManager::SFX_SLIDE);

    char debugMsg[128];
    sprintf_s(debugMsg, "Player: Started sliding! Initial speed: %.2f\n", slidingSpeed);
    OutputDebugStringA(debugMsg);
}

// **修正されたスライディング更新**
void Player::UpdateSliding()
{
    slidingTimer += 0.016f; // 60FPS想定

    // 段階的な減速システム（より自然な減速カーブ）
    float progress = slidingTimer / SLIDING_DURATION;

    if (progress < 0.2f) {
        // 初期段階: 非常に緩やかな減速（勢いを維持）
        slidingSpeed *= 0.995f;
    }
    else if (progress < 0.5f) {
        // 前期段階: 緩やかな減速
        slidingSpeed *= 0.98f;
    }
    else if (progress < 0.8f) {
        // 中期段階: 標準的な減速
        slidingSpeed *= SLIDING_DECELERATION;
    }
    else {
        // 終期段階: 急速な減速
        slidingSpeed *= 0.80f;
    }

    // 最小速度制限
    if (slidingSpeed < SLIDING_MIN_SPEED) {
        slidingSpeed = SLIDING_MIN_SPEED;
    }

    // 方向を維持して速度を適用
    if (facingRight) {
        velocityX = slidingSpeed;
    }
    else {
        velocityX = -slidingSpeed;
    }

    // **デバッグ情報（頻度を下げる）**
    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 20 == 0) { // 約0.33秒ごと
        char debugMsg[128];
        sprintf_s(debugMsg, "Sliding: Timer=%.2f, Speed=%.2f, Progress=%.1f%%\n",
            slidingTimer, slidingSpeed, progress * 100);
        OutputDebugStringA(debugMsg);
    }
}

// **修正されたスライディング終了**
void Player::EndSliding()
{
    // 状態を適切に設定
    if (onGround) {
        if (fabsf(velocityX) > 3.0f) {
            currentState = WALKING;
        }
        else if (fabsf(velocityX) > 0.5f) {
            currentState = WALKING; // 低速でも歩行状態を維持
        }
        else {
            currentState = IDLE;
            velocityX = 0.0f; // 完全停止
        }
    }
    else {
        currentState = FALLING;
    }

    // スライディング変数をリセット
    slidingTimer = 0.0f;
    slidingSpeed = 0.0f;
    wasRunningWhenSlideStarted = false;

    OutputDebugStringA("Player: Sliding ended!\n");
}

// **修正されたスライディング時の当たり判定高さ取得**
float Player::GetSlidingCollisionHeight() const
{
    if (currentState == SLIDING) {
        return 100.0f * SLIDING_HEIGHT_REDUCTION; // 通常の60%の高さ
    }
    return 100.0f; // 通常の高さ
}

// **新追加: スライディングエフェクト描画**
void Player::DrawSlidingEffect(float cameraX)
{
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    float progress = GetSlidingProgress();
    float effectIntensity = 1.0f - progress; // 時間とともに減衰

    if (effectIntensity > 0.1f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(100 * effectIntensity));

        // 後方に煙のような効果
        int smokeX = screenX - (facingRight ? 30 : -30);
        int smokeY = screenY + 40; // 足元付近

        // 複数の円で煙効果を表現
        for (int i = 0; i < 3; i++) {
            int offsetX = smokeX - (facingRight ? i * 15 : -i * 15);
            int radius = (int)(8 + i * 3 * effectIntensity);
            int alpha = (int)(80 * effectIntensity / (i + 1));

            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawCircle(offsetX, smokeY - i * 5, radius, GetColor(200, 200, 200), TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}