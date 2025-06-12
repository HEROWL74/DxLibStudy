#pragma once
#include "DxLib.h"
#include <vector>
#include <string>
#include <math.h>
#include <algorithm>

class CharacterSelectScene
{
public:
    CharacterSelectScene();
    ~CharacterSelectScene();

    void Initialize();
    void Update();
    void Draw();

    bool IsCharacterSelected() const { return selectConfirmed; }
    bool IsBackRequested() const { return backRequested; }
    bool IsTutorialRequested() const { return tutorialRequested; }  // **�V�ǉ�**
    int GetSelectedCharacter() const { return selectedCharacterIndex; }
    void ResetState(); // ��ԃ��Z�b�g�p�֐���ǉ�

private:
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;
    static const int CHARACTER_COUNT = 5;

    // �G�t�F�N�g�萔
    static constexpr float HOVER_SCALE = 1.15f;
    static constexpr float NORMAL_SCALE = 1.0f;
    static constexpr float SCALE_LERP = 0.2f;
    static constexpr float POPUP_SPEED = 0.25f;
    static constexpr float GLOW_SPEED = 0.15f;
    static constexpr float FLOAT_SPEED = 0.06f;
    static constexpr float FLOAT_AMP = 8.0f;
    static constexpr float ROTATION_SPEED = 0.008f;  // ��]���x
    static constexpr float MOVE_SPEED = 0.12f;       // �I����̈ړ����x

    enum SelectionState { SELECTING, SELECTED, CONFIRMED };

    // �L�����N�^�[���\����
    struct Character {
        std::string name;
        std::string iconPath;
        int iconHandle;
        int baseX, baseY;       // ��{�z�u�ʒu�i�~�`�z�u�j
        int currentX, currentY; // ���݂̕`��ʒu
        int targetX, targetY;   // �ڕW�ʒu
        float scale;            // �X�P�[��
        float hoverProgress;    // �z�o�[�i�s�x
        float popupProgress;    // �|�b�v�A�b�v�i�s�x
        float glowIntensity;    // �O���[���x
        float floatPhase;       // ���V���ʂ̃t�F�[�Y
        bool hovered;          // �z�o�[���
        bool selected;         // �I�����
    };

    // UI�{�^���\����
    struct UIButton {
        int x, y, w, h;
        std::string label;
        float scale;
        float glowIntensity;
        bool hovered;
        bool enabled;
    };

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int buttonBackgroundHandle;  // button_square_gradient.png
    int uiButtonHandle;          // UI�{�^���p
    int fontHandle;
    int largeFontHandle;

    // �L�����N�^�[�z��
    std::vector<Character> characters;

    // UI�{�^��
    std::vector<UIButton> uiButtons;

    // ��ԊǗ�
    bool characterSelected;
    bool backRequested;
    bool tutorialRequested;      // **�V�ǉ��F�`���[�g���A�����N�G�X�g**
    bool selectConfirmed;       // SELECT �{�^���������ꂽ
    int selectedCharacterIndex;
    int hoveredCharacterIndex;
    SelectionState selectionState;
    float rotationAngle;        // �܊p�`��]�p�x
    float titlePulsePhase;      // �^�C�g����������

    // �}�E�X����
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // �L�[�{�[�h����
    bool backspacePressed, backspacePressedPrev;

    // �w���p�[�֐�
    void UpdateCharacters();
    void UpdateUIButtons();
    void UpdateCharacterPositions();
    void DrawCharacter(const Character& character, int index);
    void DrawUIButton(const UIButton& button);
    void DrawGlowEffect(int x, int y, int w, int h, float intensity, int color);

    bool IsMouseOver(int x, int y, int w, int h) const;
    bool IsClicked(int x, int y, int w, int h) const;

    // �C�[�W���O�֐�
    float EaseOutBack(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};