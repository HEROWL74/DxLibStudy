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
    // ��� & �{�^���T�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;
    static const int BUTTON_W = 300;
    static const int BUTTON_H = 100;

    // �G�t�F�N�g�萔
    static constexpr float HOVER_SCALE = 1.08f;
    static constexpr float NORMAL_SCALE = 1.0f;
    static constexpr float SCALE_LERP = 0.18f;
    static constexpr float FLOAT_SPEED = 0.08f;
    static constexpr float FLOAT_AMP = 6.0f;
    static constexpr float ANIM_SPEED = 0.08f;  // ��芊�炩�ȃA�j���[�V����
    static constexpr float FADE_SPEED = 0.06f;  // �t�F�[�h��p���x
    static constexpr float TRANSITION_SPEED = 0.04f;  // ��ʑJ�ڃt�F�[�h���x

    // �C�[�W���O�֐��p
    enum EaseType { EASE_IN_OUT_CUBIC, EASE_OUT_ELASTIC, EASE_IN_OUT_QUART };

    enum OptionState { Hidden, Showing, Visible, Hiding };
    enum TransitionState { None, FadingOut, FadingIn, Complete };

    // �p�[�e�B�N���\����
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
        float glowIntensity;    // �O���[���ʗp
        float pressProgress;    // �v���X�G�t�F�N�g�p
    };

    // �X���C�_�[���Ǘp�\����
    struct EnhancedSlider {
        int x, y, w, h;
        float value;
        bool dragging;
        float hoverProgress;    // �z�o�[���̊g�����
        float valueDisplay;     // �\���p�̕�Ԃ��ꂽ�l
        float glowEffect;       // �O���[����
    };

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int buttonHandle;
    int slideHandle;
    int slideBarHandle;     // �X���C�_�[�o�[�p�摜
    int homeHandle;
    int fontHandle;
    int largeFontHandle;    // �^�C�g���p�傫�ȃt�H���g

    // START/OPTIONS/EXIT �{�^��
    std::vector<UIButton> buttons;

    // ���ǂ��ꂽ�X���C�_�[
    EnhancedSlider slider;

    // ��ԊǗ�
    bool        startRequested;
    bool        exitRequested;
    OptionState optionState;
    TransitionState transitionState;     // ��ʑJ�ڏ��
    float       optionAnimProgress;
    float       optionFadeProgress;      // �I�v�V������ʂ̃t�F�[�h�p
    float       optionSlideProgress;     // �I�v�V������ʂ̃X���C�h�p
    float       backgroundFadeProgress;  // �w�i�t�F�[�h�p
    float       transitionFadeProgress;  // ��ʑJ�ڃt�F�[�h�p
    float       titlePulsePhase;         // �^�C�g���̖�������

    // �}�E�X
    int  mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // �L�[�{�[�h����
    bool backspacePressed, backspacePressedPrev;

    // �p�[�e�B�N���V�X�e��
    std::vector<Particle> particles;
    std::mt19937 randomEngine;
    float particleSpawnTimer;

    // �w���p�[�֐�
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

    // �C�[�W���O�֐�
    float EaseInOutCubic(float t);
    float EaseOutElastic(float t);
    float EaseInOutQuart(float t);
    float ApplyEasing(float t, EaseType type);

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
    float SmoothStep(float edge0, float edge1, float x);
    int BlendColor(int color1, int color2, float ratio);
};