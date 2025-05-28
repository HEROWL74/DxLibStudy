#pragma once
#include <vector>
#include <string>

// メニュー項目の列挙型
enum class MenuOption {
    START_GAME,
    OPTIONS,
    EXIT,
    COUNT
};

// オプション項目の列挙型
enum class OptionItem {
    VOLUME_MASTER,
    VOLUME_BGM,
    VOLUME_SE,
    FULLSCREEN,
    BACK,
    COUNT
};

// シーンの状態
enum class TitleState {
    FADE_IN,        // フェードイン中
    MAIN_MENU,      // メインメニュー表示中
    OPTIONS_MENU,   // オプションメニュー表示中
    FADE_OUT,       // フェードアウト中
    FINISHED        // 遷移完了
};

class TitleScene {
public:
    TitleScene();
    ~TitleScene();

    void Update();
    void Draw();

    // 状態確認
    bool IsStartRequested() const;
    bool IsExitRequested() const;
    bool IsFadeOutFinished() const;

private:
    // 基本状態
    TitleState m_state;
    bool m_startRequested;
    bool m_exitRequested;

    // フェード関連
    int m_fadeAlpha;
    int m_fadeSpeed;

    // キー入力制御
    bool m_prevUpKey;
    bool m_prevDownKey;
    bool m_prevLeftKey;
    bool m_prevRightKey;
    bool m_prevEnterKey;
    bool m_prevSpaceKey;
    bool m_prevEscapeKey;

    // メニュー関連
    MenuOption m_selectedMainMenu;
    OptionItem m_selectedOption;
    int m_menuAnimationTimer;

    // UI要素の位置とサイズ
    struct UIElement {
        int x, y;
        int width, height;
        bool isVisible;
        int alpha;
    };

    UIElement m_titleLogo;
    UIElement m_startButton;
    UIElement m_optionsButton;
    UIElement m_exitButton;
    UIElement m_optionsPanel;

    // オプション設定値
    int m_masterVolume;
    int m_bgmVolume;
    int m_seVolume;
    bool m_isFullscreen;

    // エフェクト関連
    int m_sparkleTimer;
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        int color;
    };
    std::vector<Particle> m_particles;

    // 内部メソッド
    void UpdateFadeIn();
    void UpdateMainMenu();
    void UpdateOptionsMenu();
    void UpdateFadeOut();

    void HandleMainMenuInput();
    void HandleOptionsInput();

    void DrawMainMenu();
    void DrawOptionsMenu();
    void DrawParticles();
    void DrawFade();

    void CreateSparkleEffect();
    void UpdateParticles();

    void InitializeUI();
    void PlayMenuSound();
    void PlaySelectSound();
};