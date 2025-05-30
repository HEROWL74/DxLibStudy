#pragma once
#include "DxLib.h"

// ボタン構造体
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
    // UI画像ハンドル
    int playButtonHandle;
    int optionsButtonHandle;
    int exitButtonHandle;
    int checkmarkHandle;
    int rectangleHandle;
    int sliderHandle;
    int homeHandle;

    // ボタン
    Button startButton;
    Button optionsButton;
    Button exitButton;
    Button backButton;

    // UI状態
    bool isInOptionsMenu;
    bool startButtonPressed;

    // アニメーション用
    int frameCounter;
    float titlePulse;

    // オプション設定
    float masterVolume;
    float sfxVolume;
    bool fullScreen;

    // マウス座標
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

    // ボタン関連
    void UpdateButton(Button& button);
    void DrawButton(const Button& button);
    bool IsButtonClicked(const Button& button);

    // ゲッター
    bool IsStartButtonPressed() const { return startButtonPressed; }
    float GetMasterVolume() const { return masterVolume; }
    float GetSFXVolume() const { return sfxVolume; }
    bool IsFullScreen() const { return fullScreen; }

    // プライベートヘルパー
private:
    void LoadUIImages();
    void SetupButtons();
    void DrawSlider(int x, int y, int width, float value, const char* label);
    void DrawCheckbox(int x, int y, bool checked, const char* label);
};