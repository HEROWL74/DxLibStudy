#pragma once
#include "DxLib.h"
#include "Player.h"
#include <vector>
#include <memory>

class CoinSystem {
public:
    // コインの状態
    enum CoinState {
        COIN_IDLE,        // 通常状態
        COIN_ATTRACTING,  // 吸い寄せ中
        COIN_COLLECTED    // 収集済み
    };

    // 個別のコイン
    struct Coin {
        float x, y;                 // 位置
        float originalX, originalY; // 元の位置
        float velocityX, velocityY; // 移動速度
        CoinState state;            // 状態
        float animationTimer;       // アニメーションタイマー
        float bobPhase;            // 浮遊効果のフェーズ
        float attractTimer;        // 吸い寄せタイマー
        float scale;               // スケール
        int alpha;                 // 透明度
        bool active;               // アクティブフラグ

        Coin(float posX, float posY)
            : x(posX), y(posY), originalX(posX), originalY(posY)
            , velocityX(0), velocityY(0), state(COIN_IDLE)
            , animationTimer(0), bobPhase(0), attractTimer(0)
            , scale(1.0f), alpha(255), active(true) {}
    };

    CoinSystem();
    ~CoinSystem();

    void Initialize();
    void Update(Player* player, float hudCoinX, float hudCoinY);
    void Draw(float cameraX);

    // コイン管理
    void AddCoin(float x, float y);
    void ClearAllCoins();
    int GetCollectedCoinsCount() const { return collectedCoinsCount; }
    void ResetCollectedCount() { collectedCoinsCount = 0; }

    // ステージ用の自動配置
    void GenerateCoinsForStage();

private:
    // テクスチャ
    int coinTexture;
    int sparkleTexture; // きらめき効果用（オプション）

    // コイン管理
    std::vector<std::unique_ptr<Coin>> coins;
    int collectedCoinsCount;

    // 物理・演出定数
    static constexpr float COIN_SIZE = 64.0f;           // コインサイズ（48→64に拡大）
    static constexpr float COLLECTION_DISTANCE = 80.0f; // 収集開始距離
    static constexpr float ATTRACT_DISTANCE = 120.0f;   // 吸い寄せ開始距離
    static constexpr float ATTRACT_SPEED = 4.0f;        // 吸い寄せ速度（8.0→4.0に減速）
    static constexpr float BOB_SPEED = 0.04f;           // 浮遊速度
    static constexpr float BOB_AMPLITUDE = 8.0f;        // 浮遊振幅
    static constexpr float ROTATE_SPEED = 0.02f;        // 回転速度（アニメーション用）

    // ヘルパー関数
    void UpdateCoin(Coin& coin, float playerX, float playerY, float hudCoinX, float hudCoinY);
    void UpdateCoinPhysics(Coin& coin, float hudCoinX, float hudCoinY);
    void UpdateCoinAnimation(Coin& coin);
    void DrawCoin(const Coin& coin, float cameraX);
    void DrawCollectionEffect(const Coin& coin, float cameraX);

    float GetDistance(float x1, float y1, float x2, float y2);
    float Lerp(float a, float b, float t);
    void LoadTextures();
};