#pragma once
#include "DxLib.h"
#include <vector>
#include <string>
#include <memory>

class BlockModeLoadingScene
{
public:
    BlockModeLoadingScene();
    ~BlockModeLoadingScene();

    void Initialize();
    void StartLoading(int selectedCharacter);
    void Update();
    void Draw();

    bool IsLoadingComplete() const { return loadingComplete; }
    void ResetState();

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // プログレスバー設定
    static const int PROGRESS_BAR_WIDTH = 800;
    static const int PROGRESS_BAR_HEIGHT = 30;
    static constexpr float LOADING_DURATION = 5.0f; // 5秒間のローディング（速度を調整）

    // エフェクト設定（ほのぼの調整）
    static constexpr float PARTICLE_SPAWN_RATE = 0.15f;  // より少ない頻度
    static constexpr int MAX_PARTICLES = 50;             // パーティクル数を減らす
    static constexpr float TITLE_PULSE_SPEED = 0.02f;    // ゆっくりしたパルス
    static constexpr float ICON_FLOAT_SPEED = 0.03f;     // ゆっくりした浮遊
    static constexpr float GLOW_PULSE_SPEED = 0.04f;

    // パーティクル構造体
    struct Particle {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        float scale;
        int color;
        float rotation;
        float rotationSpeed;
        int type; // 0=ブロック片, 1=スパークル, 2=コイン
    };

    // アニメーション要素構造体
    struct LoadingElement {
        float x, y;
        float targetX, targetY;
        float scale;
        float rotation;
        float alpha;
        float animationPhase;
        bool isVisible;
        int elementType; // 要素タイプ（ブロック、ツール等）
    };

    // テクスチャハンドル
    int backgroundHandle;
    int fontHandle, largeFontHandle, titleFontHandle;

    // ブロックアスレチック関連テクスチャ
    int dirtBlockHandle, stoneBlockHandle, crateBlockHandle;
    int coinHandle, explosiveHandle, playerIconHandle;
    int toolHammerHandle, toolPickaxeHandle;
    int progressBarBgHandle, progressBarFillHandle;

    // UI装飾
    int decorFrameHandle, glowEffectHandle;

    // 状態管理
    bool loadingComplete;
    float loadingProgress; // 0.0f ~ 1.0f
    float loadingTimer;
    int selectedCharacterIndex;
    std::string characterName;

    // エフェクト管理
    std::vector<Particle> particles;
    std::vector<LoadingElement> loadingElements;
    float particleSpawnTimer;
    float titlePulsePhase;
    float iconFloatPhase;
    float glowPulsePhase;

    // ローディングステージ（段階的演出用）
    enum LoadingStage {
        STAGE_INTRO,          // 導入（ブロック要素が登場）
        STAGE_BUILDING,       // 構築中（アスレチックコース組み立て）
        STAGE_FINALIZING,     // 仕上げ（エフェクト強化）
        STAGE_COMPLETE        // 完了
    };
    LoadingStage currentStage;
    float stageTimer;

    // プライベートメソッド
    void LoadTextures();
    void UpdateLoading();
    void UpdateParticles();
    void UpdateLoadingElements();
    void UpdateLoadingStage();

    void CreateParticles();
    void CreateBlockParticle(float x, float y, int blockType);
    void CreateSparkleParticle(float x, float y);
    void CreateCoinParticle(float x, float y);

    void InitializeLoadingElements();
    void UpdateElementAnimations();

    void DrawBackground();
    void DrawLoadingElements();
    void DrawProgressBar();
    void DrawTitleAndInfo();
    void DrawParticles();
    void DrawLoadingTips();
    void DrawCharacterInfo();
    void DrawStageInfo();
    void DrawEffects();

    void DrawGlowEffect(int x, int y, int width, int height, float intensity, int color);
    void DrawPulsingText(const std::string& text, int x, int y, int fontHandle, int color, float pulsePhase);
    void DrawAnimatedProgressBar();
    void DrawBlockConstructionAnimation();

    // ユーティリティ
    std::string GetCharacterDisplayName(int index);
    std::string GetRandomLoadingTip();
    float Lerp(float a, float b, float t);
    float EaseOutCubic(float t);
    float EaseInOutQuad(float t);
    int GetRandomColor();

    // ローディングメッセージ管理
    std::vector<std::string> loadingMessages;
    std::vector<std::string> loadingTips;
    int currentMessageIndex;
    float messageTimer;
    static constexpr float MESSAGE_DURATION = 1.0f;
};