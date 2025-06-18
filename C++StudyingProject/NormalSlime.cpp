#include "NormalSlime.h"
#include <math.h>

NormalSlime::NormalSlime(float startX, float startY)
    : EnemyBase(startX, startY, NORMAL_SLIME)
    , slimeState(SLIME_RESTING)
    , jumpTimer(0.0f)
    , restTimer(0.0f)
    , flattenedTimer(0.0f)
    , bobPhase(0.0f)
    , flattenScale(1.0f)
    , flattenProgress(0.0f)
{
    // ノーマルスライムのパラメータ設定
    health = 50;
    maxHealth = 50;
    moveSpeed = 1.5f;
    detectionRange = 120.0f;
    attackRange = 40.0f;
    attackPower = 5;

    // パトロール範囲を狭く設定
    patrolDistance = 128.0f;
    patrolStartX = startX - 64.0f;
    patrolEndX = startX + 64.0f;
}

void NormalSlime::Initialize()
{
    LoadSprites();
    slimeState = SLIME_RESTING;
    currentState = IDLE;
    restTimer = 0.0f;
}

void NormalSlime::LoadSprites()
{
    std::string basePath = "Sprites/Enemies/slime_normal_";

    sprites.idle = LoadGraph((basePath + "rest.png").c_str());
    sprites.walk_a = LoadGraph((basePath + "walk_a.png").c_str());
    sprites.walk_b = LoadGraph((basePath + "walk_b.png").c_str());
    sprites.flat = LoadGraph((basePath + "flat.png").c_str());

    sprites.attack = sprites.walk_a;
    sprites.damaged = sprites.idle;
    sprites.dead = sprites.flat;
}

void NormalSlime::UpdateBehavior(Player* player, StageManager* stageManager)
{
    // **死亡状態なら何もしない**
    if (currentState == DEAD || !active) {
        return;
    }

    // **潰れた状態の処理（改良版）**
    if (slimeState == SLIME_FLATTENED) {
        UpdateFlattenedAnimation();
        CheckFlattenedRecovery();
        return;
    }

    // スライム固有の移動処理
    UpdateSlimeMovement();

    // プレイヤーが近くにいる場合の特別な行動
    if (IsPlayerInRange(player, detectionRange)) {
        HandleJumpMovement(player);
    }
    else {
        // 通常のパトロール
        UpdatePatrol();
    }

    // スライム固有のアニメーション
    UpdateSlimeAnimation();
}

void NormalSlime::UpdateFlattenedAnimation()
{
    // **潰れアニメーションの更新**
    if (flattenProgress < 1.0f) {
        flattenProgress += 0.08f; // ゆっくりと潰れる
        if (flattenProgress > 1.0f) {
            flattenProgress = 1.0f;
        }

        // 潰れるスケール計算（Y軸方向に縮む、X軸方向に広がる）
        flattenScale = 1.0f - flattenProgress * 0.7f; // 30%の高さまで縮む
    }
}

void NormalSlime::UpdateSlimeMovement()
{
    switch (slimeState) {
    case SLIME_RESTING:
        velocityX = 0.0f;
        restTimer += 0.016f;

        if (restTimer >= REST_DURATION) {
            slimeState = SLIME_ACTIVE;
            restTimer = 0.0f;
            currentState = WALKING;
        }
        break;

    case SLIME_ACTIVE:
        jumpTimer += 0.016f;

        if (jumpTimer >= JUMP_INTERVAL && onGround) {
            StartJump();
        }

        if (jumpTimer > JUMP_INTERVAL * 3) {
            slimeState = SLIME_RESTING;
            jumpTimer = 0.0f;
            currentState = IDLE;
        }
        break;

    case SLIME_JUMPING:
        if (onGround && velocityY >= 0) {
            slimeState = SLIME_ACTIVE;
            jumpTimer = 0.0f;
        }
        break;
    }
}

void NormalSlime::UpdateSlimeAnimation()
{
    if (slimeState == SLIME_RESTING) {
        bobPhase += BOB_SPEED;
        if (bobPhase >= 2.0f * 3.14159265f) {
            bobPhase = 0.0f;
        }
    }

    UpdateAnimation();
}

void NormalSlime::StartJump()
{
    if (!onGround) return;

    velocityY = JUMP_POWER;
    slimeState = SLIME_JUMPING;
    jumpTimer = 0.0f;
    onGround = false;
}

void NormalSlime::HandleJumpMovement(Player* player)
{
    if (!player || slimeState == SLIME_FLATTENED) return;

    float playerX = player->GetX();

    if (slimeState == SLIME_JUMPING) {
        float direction = (playerX > x) ? 1.0f : -1.0f;
        velocityX = direction * moveSpeed * 0.5f;
        facingRight = (direction > 0);
    }
    else if (slimeState == SLIME_ACTIVE) {
        float direction = (playerX > x) ? 1.0f : -1.0f;
        velocityX = direction * moveSpeed;
        facingRight = (direction > 0);
    }
}

void NormalSlime::CheckFlattenedRecovery()
{
    flattenedTimer += 0.016f;
    velocityX = 0.0f;

    // **改良版：潰れた状態の処理**
    if (flattenedTimer >= FLATTENED_DISPLAY_DURATION) {
        // **完全に破壊**
        active = false;
        currentState = DEAD;
        health = 0;

        // **デバッグ出力**
        OutputDebugStringA("NormalSlime: Completely destroyed after flattening animation!\n");
    }
}

void NormalSlime::OnPlayerCollision(Player* player)
{
    if (!player || !active || currentState == DEAD) return;

    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();
    float enemyTop = y - COLLISION_HEIGHT / 2;

    // **踏みつけ判定を明確化**
    bool isStompedFromAbove = (
        playerVelY >= 0.0f &&                   // プレイヤーが下向きに移動
        playerY < y - 10.0f &&                  // プレイヤーが敵より上
        playerY + 50.0f >= enemyTop             // プレイヤーの足が敵の頭部付近
        );

    if (isStompedFromAbove) {
        // **踏みつけ処理：即座に倒さず、アニメーション開始**
        slimeState = SLIME_FLATTENED;
        currentState = DAMAGED; // 一時的にDAMAGED状態
        flattenedTimer = 0.0f;
        flattenProgress = 0.0f;
        flattenScale = 1.0f;
        velocityX = 0.0f;
        velocityY = 0.0f;

        // **デバッグ出力**
        OutputDebugStringA("NormalSlime: STOMPED! Starting flatten animation!\n");

        // **注意: プレイヤーの跳ね返りはEnemyManagerで処理済み**

    }
    else {
        // **横からの接触: プレイヤーにダメージ**

        // **プレイヤーが無敵状態でなければダメージを与える**
        if (!player->IsInvulnerable()) {
            // ノックバック方向を計算
            float knockbackDirection = (player->GetX() > x) ? 1.0f : -1.0f;

            // **修正: Player::TakeDamage のみを使用**
            // GameSceneのライフ管理は、EnemyManager経由で処理される想定
            player->TakeDamage(1, knockbackDirection);

            // **デバッグ出力**
            OutputDebugStringA("NormalSlime: Side collision - calling Player::TakeDamage!\n");
        }

        // 敵にも少しノックバック
        float enemyKnockback = (player->GetX() > x) ? -1.0f : 1.0f;
        velocityX = enemyKnockback * 1.5f;

        if (currentState != ATTACKING) {
            currentState = ATTACKING;
            stateTimer = 0.0f;
        }
    }
}

// **描画関数をオーバーライド（潰れアニメーション対応）**
void NormalSlime::Draw(float cameraX)
{
    if (!active) return;

    int currentSprite = GetCurrentSprite();
    if (currentSprite == -1) return;

    // 画面座標に変換
    int screenX = (int)(x - cameraX);
    int screenY = (int)y;

    // 画面外チェック
    if (screenX < -ENEMY_WIDTH || screenX > 1920 + ENEMY_WIDTH) return;
    if (screenY < -ENEMY_HEIGHT || screenY > 1080 + ENEMY_HEIGHT) return;

    // スプライトサイズ取得
    int spriteWidth, spriteHeight;
    GetGraphSize(currentSprite, &spriteWidth, &spriteHeight);

    // 中央揃えで描画位置調整
    screenX -= spriteWidth / 2;
    screenY -= spriteHeight / 2;

    // **潰れアニメーション時の特別な描画**
    if (slimeState == SLIME_FLATTENED) {
        // Y軸方向に縮み、X軸方向に広がる
        int flatWidth = (int)(spriteWidth * (1.0f + (1.0f - flattenScale) * 0.8f));
        int flatHeight = (int)(spriteHeight * flattenScale);

        // 潰れた分だけY位置を調整（地面に合わせる）
        int flatY = screenY + (spriteHeight - flatHeight);

        // 透明度の調整（徐々に薄くなる）
        int alpha = (int)(255 * (1.0f - flattenProgress * 0.5f));
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        // 潰れた状態で描画
        if (facingRight) {
            DrawExtendGraph(screenX - (flatWidth - spriteWidth) / 2, flatY,
                screenX - (flatWidth - spriteWidth) / 2 + flatWidth, flatY + flatHeight,
                currentSprite, TRUE);
        }
        else {
            // 左向きの場合はDrawTurnGraphを使用（通常サイズで描画してから反転）
            DrawExtendGraph(screenX - (flatWidth - spriteWidth) / 2, flatY,
                screenX - (flatWidth - spriteWidth) / 2 + flatWidth, flatY + flatHeight,
                currentSprite, TRUE);
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else {
        // **通常の描画処理**
        // 状態に応じた描画効果
        if (currentState == DAMAGED) {
            // ダメージ時の点滅効果
            int alpha = (int)(255 * (0.5f + 0.5f * sinf(stateTimer * 20.0f)));
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        }

        // 左右反転描画
        if (facingRight) {
            DrawGraph(screenX, screenY, currentSprite, TRUE);
        }
        else {
            DrawTurnGraph(screenX, screenY, currentSprite, TRUE);
        }

        // 描画モードリセット
        if (currentState == DAMAGED) {
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // デバッグ情報の描画
#ifdef _DEBUG
    if (CheckHitKey(KEY_INPUT_F2)) {
        DrawDebugInfo(cameraX);
    }
#endif
}

int NormalSlime::GetCurrentSprite()
{
    return GetSlimeSprite();
}

int NormalSlime::GetSlimeSprite()
{
    // **死亡状態または潰れた状態の場合は常にflatスプライト**
    if (currentState == DEAD || !active || slimeState == SLIME_FLATTENED) {
        return sprites.flat;
    }

    switch (slimeState) {
    case SLIME_RESTING:
        return sprites.idle;

    case SLIME_ACTIVE:
    case SLIME_JUMPING:
        if (currentState == WALKING || slimeState == SLIME_JUMPING) {
            return animationFrame ? sprites.walk_b : sprites.walk_a;
        }
        return sprites.idle;

    default:
        return sprites.idle;
    }
}

