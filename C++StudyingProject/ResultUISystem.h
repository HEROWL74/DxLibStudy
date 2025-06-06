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

    // **�V�ǉ�: �p�[�e�B�N���\����**
    struct ConfettiParticle {
        float x, y;           // �ʒu
        float vx, vy;         // ���x
        float rotation;       // ��]�p�x
        float rotationSpeed;  // ��]���x
        float life;           // �c�����
        float maxLife;        // �ő����
        float size;           // �T�C�Y
        int color;            // �F
        bool active;          // �A�N�e�B�u�t���O
    };

    struct SparkleParticle {
        float x, y;           // �ʒu
        float vx, vy;         // ���x
        float life;           // �c�����
        float maxLife;        // �ő����
        float scale;          // �X�P�[��
        int color;            // �F
        bool active;          // �A�N�e�B�u�t���O
    };

    ResultUISystem();
    ~ResultUISystem();

    void Initialize();
    void Update();
    void Draw();

    // �\������
    void ShowResult(int starsCollected, int totalStars, int currentStage);
    void HideResult();
    void ResetState();
    bool IsVisible() const { return resultState == RESULT_VISIBLE; }
    bool IsShowing() const { return resultState == RESULT_SHOWING || resultState == RESULT_VISIBLE; }
    bool IsHidden() const { return resultState == RESULT_HIDDEN; }

    // �{�^������
    ButtonAction GetClickedButton();
    void ResetButtonStates();

private:
    // �e�N�X�`��
    int starOutlineTexture;
    int starFilledTexture;
    int buttonTexture;
    int fontHandle;
    int largeFontHandle;

    // UI���
    ResultState resultState;
    float animationProgress;
    float starsAnimProgress;
    float buttonsAlpha;
    bool buttonsVisible;
    int displayedStars;
    int totalDisplayStars;
    int currentStageNumber;

    // �{�^��
    UIButton retryButton;
    UIButton nextStageButton;

    // �}�E�X����
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // **�V�ǉ�: �N���G�C�e�B�u�G�t�F�N�g�p�ϐ�**
    float titleExplosionProgress;    // �^�C�g�������G�t�F�N�g�i�s�x
    float confettiTimer;            // �R���t�F�b�e�B�����^�C�}�[
    float sparkleWavePhase;         // ���߂��g�̃t�F�[�Y
    float backgroundRippleProgress; // �w�i�g��G�t�F�N�g�i�s�x
    float panelRotation;            // �p�l����]�p�x
    float titleBouncePhase;         // �^�C�g���o�E���X�t�F�[�Y
    float starRainTimer;            // �X�^�[���C�������^�C�}�[
    float rainbowCyclePhase;        // ���C���{�[�T�C�N���t�F�[�Y
    float scoreCounterProgress;     // �X�R�A�J�E���^�[�i�s�x
    int finalScoreDisplay;          // �\���X�R�A
    int targetScore;                // �ڕW�X�R�A

    // **�p�[�e�B�N���V�X�e��**
    std::vector<ConfettiParticle> confettiParticles;
    std::vector<SparkleParticle> sparkleParticles;

    // **�N���G�C�e�B�u�G�t�F�N�g�萔**
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

    // �]���̃A�j���[�V�����萔�i�����ς݁j
    static constexpr float ANIM_SPEED = 0.025f;
    static constexpr float STAR_ANIM_SPEED = 0.12f;
    static constexpr float BUTTON_FADE_SPEED = 0.035f;
    static constexpr float BUTTON_SCALE_NORMAL = 1.0f;
    static constexpr float BUTTON_SCALE_HOVER = 1.1f;

    // **�V�@�\: �N���G�C�e�B�u�`��֐�**
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

    // **�p�[�e�B�N���V�X�e���֐�**
    void UpdateConfettiParticles();
    void UpdateSparkleParticles();
    void SpawnConfetti();
    void SpawnStarRain();
    void ResetCreativeEffects();

    // �w���p�[�֐�
    void LoadTextures();
    void UpdateAnimation();
    void UpdateButtons();
    void HSVtoRGB(float h, float s, float v, int& r, int& g, int& b);

    bool IsMouseOver(int x, int y, int w, int h) const;

    // **�C�[�W���O�֐��i�g���j**
    float EaseOutBack(float t);
    float EaseOutBounce(float t);
    float EaseOutElastic(float t);
    float EaseOutCubic(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};