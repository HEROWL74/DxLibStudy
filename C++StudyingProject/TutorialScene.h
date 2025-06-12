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
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �`���[�g���A���i�K
    enum TutorialStep {
        STEP_WELCOME,       // �悤�������
        STEP_MOVEMENT,      // �ړ�����
        STEP_JUMPING,       // �W�����v����
        STEP_DUCKING,       // ���Ⴊ�ݑ���
        STEP_SLIDING,       // �X���C�f�B���O����
        STEP_GOAL,          // �S�[�����B
        STEP_COMPLETED      // ����
    };

    // �`���[�g���A���e�L�X�g�\����
    struct TutorialText {
        std::string mainText;
        std::string subText;
        std::string inputText;
        float displayTime;
        bool isCompleted;
    };

    // **�V�ǉ�: �G�t�F�N�g�p�\����**
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

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int fontHandle;
    int largeFontHandle;
    int smallFontHandle;  // **�V�ǉ�: �������t�H���g**

    // �Q�[���I�u�W�F�N�g
    Player tutorialPlayer;
    StageManager stageManager;
    HUDSystem hudSystem;

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterName;

    // �J�����V�X�e��
    float cameraX;
    float previousPlayerX;
    static constexpr float CAMERA_FOLLOW_SPEED = 0.08f;

    // ��ԊǗ�
    bool exitRequested;
    bool tutorialCompleted;
    TutorialStep currentStep;
    float stepTimer;
    float messageTimer;
    bool messageVisible;

    // ���͊Ǘ�
    bool escPressed, escPressedPrev;
    bool leftPressed, leftPressedPrev;
    bool rightPressed, rightPressedPrev;
    bool spacePressed, spacePressedPrev;
    bool downPressed, downPressedPrev;
    bool rPressed, rPressedPrev;

    // �`���[�g���A���e�L�X�g
    std::vector<TutorialText> tutorialTexts;

    // �v���C���[�ʒu�`�F�b�N�p
    float startX;
    float goalX;
    bool hasMovedLeft;
    bool hasMovedRight;
    bool hasJumped;
    bool hasDucked;
    bool hasSlid;

    // �t�F�[�h����
    float fadeAlpha;
    bool fadingIn;
    bool fadingOut;

    // **�V�ǉ�: �G�t�F�N�g�V�X�e��**
    std::vector<Particle> particles;
    std::vector<FloatingText> floatingTexts;
    float successEffectTimer;
    bool showingSuccessEffect;

    // **�V�ǉ�: UI�����v�f**
    float uiPulseTimer;
    float iconRotation;
    float progressBarGlow;
    bool stepJustChanged;
    float stepChangeEffectTimer;

    // **�V�ǉ�: ���͎��o��**
    struct InputIndicator {
        bool isActive;
        float intensity;
        float timer;
    };
    InputIndicator leftIndicator, rightIndicator, spaceIndicator, downIndicator;

    // �w���p�[�֐�
    void InitializeTutorialTexts();
    void InitializeTutorialStage();
    void UpdateInput();
    void UpdateCamera();
    void UpdateTutorialLogic();
    void UpdateFade();
    void CheckStepCompletion();

    // **�V�ǉ�: �G�t�F�N�g�֐�**
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

    // **�V�ǉ�: �G�t�F�N�g�`��**
    void DrawEffects();
    void DrawParticles();
    void DrawFloatingTexts();
    void DrawInputIndicators();
    void DrawEnhancedUI();

    std::string GetCharacterDisplayName(int index);
    void NextStep();
    void CompleteTutorial();

    // **�V�ǉ�: UI�����w���p�[**
    void DrawGlowingBox(int x1, int y1, int x2, int y2, int color, float glowIntensity);
    void DrawPulsingText(const std::string& text, int x, int y, int color, int fontHandle, float pulseIntensity);
    int GetPulsingAlpha(float baseAlpha, float pulseSpeed);

    // ���[�e�B���e�B�֐�
    float Lerp(float a, float b, float t);
    bool IsPlayerInRange(float minX, float maxX);
};