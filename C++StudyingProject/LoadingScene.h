#pragma once
#include "DxLib.h"
#include <string>
#include <vector>
#include <functional>

class LoadingScene {
public:
    // ローディングの種類
    enum LoadingType {
        LOADING_GAME_START,      // ゲーム開始時
        LOADING_STAGE_CHANGE,    // ステージ切り替え
        LOADING_CHARACTER_CHANGE, // キャラクター変更
        LOADING_RESOURCES        // リソース読み込み
    };

    // ローディングタスクの構造体
    struct LoadingTask {
        std::string description;           // 表示用説明
        std::function<bool()> taskFunc;   // 実行する処理
        bool completed;                   // 完了フラグ
        float weight;                     // 進行度の重み（0.1f～1.0f）

        LoadingTask(const std::string& desc, std::function<bool()> func, float w = 1.0f)
            : description(desc), taskFunc(func), completed(false), weight(w) {}
    };

    LoadingScene();
    ~LoadingScene();

    void Initialize();
    void StartLoading(LoadingType type, int selectedCharacter = -1, int targetStage = -1);
    void Update();
    void Draw();

    // ローディング状態の取得
    bool IsLoadingComplete() const { return loadingComplete; }
    bool IsLoadingActive() const { return loadingActive; }
    float GetProgress() const { return loadingProgress; }

    // カスタムタスクの追加
    void AddCustomTask(const std::string& description, std::function<bool()> task, float weight = 1.0f);

private:
    // UI要素
    int backgroundTexture;
    int loadingIconTexture;
    int fontHandle;
    int titleFontHandle;

    // ローディング状態
    bool loadingActive;
    bool loadingComplete;
    float loadingProgress;      // 0.0f ～ 1.0f
    float displayProgress;      // 表示用（スムーズアニメーション）

    // ローディングタスク
    std::vector<LoadingTask> loadingTasks;
    int currentTaskIndex;
    float currentTaskProgress;

    // アニメーション
    float animationTimer;
    float iconRotation;
    float pulsePhase;

    // 設定
    LoadingType currentLoadingType;
    int characterIndex;
    int stageIndex;

    // フェード効果
    float fadeAlpha;
    bool fadeIn;
    bool fadeOut;

    // 表示テキスト
    std::string currentTaskText;
    std::string loadingTitle;

    // 定数
    static constexpr float PROGRESS_SMOOTH_SPEED = 0.05f;
    static constexpr float ICON_ROTATION_SPEED = 0.08f;
    static constexpr float PULSE_SPEED = 0.04f;
    static constexpr float MIN_LOADING_TIME = 1.0f;  // 最小表示時間
    static constexpr float FADE_SPEED = 0.03f;

    // 内部関数
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

    // ユーティリティ
    float EaseInOut(float t);
    void ResetState();

    // 実際のローディング処理関数
    bool LoadGameResources();
    bool LoadCharacterSprites(int characterIdx);
    bool LoadStageData(int stageIdx);
    bool LoadSoundResources();
    bool LoadUIResources();
    bool InitializeGameSystems();
    bool WaitMinimumTime();
};