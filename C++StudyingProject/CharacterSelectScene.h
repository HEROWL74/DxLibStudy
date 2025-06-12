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
    bool IsTutorialRequested() const { return tutorialRequested; }  // **新追加**
    int GetSelectedCharacter() const { return selectedCharacterIndex; }
    void ResetState(); // 状態リセット用関数を追加

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;
    static const int CHARACTER_COUNT = 5;

    // エフェクト定数
    static constexpr float HOVER_SCALE = 1.15f;
    static constexpr float NORMAL_SCALE = 1.0f;
    static constexpr float SCALE_LERP = 0.2f;
    static constexpr float POPUP_SPEED = 0.25f;
    static constexpr float GLOW_SPEED = 0.15f;
    static constexpr float FLOAT_SPEED = 0.06f;
    static constexpr float FLOAT_AMP = 8.0f;
    static constexpr float ROTATION_SPEED = 0.008f;  // 回転速度
    static constexpr float MOVE_SPEED = 0.12f;       // 選択後の移動速度

    enum SelectionState { SELECTING, SELECTED, CONFIRMED };

    // キャラクター情報構造体
    struct Character {
        std::string name;
        std::string iconPath;
        int iconHandle;
        int baseX, baseY;       // 基本配置位置（円形配置）
        int currentX, currentY; // 現在の描画位置
        int targetX, targetY;   // 目標位置
        float scale;            // スケール
        float hoverProgress;    // ホバー進行度
        float popupProgress;    // ポップアップ進行度
        float glowIntensity;    // グロー強度
        float floatPhase;       // 浮遊効果のフェーズ
        bool hovered;          // ホバー状態
        bool selected;         // 選択状態
    };

    // UIボタン構造体
    struct UIButton {
        int x, y, w, h;
        std::string label;
        float scale;
        float glowIntensity;
        bool hovered;
        bool enabled;
    };

    // テクスチャハンドル
    int backgroundHandle;
    int buttonBackgroundHandle;  // button_square_gradient.png
    int uiButtonHandle;          // UIボタン用
    int fontHandle;
    int largeFontHandle;

    // キャラクター配列
    std::vector<Character> characters;

    // UIボタン
    std::vector<UIButton> uiButtons;

    // 状態管理
    bool characterSelected;
    bool backRequested;
    bool tutorialRequested;      // **新追加：チュートリアルリクエスト**
    bool selectConfirmed;       // SELECT ボタンが押された
    int selectedCharacterIndex;
    int hoveredCharacterIndex;
    SelectionState selectionState;
    float rotationAngle;        // 五角形回転角度
    float titlePulsePhase;      // タイトル脈動効果

    // マウス入力
    int mouseX, mouseY;
    bool mousePressed, mousePressedPrev;

    // キーボード入力
    bool backspacePressed, backspacePressedPrev;

    // ヘルパー関数
    void UpdateCharacters();
    void UpdateUIButtons();
    void UpdateCharacterPositions();
    void DrawCharacter(const Character& character, int index);
    void DrawUIButton(const UIButton& button);
    void DrawGlowEffect(int x, int y, int w, int h, float intensity, int color);

    bool IsMouseOver(int x, int y, int w, int h) const;
    bool IsClicked(int x, int y, int w, int h) const;

    // イージング関数
    float EaseOutBack(float t);
    float EaseInOutCubic(float t);
    float Lerp(float a, float b, float t);
};