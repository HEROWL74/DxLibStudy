#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <vector>
#include <memory>

class StarSystem {
public:
    // 星の状態
    enum StarState {
        STAR_IDLE,        // 通常状態
        STAR_ATTRACTING,  // 引き寄せ中
        STAR_COLLECTED    // 収集済み
    };

    // 個別の星
    struct Star {
        float x, y;                 // 位置
        float originalX, originalY; // 元の位置
        StarState state;            // 状態
        float animationTimer;       // アニメーションタイマー
        float bobPhase;            // 浮遊効果のフェーズ
        float attractTimer;        // 引き寄せタイマー
        float scale;               // スケール
        float rotation;            // 回転角度
        int alpha;                 // 透明度
        bool active;               // アクティブフラグ
        bool collected;            // 収集済みフラグ

        // **収集アニメーション用の変数**
        float collectionPhase;     // 収集アニメーションの進行度
        float burstIntensity;      // バースト効果の強度
        int sparkleCount;          // キラキラの数

        Star(float posX, float posY)
            : x(posX), y(posY), originalX(posX), originalY(posY)
            , state(STAR_IDLE), animationTimer(0), bobPhase(0), attractTimer(0)
            , scale(1.0f), rotation(0.0f), alpha(255), active(true), collected(false)
            , collectionPhase(0.0f), burstIntensity(0.0f), sparkleCount(0) {}
    };

    StarSystem();
    ~StarSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // 星管理
    void AddStar(float x, float y);
    void ClearAllStars();
    int GetCollectedStarsCount() const { return collectedStarsCount; }
    void ResetCollectedCount() { collectedStarsCount = 0; }

    // ステージ特化の星配置メソッド
    void GenerateStarsForStageIndex(int stageIndex);
    void GenerateStarsForGrassStage();
    void GenerateStarsForStoneStage();
    void GenerateStarsForSandStage();
    void GenerateStarsForSnowStage();
    void GenerateStarsForPurpleStage();

private:
    // テクスチャ
    int starTexture;
    int sparkleTexture;

    // 星管理
    std::vector<std::unique_ptr<Star>> stars;
    int collectedStarsCount;

    // 物理・演出定数
    static constexpr float STAR_SIZE = 64.0f;           // 星サイズ
    static constexpr float COLLECTION_DISTANCE = 100.0f; // 収集開始距離
    static constexpr float ATTRACT_DISTANCE = 150.0f;   // 引き寄せ開始距離
    static constexpr float BOB_SPEED = 0.03f;           // 浮遊速度
    static constexpr float BOB_AMPLITUDE = 12.0f;       // 浮遊振幅
    static constexpr float ROTATE_SPEED = 0.04f;        // 回転速度
    static constexpr float SPARKLE_DURATION = 2.0f;     // 点滅+パーティクル継続時間（3回点滅用）

    // ヘルパー関数
    void UpdateStar(Star& star, float playerX, float playerY);
    void UpdateStarPhysics(Star& star);
    void UpdateStarAnimation(Star& star);
    void DrawStar(const Star& star, float cameraX);
    void DrawCollectionEffect(const Star& star, float cameraX);

    // **収集エフェクト関数**
    void DrawCollectionBurst(const Star& star, int screenX, int screenY, float cameraX);

    float GetDistance(float x1, float y1, float x2, float y2);
    void LoadTextures();
};