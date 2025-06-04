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

    // �\������
    void ShowResult(int starsCollected, int totalStars, int currentStage);
    void HideResult();
    void ResetState();  // **��Ԋ��S���Z�b�g�@�\��ǉ�**
    bool IsVisible() const { return resultState == RESULT_VISIBLE; }
    bool IsShowing() const { return resultState == RESULT_SHOWING || resultState == RESULT_VISIBLE; }
    bool IsHidden() const { return resultState == RESULT_HIDDEN; }  // **���S��\���m�F�p**

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
    float buttonsAlpha;          // **�{�^���̃t�F�[�h�����x�i�V�@�\�j**
    bool buttonsVisible;         // **�{�^���\���t���O�i�V�@�\�j**
    int displayedStars;
    int totalDisplayStars;
    int currentStageNumber;

    // �{�^��
    UIButton retryButton;
    UIButton nextStageButton;

    // �}�E�X����
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // �A�j���[�V�����萔�i�������ɒ����j
    static constexpr float ANIM_SPEED = 0.02f;          // 0.04f �� 0.02f�i�����̑��x�j
    static constexpr float STAR_ANIM_SPEED = 0.15f;     // 0.3f �� 0.15f�i�����̑��x�j
    static constexpr float BUTTON_FADE_SPEED = 0.03f;   // 0.06f �� 0.03f�i�����̑��x�j
    static constexpr float BUTTON_SCALE_NORMAL = 1.0f;
    static constexpr float BUTTON_SCALE_HOVER = 1.1f;

    // �w���p�[�֐�
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

    // **�V���v���ɂȂ������A�j���[�V�����֐�**
    void DrawCollectedStarAnimation(int x, int y, int baseSize, float progress, int starIndex);
    void DrawUnCollectedStar(int x, int y, int baseSize, float progress);

    bool IsMouseOver(int x, int y, int w, int h) const;
    float EaseOutBack(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};