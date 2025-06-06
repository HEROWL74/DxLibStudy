#pragma once
#include "DxLib.h"
#include <string>
#include <vector>
#include <functional>

class LoadingScene {
public:
    // ���[�f�B���O�̎��
    enum LoadingType {
        LOADING_GAME_START,      // �Q�[���J�n��
        LOADING_STAGE_CHANGE,    // �X�e�[�W�؂�ւ�
        LOADING_CHARACTER_CHANGE, // �L�����N�^�[�ύX
        LOADING_RESOURCES        // ���\�[�X�ǂݍ���
    };

    // ���[�f�B���O�^�X�N�̍\����
    struct LoadingTask {
        std::string description;           // �\���p����
        std::function<bool()> taskFunc;   // ���s���鏈��
        bool completed;                   // �����t���O
        float weight;                     // �i�s�x�̏d�݁i0.1f�`1.0f�j

        LoadingTask(const std::string& desc, std::function<bool()> func, float w = 1.0f)
            : description(desc), taskFunc(func), completed(false), weight(w) {}
    };

    LoadingScene();
    ~LoadingScene();

    void Initialize();
    void StartLoading(LoadingType type, int selectedCharacter = -1, int targetStage = -1);
    void Update();
    void Draw();

    // ���[�f�B���O��Ԃ̎擾
    bool IsLoadingComplete() const { return loadingComplete; }
    bool IsLoadingActive() const { return loadingActive; }
    float GetProgress() const { return loadingProgress; }

    // �J�X�^���^�X�N�̒ǉ�
    void AddCustomTask(const std::string& description, std::function<bool()> task, float weight = 1.0f);

private:
    // UI�v�f
    int backgroundTexture;
    int loadingIconTexture;
    int fontHandle;
    int titleFontHandle;

    // ���[�f�B���O���
    bool loadingActive;
    bool loadingComplete;
    float loadingProgress;      // 0.0f �` 1.0f
    float displayProgress;      // �\���p�i�X���[�Y�A�j���[�V�����j

    // ���[�f�B���O�^�X�N
    std::vector<LoadingTask> loadingTasks;
    int currentTaskIndex;
    float currentTaskProgress;

    // �A�j���[�V����
    float animationTimer;
    float iconRotation;
    float pulsePhase;

    // �ݒ�
    LoadingType currentLoadingType;
    int characterIndex;
    int stageIndex;

    // �t�F�[�h����
    float fadeAlpha;
    bool fadeIn;
    bool fadeOut;

    // �\���e�L�X�g
    std::string currentTaskText;
    std::string loadingTitle;

    // �萔
    static constexpr float PROGRESS_SMOOTH_SPEED = 0.05f;
    static constexpr float ICON_ROTATION_SPEED = 0.08f;
    static constexpr float PULSE_SPEED = 0.04f;
    static constexpr float MIN_LOADING_TIME = 1.0f;  // �ŏ��\������
    static constexpr float FADE_SPEED = 0.03f;

    // �����֐�
    void LoadTextures();
    void SetupTasks();
    void SetupGameStartTasks();
    void SetupStageChangeTasks();
    void SetupCharacterChangeTasks();
    void SetupResourceTasks();

    void UpdateProgress();
    void UpdateAnimations();
    void UpdateFade();

    void DrawBackground();
    void DrawProgressBar();
    void DrawLoadingIcon();
    void DrawLoadingText();
    void DrawFade();

    // ���[�e�B���e�B
    float EaseInOut(float t);
    void ResetState();

    // ���ۂ̃��[�f�B���O�����֐�
    bool LoadGameResources();
    bool LoadCharacterSprites(int characterIdx);
    bool LoadStageData(int stageIdx);
    bool LoadSoundResources();
    bool LoadUIResources();
    bool InitializeGameSystems();
    bool WaitMinimumTime();
};