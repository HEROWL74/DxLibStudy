#pragma once
#include "DxLib.h"

// �{�^���\����
struct Button
{
    int x, y, width, height;
    int normalHandle, hoverHandle;
    bool isHovered;
    bool isPressed;
    float scale;
    float targetScale;

    Button() : x(0), y(0), width(0), height(0), normalHandle(-1), hoverHandle(-1),
        isHovered(false), isPressed(false), scale(1.0f), targetScale(1.0f) {}
};

class UIManager
{
private:
    // UI�摜�n���h��
    int playButtonHandle;
    int optionsButtonHandle;
    int exitButtonHandle;
    int checkmarkHandle;
    int rectangleHandle;
    int sliderHandle;
    int homeHandle;

    // �{�^��
    Button startButton;
    Button optionsButton;
    Button exitButton;
    Button backButton;

    // UI���
    bool isInOptionsMenu;
    bool startButtonPressed;

    // �A�j���[�V�����p
    int frameCounter;
    float titlePulse;

    // �I�v�V�����ݒ�
    float masterVolume;
    float sfxVolume;
    bool fullScreen;

    // �}�E�X���W
    int mouseX, mouseY;
    bool mousePressed;
    bool mousePressedPrev;

public:
    UIManager();
    ~UIManager();

    void Initialize();
    void UpdateStartScreen();
    void Draw();
    void DrawStartScreen();
    void DrawOptionsMenu();
    void Finalize();

    // �{�^���֘A
    void UpdateButton(Button& button);
    void DrawButton(const Button& button);
    bool IsButtonClicked(const Button& button);

    // �Q�b�^�[
    bool IsStartButtonPressed() const { return startButtonPressed; }
    float GetMasterVolume() const { return masterVolume; }
    float GetSFXVolume() const { return sfxVolume; }
    bool IsFullScreen() const { return fullScreen; }

    // �v���C�x�[�g�w���p�[
private:
    void LoadUIImages();
    void SetupButtons();
    void DrawSlider(int x, int y, int width, float value, const char* label);
    void DrawCheckbox(int x, int y, bool checked, const char* label);
};