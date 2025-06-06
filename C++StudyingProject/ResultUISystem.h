#pragma once
#include "DxLib.h"
#include <string>
#include <vector>

class ResultUISystem {
public:
    enum ResultState {
        RESULT_HIDDEN,
        RESULT_SHOWING,
        RESULT_VISIBLE,
        RESULT_HIDING
    };

    enum ButtonAction {
        BUTTON_NONE,
        BUTTON_RETRY,
        BUTTON_NEXT_STAGE
    };

    struct UIButton {
        int x, y, w, h;
        std::string label;
        float scale;
        float glowIntensity;
        bool hovered;
        bool enabled;
        float animProgress;
    };

    // **新追加: パーティクル構造体**
    struct ConfettiParticle {
        float x, y;           // 位置
        float vx, vy;         // 速度
        float rotation;       // 回転角度
        float rotationSpeed;  // 回転速度
        float life;           // 残り寿命
        float maxLife;        // 最大寿命
        float size;           // サイズ
        int color;            // 色
        bool active;          // アクティブフラグ
    };

    struct SparkleParticle {
        float x, y;           // 位置
        float vx, vy;         // 速度
        float life;           // 残り寿命
        float maxLife;        // 最大寿命
        float scale;          // スケール
        int color;            // 色
        bool active;          // アクティブフラグ
    };

    ResultUISystem();
    ~ResultUISystem();

    void Initialize();
    void Update();
    void Draw();

    // 表示制御
    void ShowResult(int starsCollected, int totalStars, int currentStage);
    void HideResult();
    void ResetState();
    bool IsVisible() const { return resultState == RESULT_VISIBLE; }
    bool IsShowing() const { return resultState == RESULT_SHOWING || resultState == RESULT_VISIBLE; }
    bool IsHidden() const { return resultState == RESULT_HIDDEN; }

    // ボタン操作
    ButtonAction GetClickedButton();
    void ResetButtonStates();

private:
    // テクスチャ
    int starOutlineTexture;
    int starFilledTexture;
    int buttonTexture;
    int fontHandle;
    int largeFontHandle;

    // UI状態
    ResultState resultState;
    float animationProgress;
    float starsAnimProgress;
    float buttonsAlpha;
    bool buttonsVisible;
    int displayedStars;
    int totalDisplayStars;
    int currentStageNumber;

    // ボタン
    UIButton retryButton;
    UIButton nextStageButton;

    // マウス入力
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // **新追加: クリエイティブエフェクト用変数**
    float titleExplosionProgress;    // タイトル爆発エフェクト進行度
    float confettiTimer;            // コンフェッティ生成タイマー
    float sparkleWavePhase;         // 煌めき波のフェーズ
    float backgroundRippleProgress; // 背景波紋エフェクト進行度
    float panelRotation;            // パネル回転角度
    float titleBouncePhase;         // タイトルバウンスフェーズ
    float starRainTimer;            // スターレイン生成タイマー
    float rainbowCyclePhase;        // レインボーサイクルフェーズ
    float scoreCounterProgress;     // スコアカウンター進行度
    int finalScoreDisplay;          // 表示スコア
    int targetScore;                // 目標スコア

    // **パーティクルシステム**
    std::vector<ConfettiParticle> confettiParticles;
    std::vector<SparkleParticle> sparkleParticles;

    // **クリエイティブエフェクト定数**
    static const int CONFETTI_COUNT = 100;
    static const int SPARKLE_COUNT = 50;

    static constexpr float EXPLOSION_SPEED = 0.025f;
    static constexpr float RIPPLE_SPEED = 0.02f;
    static constexpr float SPARKLE_WAVE_SPEED = 0.08f;
    static constexpr float PANEL_ROTATION_SPEED = 0.01f;
    static constexpr float TITLE_BOUNCE_SPEED = 0.06f;
    static constexpr float RAINBOW_CYCLE_SPEED = 0.03f;
    static constexpr float SCORE_COUNT_SPEED = 0.04f;
    static constexpr float CONFETTI_SPAWN_RATE = 0.1f;
    static constexpr float STAR_RAIN_RATE = 0.2f;

    // 従来のアニメーション定数（調整済み）
    static constexpr float ANIM_SPEED = 0.025f;
    static constexpr float STAR_ANIM_SPEED = 0.12f;
    static constexpr float BUTTON_FADE_SPEED = 0.035f;
    static constexpr float BUTTON_SCALE_NORMAL = 1.0f;
    static constexpr float BUTTON_SCALE_HOVER = 1.1f;

    // **新機能: クリエイティブ描画関数**
    void UpdateCreativeAnimations();
    void DrawEnhancedBackground();
    void DrawDynamicGradientBackground(float progress);
    void DrawRippleEffect();
    void DrawGeometricPattern(float progress);
    void DrawMainPanel();
    void DrawEnhancedTitle();
    void DrawTitleExplosion(int centerX, int centerY, int titleWidth);
    void DrawCreativeStars();
    void DrawStarEnergyRing(int centerX, int centerY, int baseSize, float progress, int starIndex);
    void DrawAdvancedCollectedStar(int x, int y, int baseSize, float progress, int starIndex);
    void DrawAdvancedUnCollectedStar(int x, int y, int baseSize, float progress);
    void DrawStarEnergyWaves(int centerX, int centerY, int size, float progress, int starIndex);
    void DrawInnerStarGlow(int centerX, int centerY, int size, int alpha);
    void DrawMagicalStarCount(int centerX, int y);
    void DrawMagicCircleBackground(int centerX, int centerY, float progress);
    void DrawScoreDisplay();
    void DrawScorePanel(int x, int y, int w, int h);
    void DrawEnhancedButtons();
    void DrawButtonHologram(const UIButton& button, float slideOffset, float delay);
    void DrawAdvancedButton(const UIButton& button, float alpha);
    void DrawEnergyBorder(int x, int y, int w, int h, float alpha);
    void DrawParticleEffects();
    void DrawMultiLayerGlow(int x, int y, int w, int h, float intensity);

    // **パーティクルシステム関数**
    void UpdateConfettiParticles();
    void UpdateSparkleParticles();
    void SpawnConfetti();
    void SpawnStarRain();
    void ResetCreativeEffects();

    // ヘルパー関数
    void LoadTextures();
    void UpdateAnimation();
    void UpdateButtons();
    void HSVtoRGB(float h, float s, float v, int& r, int& g, int& b);

    bool IsMouseOver(int x, int y, int w, int h) const;

    // **イージング関数（拡張）**
    float EaseOutBack(float t);
    float EaseOutBounce(float t);
    float EaseOutElastic(float t);
    float EaseOutCubic(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};