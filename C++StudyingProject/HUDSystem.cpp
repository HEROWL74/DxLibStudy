#include "HUDSystem.h"
#include <algorithm>
#include <math.h>

// HUDSystem.cpp のコンストラクタに追加
HUDSystem::HUDSystem()
    : maxLife(6)          // 3ハート × 2 = 6ライフ
    , currentLife(6)      // 初期値は満タン
    , coins(0)            // 初期コイン数
    , collectedStars(0)   // **初期星数（新機能）**
    , totalStars(3)       // **総星数（新機能）**
    , currentPlayerCharacter(0) // デフォルトはbeige
    , hudX(30)            // 左上から30ピクセル（少し余裕を持たせる）
    , hudY(30)            // 上から30ピクセル（少し余裕を持たせる）
    , visible(true)       // 初期状態で表示
    , previousLife(6)     // **前フレームのライフ（ダメージ検出用）**
    , heartShakeTimer(0.0f) // **ハート揺れタイマー**
    , heartShakeIntensity(0.0f) // **ハート揺れ強度**
    , heartShakePhase(0.0f)     // **ハート揺れ位相**
{
    // テクスチャハンドルを初期化
    heartTextures.full = heartTextures.half = heartTextures.empty = -1;
    playerIconTextures.beige = playerIconTextures.green = playerIconTextures.pink = -1;
    playerIconTextures.purple = playerIconTextures.yellow = -1;
    coinTextures.coin = -1;

    // **星テクスチャ初期化（新機能）**
    starTextures.starOutline = starTextures.starFilled = -1;

    for (int i = 0; i < 10; i++) {
        coinTextures.numbers[i] = -1;
    }
}

HUDSystem::~HUDSystem()
{
    // ハートテクスチャの解放
    if (heartTextures.full != -1) DeleteGraph(heartTextures.full);
    if (heartTextures.half != -1) DeleteGraph(heartTextures.half);
    if (heartTextures.empty != -1) DeleteGraph(heartTextures.empty);

    // プレイヤーアイコンテクスチャの解放
    if (playerIconTextures.beige != -1) DeleteGraph(playerIconTextures.beige);
    if (playerIconTextures.green != -1) DeleteGraph(playerIconTextures.green);
    if (playerIconTextures.pink != -1) DeleteGraph(playerIconTextures.pink);
    if (playerIconTextures.purple != -1) DeleteGraph(playerIconTextures.purple);
    if (playerIconTextures.yellow != -1) DeleteGraph(playerIconTextures.yellow);

    // コインテクスチャの解放
    if (coinTextures.coin != -1) DeleteGraph(coinTextures.coin);

    // **星テクスチャの解放（新機能）**
    if (starTextures.starOutline != -1) DeleteGraph(starTextures.starOutline);
    if (starTextures.starFilled != -1) DeleteGraph(starTextures.starFilled);

    for (int i = 0; i < 10; i++) {
        if (coinTextures.numbers[i] != -1) {
            DeleteGraph(coinTextures.numbers[i]);
        }
    }
}

void HUDSystem::Initialize()
{
    LoadTextures();
}

// LoadTextures関数に追加
void HUDSystem::LoadTextures()
{
    // ハートテクスチャの読み込み
    heartTextures.full = LoadGraph("Sprites/Tiles/hud_heart.png");
    heartTextures.half = LoadGraph("Sprites/Tiles/hud_heart_half.png");
    heartTextures.empty = LoadGraph("Sprites/Tiles/hud_heart_empty.png");

    // プレイヤーアイコンテクスチャの読み込み
    playerIconTextures.beige = LoadGraph("Sprites/Tiles/hud_player_beige.png");
    playerIconTextures.green = LoadGraph("Sprites/Tiles/hud_player_green.png");
    playerIconTextures.pink = LoadGraph("Sprites/Tiles/hud_player_pink.png");
    playerIconTextures.purple = LoadGraph("Sprites/Tiles/hud_player_purple.png");
    playerIconTextures.yellow = LoadGraph("Sprites/Tiles/hud_player_yellow.png");

    // コインテクスチャの読み込み
    coinTextures.coin = LoadGraph("Sprites/Tiles/hud_coin.png");

    // **星テクスチャの読み込み（新機能）**
    starTextures.starOutline = LoadGraph("UI/PNG/Yellow/star_outline_depth.png");
    starTextures.starFilled = LoadGraph("UI/PNG/Yellow/star.png");

    // 数字テクスチャの読み込み (0-9)
    for (int i = 0; i < 10; i++) {
        std::string numberPath = "Sprites/Tiles/hud_character_" + std::to_string(i) + ".png";
        coinTextures.numbers[i] = LoadGraph(numberPath.c_str());
    }
}

void HUDSystem::Update()
{
    // **ダメージ検出とハート揺れ開始**
    if (currentLife < previousLife) {
        // ライフが減った場合、ハート揺れを開始
        heartShakeTimer = 0.0f;
        heartShakeIntensity = 1.0f;
        heartShakePhase = 0.0f;

        // デバッグ出力
        OutputDebugStringA("HUDSystem: Player took damage! Starting heart shake animation.\n");
    }

    // 前フレームのライフを更新
    previousLife = currentLife;

    // **ハート揺れアニメーションの更新**
    if (heartShakeIntensity > 0.0f) {
        heartShakeTimer += 0.016f; // 60FPS想定
        heartShakePhase += 0.4f;   // 揺れの速度

        // 揺れ強度の減衰（1秒で完全に停止）
        heartShakeIntensity = max(0.0f, 1.0f - (heartShakeTimer / HEART_SHAKE_DURATION));

        if (heartShakeIntensity <= 0.0f) {
            heartShakeTimer = 0.0f;
            heartShakePhase = 0.0f;
        }
    }
}

// Draw関数に星描画を追加
void HUDSystem::Draw()
{
    if (!visible) return;

    // HUD要素を順番に描画
    DrawHearts();      // ライフハート（揺れ機能付き）
    DrawPlayerIcon();  // プレイヤーアイコン
    DrawCoins();       // コイン表示
    DrawStars();       // **星表示（新機能）**
}

void HUDSystem::DrawHearts()
{
    int heartsCount = 3; // 常に3つのハートを表示
    int heartY = hudY;

    for (int i = 0; i < heartsCount; i++) {
        int heartX = hudX + (i * (HEART_SIZE + 12)); // 12ピクセルの間隔（拡大）
        HeartState state = GetHeartState(i);

        int textureHandle = -1;
        switch (state) {
        case HEART_FULL:
            textureHandle = heartTextures.full;
            break;
        case HEART_HALF:
            textureHandle = heartTextures.half;
            break;
        case HEART_EMPTY:
            textureHandle = heartTextures.empty;
            break;
        }

        if (textureHandle != -1) {
            // **ハート揺れエフェクトの計算**
            float shakeOffsetX = 0.0f;
            float shakeOffsetY = 0.0f;

            if (heartShakeIntensity > 0.0f) {
                // 複数方向のランダムな揺れ
                float shakeAmount = heartShakeIntensity * HEART_SHAKE_AMOUNT;

                // X軸の揺れ（高周波）
                shakeOffsetX = sinf(heartShakePhase * 18.0f + i * 0.5f) * shakeAmount;

                // Y軸の揺れ（少し低い周波数）
                shakeOffsetY = cosf(heartShakePhase * 15.0f + i * 0.3f) * shakeAmount * 0.7f;

                // 揺れのバリエーションを各ハートで少し変える
                if (i == 1) {
                    shakeOffsetX *= 1.2f;
                    shakeOffsetY *= 0.8f;
                }
                else if (i == 2) {
                    shakeOffsetX *= 0.9f;
                    shakeOffsetY *= 1.1f;
                }
            }

            // 滑らかな拡大表示（揺れ位置を適用）
            int finalHeartX = heartX + (int)shakeOffsetX;
            int finalHeartY = heartY + (int)shakeOffsetY;

            DrawExtendGraph(
                finalHeartX, finalHeartY,
                finalHeartX + HEART_SIZE, finalHeartY + HEART_SIZE,
                textureHandle, TRUE
            );

            // **ダメージ時の追加エフェクト（オプション）**
            if (heartShakeIntensity > 0.5f && state != HEART_EMPTY) {
                // 赤い光のエフェクト
                SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(50 * heartShakeIntensity));
                DrawExtendGraph(
                    finalHeartX - 2, finalHeartY - 2,
                    finalHeartX + HEART_SIZE + 2, finalHeartY + HEART_SIZE + 2,
                    textureHandle, TRUE
                );
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            }
        }
    }
}

void HUDSystem::DrawPlayerIcon()
{
    int iconX = hudX;
    int iconY = hudY + HEART_SIZE + ELEMENT_SPACING; // ハートの下に配置

    int iconHandle = GetPlayerIconHandle();
    if (iconHandle != -1) {
        // 滑らかな拡大表示
        DrawExtendGraph(
            iconX, iconY,
            iconX + PLAYER_ICON_SIZE, iconY + PLAYER_ICON_SIZE,
            iconHandle, TRUE
        );
    }
}

void HUDSystem::DrawCoins()
{
    int coinStartX = hudX + PLAYER_ICON_SIZE + ELEMENT_SPACING; // プレイヤーアイコンの右
    int coinY = hudY + HEART_SIZE + ELEMENT_SPACING + (PLAYER_ICON_SIZE - COIN_ICON_SIZE) / 2; // 中央揃え

    // コインアイコンを描画（拡大表示）
    if (coinTextures.coin != -1) {
        DrawExtendGraph(
            coinStartX, coinY,
            coinStartX + COIN_ICON_SIZE, coinY + COIN_ICON_SIZE,
            coinTextures.coin, TRUE
        );
    }

    // コイン数を描画
    int numberX = coinStartX + COIN_ICON_SIZE + 12; // コインアイコンから12ピクセル右（拡大）
    int numberY = coinY + (COIN_ICON_SIZE - NUMBER_SIZE) / 2; // 中央揃え

    DrawNumber(coins, numberX, numberY);
}

void HUDSystem::DrawNumber(int number, int x, int y)
{
    // 数字を文字列に変換
    std::string numberStr = std::to_string(number);

    // 最低でも1桁は表示（0の場合）
    if (numberStr.empty()) {
        numberStr = "0";
    }

    // 各桁を描画（拡大表示）
    for (size_t i = 0; i < numberStr.length(); i++) {
        int digit = numberStr[i] - '0'; // 文字を数字に変換

        if (digit >= 0 && digit <= 9 && coinTextures.numbers[digit] != -1) {
            int digitX = x + (i * (NUMBER_SIZE - 6)); // 数字間の間隔を調整（拡大版）

            // 滑らかな拡大表示
            DrawExtendGraph(
                digitX, y,
                digitX + NUMBER_SIZE, y + NUMBER_SIZE,
                coinTextures.numbers[digit], TRUE
            );
        }
    }
}

HUDSystem::HeartState HUDSystem::GetHeartState(int heartIndex) const
{
    // 各ハートは2ライフ分を表す
    int heartLife = currentLife - (heartIndex * 2);

    if (heartLife >= 2) {
        return HEART_FULL;
    }
    else if (heartLife == 1) {
        return HEART_HALF;
    }
    else {
        return HEART_EMPTY;
    }
}

int HUDSystem::GetPlayerIconHandle() const
{
    switch (currentPlayerCharacter) {
    case 0: return playerIconTextures.beige;
    case 1: return playerIconTextures.green;
    case 2: return playerIconTextures.pink;
    case 3: return playerIconTextures.purple;
    case 4: return playerIconTextures.yellow;
    default: return playerIconTextures.beige;
    }
}

std::string HUDSystem::GetCharacterColorName(int characterIndex) const
{
    switch (characterIndex) {
    case 0: return "beige";
    case 1: return "green";
    case 2: return "pink";
    case 3: return "purple";
    case 4: return "yellow";
    default: return "beige";
    }
}

void HUDSystem::SetCurrentLife(int currentLife)
{
    this->currentLife = max(0, min(currentLife, maxLife));
}

void HUDSystem::AddLife(int amount)
{
    currentLife = min(currentLife + amount, maxLife);
}

void HUDSystem::SubtractLife(int amount)
{
    currentLife = max(0, currentLife - amount);
}

void HUDSystem::SetPlayerCharacter(int characterIndex)
{
    if (characterIndex >= 0 && characterIndex <= 4) {
        currentPlayerCharacter = characterIndex;
    }
}

void HUDSystem::SetPlayerCharacter(const std::string& colorName)
{
    if (colorName == "beige") currentPlayerCharacter = 0;
    else if (colorName == "green") currentPlayerCharacter = 1;
    else if (colorName == "pink") currentPlayerCharacter = 2;
    else if (colorName == "purple") currentPlayerCharacter = 3;
    else if (colorName == "yellow") currentPlayerCharacter = 4;
    else currentPlayerCharacter = 0; // デフォルト
}

void HUDSystem::DrawStars()
{
    int starStartX = hudX + PLAYER_ICON_SIZE + ELEMENT_SPACING +
        COIN_ICON_SIZE + 60 + ELEMENT_SPACING; // コインの右側
    int starY = hudY + HEART_SIZE + ELEMENT_SPACING + (PLAYER_ICON_SIZE - STAR_SIZE) / 2; // 中央揃え

    for (int i = 0; i < totalStars; i++) {
        int starX = starStartX + i * (STAR_SIZE + 8); // 8ピクセル間隔

        // 星のテクスチャ選択
        int starTexture = (i < collectedStars) ? starTextures.starFilled : starTextures.starOutline;

        if (starTexture != -1) {
            // 滑らかな拡大表示
            DrawExtendGraph(
                starX, starY,
                starX + STAR_SIZE, starY + STAR_SIZE,
                starTexture, TRUE
            );
        }
    }
}