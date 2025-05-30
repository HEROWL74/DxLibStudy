#pragma once
#include "DxLib.h"
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>
#include <random>

class TitleScene
{
public:
    TitleScene();
    ~TitleScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsStartRequested() const { return startRequested; }
    bool IsExitRequested()  const { return exitRequested; }
    bool IsTransitionComplete() const { return transitionState == Complete; }
    void ResetTransition() { transitionState = None; transitionFadeProgress = 0.0f; }

private:
    // 画面 & ボタンサイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;
    static const int BUTTON_W = 300;
    static const int BUTTON_H = 100;

    // エフェクト定数
    static constexpr float HOVER_SCALE = 1.08f;
    static constexpr float NORMAL_SCALE = 1.0f;
    static constexpr float SCALE_LERP = 0.18f;
    static constexpr float FLOAT_SPEED = 0.08f;
    static constexpr float FLOAT_AMP = 6.0f;
    static constexpr float ANIM_SPEED = 0.08f;  // より滑らかなアニメーション
    static constexpr float FADE_SPEED = 0.06f;  // フェード専用速度
    static constexpr float TRANSITION_SPEED = 0.04f;  // 画面遷移フェード速度

    // イージング関数用
    enum EaseType { EASE_IN_OUT_CUBIC, EASE_OUT_ELASTIC, EASE_IN_OUT_QUART };

    enum OptionState { Hidden, Showing, Visible, Hiding };
    enum TransitionState { None, FadingOut, FadingIn, Complete };

    // パーティクル構造体
    struct Particle {
        float x, y;
        float vx, vy;
        float life;
        float maxLife;
        float size;
        int color;
        bool active;
    };

    struct UIButton {
        int   x, y, w, h;
        std::string label;
        float scale;
        float floatPhase;
        bool  hovered;
        float glowIntensity;    // グロー効果用
        float pressProgress;    // プレスエフェクト用
    };

    // スライダー改良用構造体
    struct EnhancedSlider {
        int x, y, w, h;
        float value;
        bool dragging;
        float hoverProgress;    // ホバー時の拡大効果
        float valueDisplay;     // 表示用の補間された値
        float glowEffect;       // グロー効果
    };

    // テクスチャハンドル
    int backgroundHandle;
    int buttonHandle;
    int slideHandle;
    int slideBarHandle;     // スライダーバー用画像
    int homeHandle;
    int fontHandle;
    int largeFontHandle;    // タイトル用大きなフォント

    // START/OPTIONS/EXIT ボタン
    std::vector<UIButton> buttons;

    // 改良されたスライダー
    EnhancedSlider slider;

    // 状態管理
    bool        startRequested;
    bool        exitRequested;
    OptionState optionState;
    TransitionState transitionState;     // 画面遷移状態
    float       optionAnimProgress;
    float       optionFadeProgress;      // オプション画面のフェード用
    float       optionSlideProgress;     // オプション画面のスライド用
    float       backgroundFadeProgress;  // 背景フェード用
    float       transitionFadeProgress;  // 画面遷移フェード用
    float       titlePulsePhase;         // タイトルの脈動効果

    // マウス
    int  mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // キーボード入力
    bool backspacePressed, backspacePressedPrev;

    // パーティクルシステム
    std::vector<Particle> particles;
    std::mt19937 randomEngine;
    float particleSpawnTimer;

    // ヘルパー関数
    void UpdateButtons();
    void UpdateParticles();
    void UpdateSlider();
    void SpawnParticle(float x, float y);
    void DrawEnhancedButton(const UIButton& btn);
    void DrawEnhancedSlider();
    void DrawParticles();
    void DrawGlowEffect(int x, int y, int w, int h, float intensity, int color);

    bool IsMouseOver(int x, int y, int w, int h) const;
    bool IsClicked(int x, int y, int w, int h) const;

    // イージング関数
    float EaseInOutCubic(float t);
    float EaseOutElastic(float t);
    float EaseInOutQuart(float t);
    float ApplyEasing(float t, EaseType type);

    // ユーティリティ
    float Lerp(float a, float b, float t);
    float SmoothStep(float edge0, float edge1, float x);
    int BlendColor(int color1, int color2, float ratio);
};