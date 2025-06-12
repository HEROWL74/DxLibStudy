#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include "HUDSystem.h"
#include "SoundManager.h"
#include <vector>
#include <string>

class TutorialScene
{
public:
    TutorialScene();
    ~TutorialScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }
    bool IsCompleted() const { return tutorialCompleted; }

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // チュートリアル段階
    enum TutorialStep {
        STEP_WELCOME,       // ようこそ画面
        STEP_MOVEMENT,      // 移動操作
        STEP_JUMPING,       // ジャンプ操作
        STEP_DUCKING,       // しゃがみ操作
        STEP_SLIDING,       // スライディング操作
        STEP_GOAL,          // ゴール到達
        STEP_COMPLETED      // 完了
    };

    // チュートリアルテキスト構造体
    struct TutorialText {
        std::string mainText;
        std::string subText;
        std::string inputText;
        float displayTime;
        bool isCompleted;
    };

    // **新追加: エフェクト用構造体**
    struct Particle {
        float x, y;
        float velocityX, velocityY;
        float life;
        float maxLife;
        int color;
        float scale;
    };

    struct FloatingText {
        std::string text;
        float x, y;
        float velocityY;
        float life;
        float maxLife;
        int color;
        bool isImportant;
    };

    // テクスチャハンドル
    int backgroundHandle;
    int fontHandle;
    int largeFontHandle;
    int smallFontHandle;  // **新追加: 小さいフォント**

    // ゲームオブジェクト
    Player tutorialPlayer;
    StageManager stageManager;
    HUDSystem hudSystem;

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterName;

    // カメラシステム
    float cameraX;
    float previousPlayerX;
    static constexpr float CAMERA_FOLLOW_SPEED = 0.08f;

    // 状態管理
    bool exitRequested;
    bool tutorialCompleted;
    TutorialStep currentStep;
    float stepTimer;
    float messageTimer;
    bool messageVisible;

    // 入力管理
    bool escPressed, escPressedPrev;
    bool leftPressed, leftPressedPrev;
    bool rightPressed, rightPressedPrev;
    bool spacePressed, spacePressedPrev;
    bool downPressed, downPressedPrev;
    bool rPressed, rPressedPrev;

    // チュートリアルテキスト
    std::vector<TutorialText> tutorialTexts;

    // プレイヤー位置チェック用
    float startX;
    float goalX;
    bool hasMovedLeft;
    bool hasMovedRight;
    bool hasJumped;
    bool hasDucked;
    bool hasSlid;

    // フェード効果
    float fadeAlpha;
    bool fadingIn;
    bool fadingOut;

    // **新追加: エフェクトシステム**
    std::vector<Particle> particles;
    std::vector<FloatingText> floatingTexts;
    float successEffectTimer;
    bool showingSuccessEffect;

    // **新追加: UI強化要素**
    float uiPulseTimer;
    float iconRotation;
    float progressBarGlow;
    bool stepJustChanged;
    float stepChangeEffectTimer;

    // **新追加: 入力視覚化**
    struct InputIndicator {
        bool isActive;
        float intensity;
        float timer;
    };
    InputIndicator leftIndicator, rightIndicator, spaceIndicator, downIndicator;

    // ヘルパー関数
    void InitializeTutorialTexts();
    void InitializeTutorialStage();
    void UpdateInput();
    void UpdateCamera();
    void UpdateTutorialLogic();
    void UpdateFade();
    void CheckStepCompletion();

    // **新追加: エフェクト関数**
    void UpdateEffects();
    void UpdateParticles();
    void UpdateFloatingTexts();
    void UpdateInputIndicators();
    void UpdateUIEffects();

    void CreateSuccessParticles(float x, float y);
    void CreateStepChangeEffect();
    void AddFloatingText(const std::string& text, float x, float y, int color, bool important = false);

    void DrawBackground();
    void DrawTutorialUI();
    void DrawInstructions();
    void DrawStepIndicator();
    void DrawFade();

    // **新追加: エフェクト描画**
    void DrawEffects();
    void DrawParticles();
    void DrawFloatingTexts();
    void DrawInputIndicators();
    void DrawEnhancedUI();

    std::string GetCharacterDisplayName(int index);
    void NextStep();
    void CompleteTutorial();

    // **新追加: UI強化ヘルパー**
    void DrawGlowingBox(int x1, int y1, int x2, int y2, int color, float glowIntensity);
    void DrawPulsingText(const std::string& text, int x, int y, int color, int fontHandle, float pulseIntensity);
    int GetPulsingAlpha(float baseAlpha, float pulseSpeed);

    // ユーティリティ関数
    float Lerp(float a, float b, float t);
    bool IsPlayerInRange(float minX, float maxX);
};