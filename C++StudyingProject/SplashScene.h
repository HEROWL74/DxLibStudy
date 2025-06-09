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
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �t�F�[�Y�Ǘ�
    enum SplashPhase {
        PHASE_LOGO_FADEIN,      // ���S�t�F�[�h�C��
        PHASE_LOGO_DISPLAY,     // ���S�\��
        PHASE_EXPLOSION_PREP,   // ��������
        PHASE_EXPLOSION,        // �����G�t�F�N�g
        PHASE_SLIDE_UP,         // �X���C�h�A�b�v�g�����W�V����
        PHASE_COMPLETE          // ����
    };

    SplashPhase currentPhase;
    float phaseTimer;

    // ���\�[�X
    int logoHandle;           // �X�^�W�I���S�i�K���ȉ摜���g�p�j
    int bombHandle;           // bomb.png
    int bombActiveHandle;     // bomb_active.png
    int fontHandle;           // �t�H���g

    // ���S�\���p
    float logoAlpha;
    float logoScale;

    // �����G�t�F�N�g�p
    struct ExplosionParticle {
        float x, y;           // �ʒu
        float vx, vy;         // ���x
        float life;           // ����
        float maxLife;        // �ő����
        float size;           // �T�C�Y
        int color;            // �F
        bool active;          // �A�N�e�B�u���
    };

    std::vector<ExplosionParticle> explosionParticles;
    float explosionTimer;
    bool explosionStarted;
    int bombAnimFrame;        // �{���摜�̃A�j���[�V�����p

    // �X���C�h�A�b�v�g�����W�V�����p
    float slideOffset;        // �X���C�h�̃I�t�Z�b�g
    bool transitionComplete;

    // �^�C�~���O�萔
    static constexpr float LOGO_FADEIN_DURATION = 1.0f;
    static constexpr float LOGO_DISPLAY_DURATION = 2.0f;
    static constexpr float EXPLOSION_PREP_DURATION = 0.5f;
    static constexpr float EXPLOSION_DURATION = 1.5f;
    static constexpr float SLIDE_UP_DURATION = 1.0f;

    // �����֐�
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