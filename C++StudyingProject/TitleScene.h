#pragma once
#include <vector>
#include <string>

// ���j���[���ڂ̗񋓌^
enum class MenuOption {
    START_GAME,
    OPTIONS,
    EXIT,
    COUNT
};

// �I�v�V�������ڂ̗񋓌^
enum class OptionItem {
    VOLUME_MASTER,
    VOLUME_BGM,
    VOLUME_SE,
    FULLSCREEN,
    BACK,
    COUNT
};

// �V�[���̏��
enum class TitleState {
    FADE_IN,        // �t�F�[�h�C����
    MAIN_MENU,      // ���C�����j���[�\����
    OPTIONS_MENU,   // �I�v�V�������j���[�\����
    FADE_OUT,       // �t�F�[�h�A�E�g��
    FINISHED        // �J�ڊ���
};

class TitleScene {
public:
    TitleScene();
    ~TitleScene();

    void Update();
    void Draw();

    // ��Ԋm�F
    bool IsStartRequested() const;
    bool IsExitRequested() const;
    bool IsFadeOutFinished() const;

private:
    // ��{���
    TitleState m_state;
    bool m_startRequested;
    bool m_exitRequested;

    // �t�F�[�h�֘A
    int m_fadeAlpha;
    int m_fadeSpeed;

    // �L�[���͐���
    bool m_prevUpKey;
    bool m_prevDownKey;
    bool m_prevLeftKey;
    bool m_prevRightKey;
    bool m_prevEnterKey;
    bool m_prevSpaceKey;
    bool m_prevEscapeKey;

    // ���j���[�֘A
    MenuOption m_selectedMainMenu;
    OptionItem m_selectedOption;
    int m_menuAnimationTimer;

    // UI�v�f�̈ʒu�ƃT�C�Y
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

    // �I�v�V�����ݒ�l
    int m_masterVolume;
    int m_bgmVolume;
    int m_seVolume;
    bool m_isFullscreen;

    // �G�t�F�N�g�֘A
    int m_sparkleTimer;
    struct Particle {
        float x, y;
        float vx, vy;
        int life;
        int color;
    };
    std::vector<Particle> m_particles;

    // �������\�b�h
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