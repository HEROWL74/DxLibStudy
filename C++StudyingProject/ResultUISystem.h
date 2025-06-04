#pragma once
#include "DxLib.h"
#include <string>

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

    ResultUISystem();
    ~ResultUISystem();

    void Initialize();
    void Update();
    void Draw();

    // 表示制御
    void ShowResult(int starsCollected, int totalStars, int currentStage);
    void HideResult();
    void ResetState();  // **状態完全リセット機能を追加**
    bool IsVisible() const { return resultState == RESULT_VISIBLE; }
    bool IsShowing() const { return resultState == RESULT_SHOWING || resultState == RESULT_VISIBLE; }
    bool IsHidden() const { return resultState == RESULT_HIDDEN; }  // **完全非表示確認用**

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
    float buttonsAlpha;          // **ボタンのフェード透明度（新機能）**
    bool buttonsVisible;         // **ボタン表示フラグ（新機能）**
    int displayedStars;
    int totalDisplayStars;
    int currentStageNumber;

    // ボタン
    UIButton retryButton;
    UIButton nextStageButton;

    // マウス入力
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // アニメーション定数（ゆっくりに調整）
    static constexpr float ANIM_SPEED = 0.02f;          // 0.04f → 0.02f（半分の速度）
    static constexpr float STAR_ANIM_SPEED = 0.15f;     // 0.3f → 0.15f（半分の速度）
    static constexpr float BUTTON_FADE_SPEED = 0.03f;   // 0.06f → 0.03f（半分の速度）
    static constexpr float BUTTON_SCALE_NORMAL = 1.0f;
    static constexpr float BUTTON_SCALE_HOVER = 1.1f;

    // ヘルパー関数
    void LoadTextures();
    void UpdateAnimation();
    void UpdateButtons();
    void DrawBackground();
    void DrawTitle();
    void DrawStars();
    void DrawButtons();
    void DrawButton(const UIButton& button);
    void DrawButtonWithAlpha(const UIButton& button, int alpha);
    void DrawGlowEffect(int x, int y, int w, int h, float intensity, int color);

    // **シンプルになった星アニメーション関数**
    void DrawCollectedStarAnimation(int x, int y, int baseSize, float progress, int starIndex);
    void DrawUnCollectedStar(int x, int y, int baseSize, float progress);

    bool IsMouseOver(int x, int y, int w, int h) const;
    float EaseOutBack(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};