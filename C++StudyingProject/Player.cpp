#include "Player.h"
#include <math.h>
#include <algorithm>
#include <cmath>
#include "SoundManager.h"
#include "BlockSystem.h"
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
    , slidingTimer(0.0f)
    , slidingSpeed(0.0f)
    , wasRunningWhenSlideStarted(false)
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
    // 踏みつけクールダウンの更新
    if (stompCooldown > 0.0f) {
        stompCooldown -= 0.016f; 
        if (stompCooldown <= 0.0f) {
            wasStomping = false;
        }
    }

    // ダメージ状態の更新
    UpdateDamageState();

    // 無敵時間の更新
    UpdateInvulnerability();

    // 物理演算更新
    UpdatePhysics(stageManager);
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
    if (safeY < -200) safeY = -200;
    if (safeY > 1500) safeY = 1500;

    // アイドル時の上下揺れ効果（安全な範囲で）
    float bobOffset = 0.0f;
    if (currentState == IDLE && onGround) {
        bobOffset = sinf(bobPhase) * 2.0f;
    }

    // スライディング時の特別な効果
    float slidingOffset = 0.0f;
    if (currentState == SLIDING) {
        float slidingProgress = GetSlidingProgress();
        float vibrationIntensity = (1.0f - slidingProgress) * 1.5f;
        slidingOffset = sinf(slidingTimer * 40.0f) * vibrationIntensity;
    }

    // ダメージ時の自然なノックバック振動効果
    float hitShake = 0.0f;
    if (currentState == HIT && hitTimer < HIT_DURATION * 0.7f) {
        float shakeProgress = hitTimer / (HIT_DURATION * 0.7f);
        float shakeIntensity = (1.0f - shakeProgress) * 2.5f;
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

    // 無敵時間中の点滅効果
    bool shouldDraw = true;
    if (invulnerabilityTimer > 0.0f) {
        float blinkSpeed = 12.0f;
        shouldDraw = ((int)(invulnerabilityTimer * blinkSpeed) % 2 == 0);
    }

    if (shouldDraw) {
        // スライディング時の特別な色効果
        if (currentState == SLIDING) {
            float slidingProgress = GetSlidingProgress();
            int blueIntensity = 255 + (int)(50 * sinf(slidingTimer * 8.0f) * (1.0f - slidingProgress));
            if (blueIntensity > 255) blueIntensity = 255;
            SetDrawBright(255, 255, blueIntensity);
        }
        // ダメージ時の赤色効果
        else if (currentState == HIT && hitTimer < HIT_DURATION * 0.4f) {
            float flashProgress = hitTimer / (HIT_DURATION * 0.4f);
            int redIntensity = (int)(255 * (1.0f - flashProgress));
            SetDrawBright(255, 255 - redIntensity / 2, 255 - redIntensity);
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

    // スライディング時の煙エフェクト
    if (currentState == SLIDING && onGround) {
        DrawSlidingEffect(cameraX);
    }
}

void Player::UpdatePhysics(StageManager* stageManager)
{
    // 自動歩行モード専用の処理
    if (isAutoWalking) {
        UpdateAutoWalkPhysics(stageManager);
        return;
    }

    // HIT状態の制御
    bool canControl = true;
    if (currentState == HIT) {
        hitTimer += 0.016f;
        canControl = (hitTimer > HIT_DURATION * 0.5f);
    }

    // HIT→IDLE/WALKINGへの復帰を強制
    if (onGround && currentState == HIT && canControl) {
        bool leftPressed = CheckHitKey(KEY_INPUT_LEFT);
        bool rightPressed = CheckHitKey(KEY_INPUT_RIGHT);
        if (leftPressed || rightPressed) {
            currentState = WALKING;
        }
        else {
            currentState = IDLE;
        }
        hitTimer = 0.0f;
        knockbackDecay = 0.0f;
    }

    // スライディング状態の更新
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

        // **位置更新を追加**
        x += velocityX;
        y += velocityY;
        return;
    }

    // 改良された水平移動処理（速度調整版）
    bool leftPressed = canControl && CheckHitKey(KEY_INPUT_LEFT);
    bool rightPressed = canControl && CheckHitKey(KEY_INPUT_RIGHT);
    bool downPressed = canControl && CheckHitKey(KEY_INPUT_DOWN);

    // スライディング開始判定
    static bool downWasPressedLastFrame = false;
    bool downJustPressed = downPressed && !downWasPressedLastFrame;

    if (downJustPressed && onGround && fabsf(velocityX) > 4.0f && currentState != DUCKING) {
        StartSliding();
        downWasPressedLastFrame = downPressed;
        return;
    }
    downWasPressedLastFrame = downPressed;

    // 段階的加速度システム（調整版）
    float currentSpeed = fabsf(velocityX);
    float targetSpeed = 0.0f;
    float accelerationRate = ACCELERATION; // 0.6f（調整済み）

    if (leftPressed && !downPressed) {
        targetSpeed = -MAX_HORIZONTAL_SPEED; // -8.0f（調整済み）

        // より段階的な加速
        if (currentSpeed < 1.5f) {
            accelerationRate = ACCELERATION * 1.2f; // 初期加速をやや強めに
        }
        else if (currentSpeed < 4.0f) {
            accelerationRate = ACCELERATION;        // 通常加速
        }
        else {
            accelerationRate = ACCELERATION * 0.6f; // 高速域では加速を抑制
        }

        velocityX -= accelerationRate;
        if (velocityX < targetSpeed) {
            velocityX = targetSpeed;
        }
        facingRight = false;

        // HIT状態で、空中でも操作復帰できるように
        if (currentState == HIT && canControl) {
            if (onGround) {
                currentState = WALKING;
            }
            else {
                currentState = FALLING;
            }
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;
        }
    }
    else if (rightPressed && !downPressed) {
        targetSpeed = MAX_HORIZONTAL_SPEED; // 8.0f（修正: 正の値）

        // より段階的な加速
        if (currentSpeed < 1.5f) {
            accelerationRate = ACCELERATION * 1.2f; // 初期加速をやや強めに
        }
        else if (currentSpeed < 4.0f) {
            accelerationRate = ACCELERATION;        // 通常加速
        }
        else {
            accelerationRate = ACCELERATION * 0.6f; // 高速域では加速を抑制
        }

        velocityX += accelerationRate; // 修正: 正の方向に加速
        if (velocityX > targetSpeed) {
            velocityX = targetSpeed;
        }
        facingRight = true; // 修正: 右向きに設定

        // **修正: HIT状態でも動けるようになったら状態を更新**
        if (onGround && currentState == HIT && canControl) {
            currentState = WALKING;
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;
        }
        else if (onGround && currentState != HIT) {
            currentState = (currentSpeed > 1.0f) ? WALKING : IDLE;
        }
    }
    else {
        // 改良された減速システム（より止まりやすく）
        float currentFriction = FRICTION; // 0.88f（調整済み）

        // **修正: HIT状態でのノックバック減速処理を簡素化**
        if (currentState == HIT && knockbackDecay > 0.0f) {
            currentFriction = 0.90f; // ノックバック中の減速を調整
        }
        else {
            if (currentSpeed > 5.0f) {
                currentFriction = 0.90f; // 高速時の摩擦を増加
            }
            else if (currentSpeed > 2.5f) {
                currentFriction = 0.88f; // 中速時の摩擦
            }
            else {
                currentFriction = 0.84f; // 低速時の摩擦
            }
        }

        velocityX *= currentFriction;

        // 停止判定を厳しく
        if (fabsf(velocityX) < 0.2f) {
            velocityX = 0.0f;
        }

        // **修正: HIT状態から回復**
        if (onGround && currentState == HIT && canControl && fabsf(velocityX) < 0.5f) {
            currentState = IDLE;
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;
        }
        else if (onGround && currentState != JUMPING && currentState != FALLING && currentState != HIT) {
            if (fabsf(velocityX) < 0.5f) {
                currentState = IDLE;
            }
            else {
                currentState = WALKING;
            }
        }
    }

    // しゃがみ処理
    if (canControl && downPressed && onGround && currentState != HIT && currentState != SLIDING) {
        if (fabsf(velocityX) < 1.0f && !leftPressed && !rightPressed) {
            currentState = DUCKING;
            velocityX *= 0.6f;
        }
    }

    // ジャンプ処理
    static bool spaceWasPressedLastFrame = false;
    bool spacePressed = CheckHitKey(KEY_INPUT_SPACE) != 0;

    if (canControl && spacePressed && !spaceWasPressedLastFrame && onGround &&
        currentState != JUMPING && currentState != SLIDING) {

        float jumpPower = JUMP_POWER; // -16.0f（強化済み）
        if (fabsf(velocityX) > 6.0f) {
            jumpPower *= 1.1f;
        }

        velocityY = jumpPower;
        currentState = JUMPING;
        onGround = false;
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_JUMP);

        // **修正: HIT状態からジャンプで回復**
        if (currentState == HIT) {
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;
        }

        OutputDebugStringA("Player: Jump executed!\n");
    }

    spaceWasPressedLastFrame = spacePressed;

    // 空中での重力処理（ふわっとジャンプ対応）
    if (!onGround) {
        float currentGravity = GRAVITY; // 0.35f（調整済み）

        // ★1. ジャンプボタンを離した時の早期落下（強化）
        if (currentState == JUMPING && !spacePressed && velocityY < 0) {
            velocityY *= JUMP_RELEASE_MULTIPLIER; // 0.3f（強化済み）
        }

        // ★2. ジャンプ頂点付近での重力軽減（ふわっと効果強化）
        if (fabsf(velocityY) < APEX_THRESHOLD) { // 3.0f（拡大済み）
            currentGravity *= APEX_GRAVITY_REDUCTION; // 0.4f（大幅軽減）

            // デバッグ出力（頂点付近のふわっと効果）
            static int apexDebugCounter = 0;
            apexDebugCounter++;
            if (apexDebugCounter % 30 == 0) {
                OutputDebugStringA("Player: Enhanced apex float effect active!\n");
            }
        }

        // ★3. 上昇中と下降中で異なる重力（滑らかな落下）
        if (velocityY < 0) {
            // 上昇中：軽減重力またはさらに軽い重力
            velocityY += currentGravity;
        }
        else {
            // 下降中：少し重い重力だが、ストンッとしないように調整
            velocityY += currentGravity * 1.15f; // 控えめな落下加速
        }

        // ★4. 速度制限（改良版）
        if (velocityY < -22.0f) {  // 上昇速度上限をさらに引き上げ
            velocityY = -22.0f;
        }
        if (velocityY > MAX_FALL_SPEED) {  // 9.0f（落下速度上限を低く保つ）
            velocityY = MAX_FALL_SPEED;
        }

        // ★5. 状態遷移（改良版）
        if (velocityY > 0.5f && currentState == JUMPING) {  // 落下判定をより緩く
            currentState = FALLING;
        }
    }
    else {
        if (velocityY > 0) {
            velocityY = 0;
        }
    }

    // **重要: 位置更新を復活**
    x += velocityX;
    y += velocityY;
}

void Player::HandleCollisions(StageManager* stageManager)
{
    const float COLLISION_WIDTH = 80.0f;
    float collisionHeight = GetSlidingCollisionHeight();

    // **重要: ブロックシステムとの統合のため、GameSceneから渡されるblockSystemを参照**
    // この関数は後でGameSceneから呼び出される形に変更する

    // ===== X方向の移動と衝突判定 =====
    float newX = x + velocityX;

    // ステージ境界チェック
    if (newX - COLLISION_WIDTH / 2 < 0) {
        x = COLLISION_WIDTH / 2;
        velocityX = 0.0f;
        if (currentState == SLIDING) {
            EndSliding();
        }
        return; // 早期リターン
    }
    else if (newX + COLLISION_WIDTH / 2 > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - COLLISION_WIDTH / 2;
        velocityX = 0.0f;
        if (currentState == SLIDING) {
            EndSliding();
        }
        return; // 早期リターン
    }

    // **通常のステージとの衝突チェック（ブロック衝突は後でGameSceneで処理）**
    if (CheckXCollision(newX, y, COLLISION_WIDTH, collisionHeight, stageManager)) {
        velocityX = 0.0f;
        newX = AdjustXPosition(x, velocityX > 0, COLLISION_WIDTH, stageManager);
        if (currentState == SLIDING) {
            EndSliding();
        }
    }
    x = newX;

    // ===== Y方向の移動と衝突判定 =====
    float newY = y + velocityY;

    if (velocityY > 0) {
        // 下方向移動（落下）
        HandleDownwardMovementStageOnly(newY, COLLISION_WIDTH, collisionHeight, stageManager);
    }
    else if (velocityY < 0) {
        // 上方向移動（ジャンプ）
        HandleUpwardMovementStageOnly(newY, COLLISION_WIDTH, collisionHeight, stageManager);
    }
    else {
        // Y方向の速度が0の場合
        y = newY;
    }

    // 画面外落下の救済
    if (y > 1400) {
        ResetPosition();
    }
}

void Player::HandleDownwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager)
{
    float footY = newY + height / 2;
    float leftFoot = x - width / 3;
    float rightFoot = x + width / 3;
    float centerFoot = x;

    // **ステージ地面のみをチェック**
    bool leftHit = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // ステージ地面に着地
        float groundY = FindPreciseGroundY(x, newY, width, stageManager);
        if (groundY != -1) {
            y = groundY - height / 2;
            velocityY = 0.0f;

            if (!onGround) {
                onGround = true;
                SetLandingState();
            }
        }
    }
    else {
        // **修正: 自由落下時も位置更新**
        y = newY;

        // **重要: ブロック上にいる可能性があるので、onGroundの変更は慎重に**
        // この判定はGameSceneのブロック処理後に行う
    }
}

// ステージ地面のみの判定
bool Player::IsOnStageGround(float playerX, float playerY, float width, float height, StageManager* stageManager)
{
    float footY = playerY + height / 2 + 4;
    float leftFoot = playerX - width / 3;
    float rightFoot = playerX + width / 3;
    float centerFoot = playerX;

    return CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);
}

void Player::HandleCollisionsWithBlocks(StageManager* stageManager, BlockSystem* blockSystem)
{
    const float COLLISION_WIDTH = 80.0f;
    float collisionHeight = GetSlidingCollisionHeight();

    // **1. 横方向の衝突処理**
    if (fabsf(velocityX) > 0.1f) {
        if (blockSystem->CheckCollision(x, y, COLLISION_WIDTH, collisionHeight)) {
            // ブロックとの横衝突
            if (velocityX > 0) {
                // 右移動中の衝突 - 左に押し戻し
                float adjustedX = x;
                for (int i = 0; i < 32 && adjustedX > 0; i++) {
                    adjustedX -= 2.0f;
                    if (!blockSystem->CheckCollision(adjustedX, y, COLLISION_WIDTH, collisionHeight)) {
                        x = adjustedX;
                        break;
                    }
                }
            }
            else {
                // 左移動中の衝突 - 右に押し戻し
                float adjustedX = x;
                for (int i = 0; i < 32; i++) {
                    adjustedX += 2.0f;
                    if (!blockSystem->CheckCollision(adjustedX, y, COLLISION_WIDTH, collisionHeight)) {
                        x = adjustedX;
                        break;
                    }
                }
            }
            velocityX = 0.0f;
            if (currentState == SLIDING) {
                EndSliding();
            }
        }
    }

    // **2. 上方向の移動（ブロック下面への衝突）**
    if (velocityY < 0.0f) {
        float headY = y - collisionHeight / 2;
        if (blockSystem->CheckCollision(x, headY, COLLISION_WIDTH, 16.0f)) {
            // ブロック下面にぶつかった
            velocityY = 0.0f;
            if (currentState == JUMPING) {
                currentState = FALLING;
            }
        }
    }

    // **3. 地面状態の確認（ブロック上にいるかチェック）**
    if (onGround && velocityY >= 0.0f) {
        bool onStageGround = IsOnStageGround(x, y, COLLISION_WIDTH, collisionHeight, stageManager);
        bool onBlockGround = blockSystem->CheckPlayerLandingOnBlocksImproved(x, y, COLLISION_WIDTH, collisionHeight);

        if (!onStageGround && !onBlockGround) {
            onGround = false;
            if (currentState != JUMPING && currentState != SLIDING) {
                currentState = FALLING;
            }
        }
    }
}

// ステージのみの上方向移動処理
void Player::HandleUpwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager)
{
    float headY = newY - height / 2;
    float leftHead = x - width / 3;
    float rightHead = x + width / 3;
    float centerHead = x;

    // ステージの天井チェック
    bool leftHit = CheckPointCollision(leftHead, headY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerHead, headY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightHead, headY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        float ceilingY = FindPreciseCeilingY(x, newY, width, stageManager);
        if (ceilingY != -1) {
            y = ceilingY + height / 2;
            velocityY = 0.0f;

            if (currentState == JUMPING) {
                currentState = FALLING;
            }
            onGround = false;
        }
    }
    else {
        y = newY;
    }
}
bool Player::CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager)
{
    float checkPoints[] = {
        currentY - height / 2 + 16,
        currentY,
        currentY + height / 2 - 16
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
    float collisionHeight = (currentState == SLIDING) ? GetSlidingCollisionHeight() : height;

    // **修正: ジャンプ中は地面判定をスキップ**
    if (currentState == JUMPING && velocityY < 0) {
        y = newY;
        return;
    }

    // 足元の判定点
    float footY = newY + collisionHeight / 2;
    float leftFoot = x - width / 3;
    float rightFoot = x + width / 3;
    float centerFoot = x;

    // **ステージ地面との衝突チェック**
    bool leftHit = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // **ステージ地面に着地**
        float groundY = FindPreciseGroundY(x, newY, width, stageManager);
        if (groundY != -1) {
            y = groundY - collisionHeight / 2;
            velocityY = 0.0f;

            // **onGroundの状態変化を安定化**
            if (!onGround) {
                onGround = true;
                SetLandingState();

                OutputDebugStringA("Player: Landed on stage ground\n");
            }
        }
    }
    else {
        // **自由落下**
        y = newY;

        // **地面から離れた時の処理を慎重に**
        if (onGround && velocityY > 2.0f) { // 十分な落下速度がある場合のみ
            bool stillNearGround = IsOnGround(x, y, width, collisionHeight, stageManager);
            if (!stillNearGround) {
                onGround = false;
                if (currentState != JUMPING && currentState != SLIDING) {
                    currentState = FALLING;
                }
                OutputDebugStringA("Player: Left ground, now falling\n");
            }
        }
    }
}



// **修正版HandleUpwardMovement: ブロック下への衝突を正しく処理**
void Player::HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager)
{
    float headY = newY - height / 2;
    float leftHead = x - width / 3;
    float rightHead = x + width / 3;
    float centerHead = x;

    // 3点で天井チェック（ブロック下面も含む）
    bool leftHit = CheckPointCollision(leftHead, headY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerHead, headY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightHead, headY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        // 天井またはブロック下面にぶつかった
        float ceilingY = FindPreciseCeilingY(x, newY, width, stageManager);
        if (ceilingY != -1) {
            y = ceilingY + height / 2;
            velocityY = 0.0f;  // **修正: 上方向の速度を完全に停止**

            // **修正: ジャンプ中に天井にぶつかった場合は落下状態に**
            if (currentState == JUMPING) {
                currentState = FALLING;
            }

            onGround = false;

            // **デバッグ出力**
            OutputDebugStringA("Player: Hit ceiling/block from below\n");
        }
    }
    else {
        // 自由移動
        y = newY;
    }
}


void Player::SetLandingState()
{
    if (currentState == JUMPING || currentState == FALLING) {
        bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);

        if (currentState == SLIDING) {
            // スライディング中は状態を維持
        }
        else if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
            currentState = DUCKING;
        }
        else if (hasInput) {
            currentState = WALKING;
        }
        else {
            currentState = IDLE;
        }
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
    float adjustedX = currentX;
    float step = movingRight ? -1.0f : 1.0f;

    for (int i = 0; i < 64; i++) {
        adjustedX += step;
        if (!CheckXCollision(adjustedX, y, width, 100.0f, stageManager)) {
            return adjustedX;
        }
    }

    return currentX;
}

// **修正版FindPreciseGroundY: ブロック上の地面も正確に検出**
float Player::FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager)
{
    int tileSize = Stage::TILE_SIZE; // 64px

    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY + 4;

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * tileSize;

        float leftX = playerX - width / 2 + 8;
        float rightX = playerX + width / 2 - 8;
        float centerX = playerX;

        // **修正: より厳密な地面検出**
        if (stageManager->CheckCollision(leftX, checkY, 8, 8) ||
            stageManager->CheckCollision(centerX, checkY, 8, 8) ||
            stageManager->CheckCollision(rightX, checkY, 8, 8)) {

            return checkY; // このタイルの上端が地面
        }
    }

    return -1;
}

float Player::FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager)
{
    int tileSize = Stage::TILE_SIZE;

    int startTileY = (int)(playerY / tileSize);
    int endTileY = startTileY - 4;

    for (int tileY = startTileY; tileY >= endTileY && tileY >= 0; tileY--) {
        float checkY = (tileY + 1) * tileSize;

        float leftX = playerX - width / 2 + 8;
        float rightX = playerX + width / 2 - 8;
        float centerX = playerX;

        if (stageManager->CheckCollision(leftX, checkY - 8, 8, 8) ||
            stageManager->CheckCollision(centerX, checkY - 8, 8, 8) ||
            stageManager->CheckCollision(rightX, checkY - 8, 8, 8)) {

            return checkY;
        }
    }

    return -1;
}

bool Player::IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager)
{
    float footY = playerY + height / 2 + 4;
    float leftFoot = playerX - width / 3;
    float rightFoot = playerX + width / 3;
    float centerFoot = playerX;

    // ステージの地面をチェック
    bool onStageGround = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager) ||
        CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

    return onStageGround;
}

// **その他の関数は元のままで省略（影描画、アニメーション、スライディングなど）**

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
    case SLIDING: return sprites.duck;
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

void Player::ResetPosition()
{
    x = 300.0f;
    y = 200.0f;
    velocityX = 0.0f;
    velocityY = 0.0f;
    currentState = FALLING;
    onGround = false;
    facingRight = true;

    hitTimer = 0.0f;
    invulnerabilityTimer = 0.0f;
    knockbackDecay = 0.0f;

    slidingTimer = 0.0f;
    slidingSpeed = 0.0f;
    wasRunningWhenSlideStarted = false;
}

void Player::ApplyStompBounce(float bounceVelocity)
{
    velocityY = (bounceVelocity != -8.0f) ? bounceVelocity : -12.0f;
    currentState = JUMPING;
    onGround = false;
    wasStomping = true;
    stompCooldown = STOMP_COOLDOWN_DURATION;

    OutputDebugStringA("Player: Stomp bounce applied!\n");
}

void Player::UpdateDamageState()
{
    if (currentState == HIT) {
        hitTimer += 0.016f;

        // **修正: シンプルな回復処理に変更**
        if (hitTimer >= HIT_DURATION) {
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;

            // 状態遷移（地面にいるならIDLE、空中ならFALLING）
            if (onGround) {
                currentState = IDLE;
            }
            else {
                currentState = FALLING;
            }
            // タイマーとノックバックをリセット
            hitTimer = 0.0f;
            knockbackDecay = 0.0f;

            // **デバッグ出力**
            OutputDebugStringA("Player: Fully recovered from HIT state!\n");
        }

        // ノックバック減衰処理（シンプル化）
        if (knockbackDecay > 0.0f) {
            knockbackDecay -= 0.02f; // より速く減衰
            if (knockbackDecay <= 0.0f) {
                knockbackDecay = 0.0f;
                // ノックバックが完全に終了したら速度を減速
                velocityX *= 0.5f;
            }
        }
        
    }
}

void Player::UpdateInvulnerability()
{
    if (invulnerabilityTimer > 0.0f) {
        invulnerabilityTimer -= 0.016f;
        if (invulnerabilityTimer <= 0.0f) {
            invulnerabilityTimer = 0.0f;
        }
    }
}

void Player::TakeDamage(int damage, float knockbackDirection)
{    // 既に無敵なら無視
    if (IsInvulnerable()) return;

    // 状態をHITに変更し、タイマー初期化
    currentState = HIT;
    hitTimer = 0.0f;  // ← これがないと一生HITのままになる可能性

    // ノックバック反映
    velocityX = knockbackDirection * KNOCKBACK_FORCE;
    velocityY = KNOCKBACK_VERTICAL;
    onGround = false;
    knockbackDecay = 1.0f;

    // ノックバック方向が指定されていない場合は、現在の向きの逆方向
    if (knockbackDirection == 0.0f) {
        knockbackDirection = facingRight ? -1.0f : 1.0f;
    }

    // ノックバック力を適用
    float knockbackForce = KNOCKBACK_FORCE;

    if (onGround) {
        // 地面にいる場合は上方向にも飛ばす
        velocityY = KNOCKBACK_VERTICAL;
        onGround = false;
    }

    // 水平方向のノックバック
    velocityX = knockbackDirection * knockbackForce;
    velocityY = KNOCKBACK_VERTICAL;
    // ノックバック方向に向きを変える
    facingRight = (knockbackDirection > 0.0f);

    // **デバッグ出力を強化**
    char debugMsg[256];
    sprintf_s(debugMsg, "Player: Taking damage! HIT state started. Direction: %.2f, VelX: %.2f, VelY: %.2f\n",
        knockbackDirection, velocityX, velocityY);
    OutputDebugStringA(debugMsg);
}

void Player::ApplyGravityOnly(StageManager* stageManager)
{
    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }

    y += velocityY;
    HandleGroundCollisionOnly(stageManager);
}

void Player::HandleGroundCollisionOnly(StageManager* stageManager)
{
    const float COLLISION_WIDTH = 80.0f;
    const float COLLISION_HEIGHT = 100.0f;

    float footY = y + COLLISION_HEIGHT / 2;
    float leftFoot = x - COLLISION_WIDTH / 3;
    float rightFoot = x + COLLISION_WIDTH / 3;
    float centerFoot = x;

    bool leftHit = CheckPointCollision(leftFoot, footY, 8.0f, 8.0f, stageManager);
    bool centerHit = CheckPointCollision(centerFoot, footY, 8.0f, 8.0f, stageManager);
    bool rightHit = CheckPointCollision(rightFoot, footY, 8.0f, 8.0f, stageManager);

    if (leftHit || centerHit || rightHit) {
        float groundY = FindPreciseGroundY(x, y, COLLISION_WIDTH, stageManager);
        if (groundY != -1) {
            y = groundY - COLLISION_HEIGHT / 2;
            velocityY = 0.0f;
            onGround = true;

            if (isAutoWalking) {
                currentState = WALKING;
            }
        }
    }
    else {
        if (onGround) {
            onGround = false;
            if (currentState != JUMPING) {
                currentState = FALLING;
            }
        }
    }
}

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

void Player::UpdateAutoWalkPhysics(StageManager* stageManager)
{
    const float AUTO_WALK_SPEED = 3.5f;

    if (fabsf(velocityX) < AUTO_WALK_SPEED) {
        velocityX += 0.3f;
    }
    velocityX = min(velocityX, AUTO_WALK_SPEED);

    facingRight = true;

    if (!onGround) {
        velocityY += GRAVITY;
        if (velocityY > MAX_FALL_SPEED) {
            velocityY = MAX_FALL_SPEED;
        }
    }
    else {
        if (velocityY > 0) {
            velocityY = 0;
        }
    }

    x += velocityX;
    y += velocityY;

    if (x + 40.0f > Stage::STAGE_WIDTH) {
        x = Stage::STAGE_WIDTH - 40.0f;
        velocityX = 0.0f;
    }

    HandleCollisions(stageManager);

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

void Player::StartSliding()
{
    currentState = SLIDING;
    slidingTimer = 0.0f;

    float currentSpeed = fabsf(velocityX);
    slidingSpeed = max(currentSpeed, SLIDING_INITIAL_SPEED);

    if (currentSpeed > 7.0f) {
        slidingSpeed = currentSpeed * 1.2f;
    }

    wasRunningWhenSlideStarted = (currentSpeed > 6.0f);

    if (facingRight) {
        velocityX = slidingSpeed;
    }
    else {
        velocityX = -slidingSpeed;
    }

    char debugMsg[128];
    sprintf_s(debugMsg, "Player: Started sliding! Initial speed: %.2f\n", slidingSpeed);
    OutputDebugStringA(debugMsg);
}

void Player::UpdateSliding()
{
    slidingTimer += 0.016f;

    float progress = slidingTimer / SLIDING_DURATION;

    if (progress < 0.2f) {
        slidingSpeed *= 0.995f;
    }
    else if (progress < 0.5f) {
        slidingSpeed *= 0.98f;
    }
    else if (progress < 0.8f) {
        slidingSpeed *= SLIDING_DECELERATION;
    }
    else {
        slidingSpeed *= 0.80f;
    }

    if (slidingSpeed < SLIDING_MIN_SPEED) {
        slidingSpeed = SLIDING_MIN_SPEED;
    }

    if (facingRight) {
        velocityX = slidingSpeed;
    }
    else {
        velocityX = -slidingSpeed;
    }

    static int debugCounter = 0;
    debugCounter++;
    if (debugCounter % 20 == 0) {
        char debugMsg[128];
        sprintf_s(debugMsg, "Sliding: Timer=%.2f, Speed=%.2f, Progress=%.1f%%\n",
            slidingTimer, slidingSpeed, progress * 100);
        OutputDebugStringA(debugMsg);
    }
}

void Player::EndSliding()
{
    if (onGround) {
        if (fabsf(velocityX) > 3.0f) {
            currentState = WALKING;
        }
        else if (fabsf(velocityX) > 0.5f) {
            currentState = WALKING;
        }
        else {
            currentState = IDLE;
            velocityX = 0.0f;
        }
    }
    else {
        currentState = FALLING;
    }

    slidingTimer = 0.0f;
    slidingSpeed = 0.0f;
    wasRunningWhenSlideStarted = false;

    OutputDebugStringA("Player: Sliding ended!\n");
}

float Player::GetSlidingCollisionHeight() const
{
    if (currentState == SLIDING) {
        return 100.0f * SLIDING_HEIGHT_REDUCTION;
    }
    return 100.0f;
}

void Player::DrawSlidingEffect(float cameraX)
{
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    float progress = GetSlidingProgress();
    float effectIntensity = 1.0f - progress;

    if (effectIntensity > 0.1f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(100 * effectIntensity));

        int smokeX = screenX - (facingRight ? 30 : -30);
        int smokeY = screenY + 40;

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

// **影描画システム（省略版）**
void Player::DrawShadow(float cameraX, StageManager* stageManager)
{
    float playerFootX = x;
    float playerFootY = y + (PLAYER_HEIGHT / 2) - 8;

    float groundY = FindOptimalGroundForShadow(playerFootX, playerFootY, stageManager);

    if (groundY == -1) return;

    float distanceToGround = groundY - playerFootY;

    if (distanceToGround < 0 || distanceToGround > MAX_SHADOW_DISTANCE) return;

    int shadowX = (int)(playerFootX - cameraX);
    int shadowY = (int)groundY - 2;

    if (shadowX < -100 || shadowX > 1920 + 100) return;
    if (shadowY < -50 || shadowY > 1080 + 50) return;

    float normalizedDistance = min(distanceToGround / MAX_SHADOW_DISTANCE, 1.0f);

    float stateMultiplier = 1.0f;
    switch (currentState) {
    case DUCKING:
        stateMultiplier = 1.2f;
        break;
    case JUMPING:
    case FALLING:
        stateMultiplier = 0.9f;
        break;
    default:
        stateMultiplier = 1.0f;
        break;
    }

    float distanceMultiplier = 1.0f - powf(normalizedDistance, 1.5f) * 0.8f;

    float finalSizeMultiplier = stateMultiplier * distanceMultiplier;
    int shadowRadiusX = (int)(BASE_SHADOW_SIZE_X * finalSizeMultiplier);
    int shadowRadiusY = (int)(BASE_SHADOW_SIZE_Y * finalSizeMultiplier);

    float alphaMultiplier = 1.0f - powf(normalizedDistance, 0.8f) * 0.7f;
    int shadowAlpha = (int)(BASE_SHADOW_ALPHA * alphaMultiplier);

    if (shadowRadiusX < 6 || shadowRadiusY < 2 || shadowAlpha < 20) return;

    float colorIntensity = 50.0f * (1.0f - normalizedDistance * 0.6f);
    int shadowColor = GetColor((int)colorIntensity, (int)colorIntensity, (int)colorIntensity);

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, shadowAlpha);
    DrawOval(shadowX, shadowY, shadowRadiusX, shadowRadiusY, shadowColor, TRUE);

    if (distanceToGround < MAX_SHADOW_DISTANCE * 0.5f) {
        int innerAlpha = shadowAlpha / 3;
        int innerRadiusX = shadowRadiusX * 2 / 3;
        int innerRadiusY = shadowRadiusY * 2 / 3;

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, innerAlpha);
        DrawOval(shadowX, shadowY, innerRadiusX, innerRadiusY, GetColor(20, 20, 20), TRUE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

float Player::FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    float basicGround = FindNearestGroundForShadow(playerX, playerY, stageManager);

    if (basicGround == -1) {
        basicGround = FindPreciseGroundForShadow(playerX, playerY, stageManager);
    }

    if (basicGround == -1) {
        basicGround = FindAdaptiveGroundForShadow(playerX, playerY, stageManager);
    }

    return basicGround;
}

float Player::FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const int TILE_SIZE = Stage::TILE_SIZE;
    const float SEARCH_WIDTH = 60.0f;
    const int MAX_SEARCH_TILES = 15;

    int startTileY = (int)(playerY / TILE_SIZE);
    int endTileY = startTileY + MAX_SEARCH_TILES;

    for (int tileY = startTileY; tileY <= endTileY; tileY++) {
        float checkY = tileY * TILE_SIZE;

        bool groundFound = false;

        float checkPositions[] = {
            playerX,
            playerX - SEARCH_WIDTH / 2,
            playerX + SEARCH_WIDTH / 2
        };

        for (float checkX : checkPositions) {
            if (stageManager->CheckCollision(checkX, checkY, 4, 4)) {
                groundFound = true;
                break;
            }
        }

        if (groundFound) {
            return checkY;
        }
    }

    return -1;
}

float Player::FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const float SEARCH_WIDTH = 50.0f;
    const int MAX_SEARCH_DISTANCE = 960;
    const int SEARCH_STEP = 8;

    for (int searchY = (int)playerY; searchY < playerY + MAX_SEARCH_DISTANCE; searchY += SEARCH_STEP) {
        for (int i = 0; i < 5; i++) {
            float checkX = playerX - SEARCH_WIDTH / 2 + (SEARCH_WIDTH * i / 4);

            if (stageManager->CheckCollision(checkX, searchY, 4, 4)) {
                int tileY = searchY / Stage::TILE_SIZE;
                return (float)(tileY * Stage::TILE_SIZE);
            }
        }
    }

    return -1;
}

float Player::FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager)
{
    const float SEARCH_RADIUS = 30.0f;
    const int SEARCH_POINTS = 6;
    const int MAX_SEARCH_DISTANCE = 480;

    float nearestGroundY = -1;
    float minDistance = (float)(MAX_SEARCH_DISTANCE + 1);

    for (int i = 0; i < SEARCH_POINTS; i++) {
        float angle = (2.0f * 3.14159265f * i) / SEARCH_POINTS;
        float searchStartX = playerX + cosf(angle) * SEARCH_RADIUS;

        for (int searchY = (int)playerY; searchY < playerY + MAX_SEARCH_DISTANCE; searchY += 12) {
            if (stageManager->CheckCollision(searchStartX, searchY, 4, 4)) {
                float distance = searchY - playerY;
                if (distance < minDistance) {
                    minDistance = distance;
                    nearestGroundY = searchY;
                }
                break;
            }
        }
    }

    if (nearestGroundY != -1) {
        int tileY = (int)(nearestGroundY / Stage::TILE_SIZE);
        nearestGroundY = (float)(tileY * Stage::TILE_SIZE);
    }

    return nearestGroundY;
}

// **その他の互換性関数**
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

void Player::DrawShadowDebugInfo(float cameraX, int shadowX, int shadowY, float distanceToGround, float normalizedDistance)
{
    int playerScreenX = (int)(x - cameraX);
    int playerScreenY = (int)y;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);

    DrawLine(playerScreenX, playerScreenY, shadowX, shadowY, GetColor(255, 255, 0));

    int footY = playerScreenY + PLAYER_HEIGHT / 2;
    DrawCircle(playerScreenX, footY, 3, GetColor(0, 255, 0), TRUE);

    std::string distInfo = "Ground Dist: " + std::to_string((int)distanceToGround) + "px";
    std::string normalInfo = "Normalized: " + std::to_string(normalizedDistance).substr(0, 4);
    std::string stateInfo = "State: ";

    switch (currentState) {
    case IDLE: stateInfo += "IDLE"; break;
    case WALKING: stateInfo += "WALKING"; break;
    case JUMPING: stateInfo += "JUMPING"; break;
    case FALLING: stateInfo += "FALLING"; break;
    case DUCKING: stateInfo += "DUCKING"; break;
    case SLIDING: stateInfo += "SLIDING"; break;
    case HIT: stateInfo += "HIT"; break;
    }

    DrawString(playerScreenX + 15, playerScreenY - 60, distInfo.c_str(), GetColor(255, 255, 0));
    DrawString(playerScreenX + 15, playerScreenY - 40, normalInfo.c_str(), GetColor(255, 255, 0));
    DrawString(playerScreenX + 15, playerScreenY - 20, stateInfo.c_str(), GetColor(255, 255, 0));

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void Player::DrawDebugInfo(float cameraX)
{
    const float COLLISION_WIDTH = 80.0f;
    const float COLLISION_HEIGHT = 100.0f;

    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    if (screenX < -200 || screenX > 2200) return;
    if (screenY < -200 || screenY > 1300) return;

    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(
        screenX - COLLISION_WIDTH / 2,
        screenY - COLLISION_HEIGHT / 2,
        screenX + COLLISION_WIDTH / 2,
        screenY + COLLISION_HEIGHT / 2,
        GetColor(255, 0, 0), FALSE
    );
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    std::string debugText =
        "Pos: (" + std::to_string((int)x) + "," + std::to_string((int)y) + ") " +
        "Vel: (" + std::to_string((int)(velocityX * 10)) + "," + std::to_string((int)(velocityY * 10)) + ") " +
        "Ground: " + (onGround ? "YES" : "NO");

    DrawString(screenX - 100, screenY - 140, debugText.c_str(), GetColor(255, 255, 0));

    if (fabsf(velocityX) > 0.1f || fabsf(velocityY) > 0.1f) {
        int arrowEndX = screenX + (int)(velocityX * 3);
        int arrowEndY = screenY + (int)(velocityY * 3);

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