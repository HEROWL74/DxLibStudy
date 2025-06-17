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
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �v���O���X�o�[�ݒ�
    static const int PROGRESS_BAR_WIDTH = 800;
    static const int PROGRESS_BAR_HEIGHT = 30;
    static constexpr float LOADING_DURATION = 5.0f; // 5�b�Ԃ̃��[�f�B���O�i���x�𒲐��j

    // �G�t�F�N�g�ݒ�i�ق̂ڂ̒����j
    static constexpr float PARTICLE_SPAWN_RATE = 0.15f;  // ��菭�Ȃ��p�x
    static constexpr int MAX_PARTICLES = 50;             // �p�[�e�B�N���������炷
    static constexpr float TITLE_PULSE_SPEED = 0.02f;    // ������肵���p���X
    static constexpr float ICON_FLOAT_SPEED = 0.03f;     // ������肵�����V
    static constexpr float GLOW_PULSE_SPEED = 0.04f;

    // �p�[�e�B�N���\����
    struct Particle {
        float x, y;
        float velocityX, velocityY;
        float life, maxLife;
        float scale;
        int color;
        float rotation;
        float rotationSpeed;
        int type; // 0=�u���b�N��, 1=�X�p�[�N��, 2=�R�C��
    };

    // �A�j���[�V�����v�f�\����
    struct LoadingElement {
        float x, y;
        float targetX, targetY;
        float scale;
        float rotation;
        float alpha;
        float animationPhase;
        bool isVisible;
        int elementType; // �v�f�^�C�v�i�u���b�N�A�c�[�����j
    };

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int fontHandle, largeFontHandle, titleFontHandle;

    // �u���b�N�A�X���`�b�N�֘A�e�N�X�`��
    int dirtBlockHandle, stoneBlockHandle, crateBlockHandle;
    int coinHandle, explosiveHandle, playerIconHandle;
    int toolHammerHandle, toolPickaxeHandle;
    int progressBarBgHandle, progressBarFillHandle;

    // UI����
    int decorFrameHandle, glowEffectHandle;

    // ��ԊǗ�
    bool loadingComplete;
    float loadingProgress; // 0.0f ~ 1.0f
    float loadingTimer;
    int selectedCharacterIndex;
    std::string characterName;

    // �G�t�F�N�g�Ǘ�
    std::vector<Particle> particles;
    std::vector<LoadingElement> loadingElements;
    float particleSpawnTimer;
    float titlePulsePhase;
    float iconFloatPhase;
    float glowPulsePhase;

    // ���[�f�B���O�X�e�[�W�i�i�K�I���o�p�j
    enum LoadingStage {
        STAGE_INTRO,          // �����i�u���b�N�v�f���o��j
        STAGE_BUILDING,       // �\�z���i�A�X���`�b�N�R�[�X�g�ݗ��āj
        STAGE_FINALIZING,     // �d�グ�i�G�t�F�N�g�����j
        STAGE_COMPLETE        // ����
    };
    LoadingStage currentStage;
    float stageTimer;

    // �v���C�x�[�g���\�b�h
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

    // ���[�e�B���e�B
    std::string GetCharacterDisplayName(int index);
    std::string GetRandomLoadingTip();
    float Lerp(float a, float b, float t);
    float EaseOutCubic(float t);
    float EaseInOutQuad(float t);
    int GetRandomColor();

    // ���[�f�B���O���b�Z�[�W�Ǘ�
    std::vector<std::string> loadingMessages;
    std::vector<std::string> loadingTips;
    int currentMessageIndex;
    float messageTimer;
    static constexpr float MESSAGE_DURATION = 1.0f;
};