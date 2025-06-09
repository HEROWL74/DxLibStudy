#pragma once
#include "DxLib.h"
#include "SoundManager.h"
#include <vector>

class SplashScene
{
public:
    SplashScene();
    ~SplashScene();

    void Initialize();
    void Update();
    void Draw();
    void Finalize();

    bool IsTransitionComplete() const { return transitionComplete; }
    void ResetTransition() { transitionComplete = false; }

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // フェーズ管理
    enum SplashPhase {
        PHASE_LOGO_FADEIN,      // ロゴフェードイン
        PHASE_LOGO_DISPLAY,     // ロゴ表示
        PHASE_EXPLOSION_PREP,   // 爆発準備
        PHASE_EXPLOSION,        // 爆発エフェクト
        PHASE_SLIDE_UP,         // スライドアップトランジション
        PHASE_COMPLETE          // 完了
    };

    SplashPhase currentPhase;
    float phaseTimer;

    // リソース
    int logoHandle;           // スタジオロゴ（適当な画像を使用）
    int bombHandle;           // bomb.png
    int bombActiveHandle;     // bomb_active.png
    int fontHandle;           // フォント

    // ロゴ表示用
    float logoAlpha;
    float logoScale;

    // 爆発エフェクト用
    struct ExplosionParticle {
        float x, y;           // 位置
        float vx, vy;         // 速度
        float life;           // 寿命
        float maxLife;        // 最大寿命
        float size;           // サイズ
        int color;            // 色
        bool active;          // アクティブ状態
    };

    std::vector<ExplosionParticle> explosionParticles;
    float explosionTimer;
    bool explosionStarted;
    int bombAnimFrame;        // ボム画像のアニメーション用

    // スライドアップトランジション用
    float slideOffset;        // スライドのオフセット
    bool transitionComplete;

    // タイミング定数
    static constexpr float LOGO_FADEIN_DURATION = 1.0f;
    static constexpr float LOGO_DISPLAY_DURATION = 2.0f;
    static constexpr float EXPLOSION_PREP_DURATION = 0.5f;
    static constexpr float EXPLOSION_DURATION = 1.5f;
    static constexpr float SLIDE_UP_DURATION = 1.0f;

    // 内部関数
    void UpdateLogoFadeIn();
    void UpdateLogoDisplay();
    void UpdateExplosionPrep();
    void UpdateExplosion();
    void UpdateSlideUp();

    void DrawLogo();
    void DrawExplosion();
    void DrawSlideTransition();

    void InitializeExplosion();
    void CreateExplosionParticles(float centerX, float centerY);
    void UpdateExplosionParticles();
};