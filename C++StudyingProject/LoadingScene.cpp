#include "LoadingScene.h"
#include "SoundManager.h"
#include <algorithm>
#include <math.h>

LoadingScene::LoadingScene()
    : backgroundTexture(-1)
    , loadingIconTexture(-1)
    , fontHandle(-1)
    , titleFontHandle(-1)
    , loadingActive(false)
    , loadingComplete(false)
    , loadingProgress(0.0f)
    , displayProgress(0.0f)
    , currentTaskIndex(0)
    , currentTaskProgress(0.0f)
    , animationTimer(0.0f)
    , iconRotation(0.0f)
    , pulsePhase(0.0f)
    , currentLoadingType(LOADING_GAME_START)
    , characterIndex(-1)
    , stageIndex(-1)
    , fadeAlpha(0.0f)
    , fadeIn(true)
    , fadeOut(false)
    , currentTaskText("")
    , loadingTitle("Loading...")
{
}

LoadingScene::~LoadingScene()
{
    if (backgroundTexture != -1) DeleteGraph(backgroundTexture);
    if (loadingIconTexture != -1) DeleteGraph(loadingIconTexture);
    if (fontHandle != -1) DeleteFontToHandle(fontHandle);
    if (titleFontHandle != -1) DeleteFontToHandle(titleFontHandle);
}

void LoadingScene::Initialize()
{
    LoadTextures();
    ResetState();
}

void LoadingScene::LoadTextures()
{
    // ���[�f�B���O��ʗp�̔w�i�i�V���v���ȃO���f�[�V�����z��j
    backgroundTexture = LoadGraph("Sprites/Backgrounds/loading_background.png");
    if (backgroundTexture == -1) {
        // �t�@�C����������Ȃ��ꍇ�́A�v���[�X�z���_�[�Ƃ��Ė����l�̂܂�
        OutputDebugStringA("LoadingScene: Background texture not found, using solid color\n");
    }

    // ���[�f�B���O�A�C�R��
    loadingIconTexture = LoadGraph("Sprites/Tiles/Saw.png"); // �܂��� spinner.png
    if (loadingIconTexture == -1) {
        OutputDebugStringA("LoadingScene: Loading icon not found, using default graphics\n");
    }

    // �t�H���g�쐬
    fontHandle = CreateFontToHandle(NULL, 24, 3);
    titleFontHandle = CreateFontToHandle(NULL, 36, 4);
}

void LoadingScene::StartLoading(LoadingType type, int selectedCharacter, int targetStage)
{
    currentLoadingType = type;
    characterIndex = selectedCharacter;
    stageIndex = targetStage;

    ResetState();
    SetupTasks();

    loadingActive = true;
    loadingComplete = false;

    // ���[�f�B���O�^�C�v�ɉ������^�C�g���ݒ�
    switch (type) {
    case LOADING_GAME_START:
        loadingTitle = "Initializing Game...";
        break;
    case LOADING_STAGE_CHANGE:
        loadingTitle = "Loading Stage " + std::to_string(targetStage + 1) + "...";
        break;
    case LOADING_CHARACTER_CHANGE:
        loadingTitle = "Loading Character...";
        break;
    case LOADING_RESOURCES:
        loadingTitle = "Loading Resources...";
        break;
    }

    OutputDebugStringA("LoadingScene: Started loading\n");
}

void LoadingScene::SetupTasks()
{
    loadingTasks.clear();

    switch (currentLoadingType) {
    case LOADING_GAME_START:
        SetupGameStartTasks();
        break;
    case LOADING_STAGE_CHANGE:
        SetupStageChangeTasks();
        break;
    case LOADING_CHARACTER_CHANGE:
        SetupCharacterChangeTasks();
        break;
    case LOADING_RESOURCES:
        SetupResourceTasks();
        break;
    }
}

void LoadingScene::SetupGameStartTasks()
{
    loadingTasks.emplace_back("Loading game resources...",
        [this]() { return LoadGameResources(); }, 0.3f);

    loadingTasks.emplace_back("Loading character data...",
        [this]() { return LoadCharacterSprites(characterIndex); }, 0.2f);

    loadingTasks.emplace_back("Loading sound system...",
        [this]() { return LoadSoundResources(); }, 0.2f);

    loadingTasks.emplace_back("Loading UI elements...",
        [this]() { return LoadUIResources(); }, 0.15f);

    loadingTasks.emplace_back("Initializing game systems...",
        [this]() { return InitializeGameSystems(); }, 0.1f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.05f);
}

void LoadingScene::SetupStageChangeTasks()
{
    loadingTasks.emplace_back("Loading stage data...",
        [this]() { return LoadStageData(stageIndex); }, 0.4f);

    loadingTasks.emplace_back("Loading stage graphics...",
        [this]() { return LoadGameResources(); }, 0.3f);

    loadingTasks.emplace_back("Preparing enemies...",
        [this]() { return InitializeGameSystems(); }, 0.2f);

    loadingTasks.emplace_back("Finalizing stage...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::SetupCharacterChangeTasks()
{
    loadingTasks.emplace_back("Loading character sprites...",
        [this]() { return LoadCharacterSprites(characterIndex); }, 0.6f);

    loadingTasks.emplace_back("Updating character data...",
        [this]() { return InitializeGameSystems(); }, 0.3f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::SetupResourceTasks()
{
    loadingTasks.emplace_back("Loading textures...",
        [this]() { return LoadGameResources(); }, 0.4f);

    loadingTasks.emplace_back("Loading sounds...",
        [this]() { return LoadSoundResources(); }, 0.3f);

    loadingTasks.emplace_back("Loading UI...",
        [this]() { return LoadUIResources(); }, 0.2f);

    loadingTasks.emplace_back("Finalizing...",
        [this]() { return WaitMinimumTime(); }, 0.1f);
}

void LoadingScene::Update()
{
    if (!loadingActive || loadingComplete) return;

    UpdateProgress();
    UpdateAnimations();
    UpdateFade();
}

void LoadingScene::UpdateProgress()
{
    if (currentTaskIndex >= loadingTasks.size()) {
        loadingComplete = true;
        fadeOut = true;
        return;
    }

    // ���݂̃^�X�N�����s
    LoadingTask& currentTask = loadingTasks[currentTaskIndex];
    if (!currentTask.completed) {
        currentTaskText = currentTask.description;

        // �^�X�N�����s�i�����t���[���ɂ킽���Ď��s�����\��������j
        bool taskCompleted = currentTask.taskFunc();

        if (taskCompleted) {
            currentTask.completed = true;
            currentTaskIndex++;

            // �f�o�b�O�o��
            OutputDebugStringA(("LoadingScene: Completed - " + currentTask.description + "\n").c_str());
        }
    }

    // �S�̂̐i�s�x���v�Z
    float totalWeight = 0.0f;
    float completedWeight = 0.0f;

    for (size_t i = 0; i < loadingTasks.size(); i++) {
        totalWeight += loadingTasks[i].weight;
        if (loadingTasks[i].completed) {
            completedWeight += loadingTasks[i].weight;
        }
    }

    loadingProgress = (totalWeight > 0.0f) ? (completedWeight / totalWeight) : 0.0f;

    // �\���p�i�s�x���X���[�Y�ɍX�V
    displayProgress += (loadingProgress - displayProgress) * PROGRESS_SMOOTH_SPEED;
}

void LoadingScene::UpdateAnimations()
{
    animationTimer += 0.016f; // 60FPS�z��

    // �A�C�R���̉�]
    iconRotation += ICON_ROTATION_SPEED;
    if (iconRotation >= 2.0f * DX_PI) {
        iconRotation -= 2.0f * DX_PI;
    }

    // �p���X����
    pulsePhase += PULSE_SPEED;
    if (pulsePhase >= 2.0f * DX_PI) {
        pulsePhase -= 2.0f * DX_PI;
    }
}

void LoadingScene::UpdateFade()
{
    if (fadeIn) {
        fadeAlpha += FADE_SPEED;
        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            fadeIn = false;
        }
    }
    else if (fadeOut) {
        fadeAlpha -= FADE_SPEED;
        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fadeOut = false;
            loadingActive = false;
        }
    }
}

void LoadingScene::Draw()
{
    if (!loadingActive) return;

    DrawBackground();
    DrawProgressBar();
    DrawLoadingIcon();
    DrawLoadingText();  // ���O�ύX: DrawText �� DrawLoadingText
    DrawFade();
}

void LoadingScene::DrawBackground()
{
    if (backgroundTexture != -1) {
        // �w�i�摜����ʑS�̂ɕ`��
        DrawExtendGraph(0, 0, 1920, 1080, backgroundTexture, TRUE);
    }
    else {
        // �w�i�摜���Ȃ��ꍇ�́A�O���f�[�V�������̔w�i��`��
        for (int y = 0; y < 1080; y++) {
            int alpha = 255 - (y * 100 / 1080);
            int color = GetColor(20 + alpha / 10, 30 + alpha / 8, 50 + alpha / 5);
            DrawLine(0, y, 1920, y, color);
        }
    }
}

void LoadingScene::DrawProgressBar()
{
    const int BAR_WIDTH = 600;
    const int BAR_HEIGHT = 20;
    const int BAR_X = (1920 - BAR_WIDTH) / 2;  // ���S�ɒ����z�u
    const int BAR_Y = 1080 / 2 + 80;  // ������Ɉړ�

    // �v���O���X�o�[�̔w�i
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
    DrawBox(BAR_X - 2, BAR_Y - 2, BAR_X + BAR_WIDTH + 2, BAR_Y + BAR_HEIGHT + 2,
        GetColor(255, 255, 255), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawBox(BAR_X, BAR_Y, BAR_X + BAR_WIDTH, BAR_Y + BAR_HEIGHT,
        GetColor(40, 40, 60), TRUE);

    // �v���O���X�o�[�̐i�s����
    int progressWidth = (int)(BAR_WIDTH * displayProgress);
    if (progressWidth > 0) {
        // �O���f�[�V��������
        for (int x = 0; x < progressWidth; x++) {
            float ratio = (float)x / progressWidth;
            int r = (int)(100 + 100 * ratio);
            int g = (int)(150 + 50 * ratio);
            int b = (int)(255 - 50 * ratio);

            DrawLine(BAR_X + x, BAR_Y, BAR_X + x, BAR_Y + BAR_HEIGHT,
                GetColor(r, g, b));
        }

        // �������
        float glowIntensity = sinf(animationTimer * 3.0f) * 0.3f + 0.7f;
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(80 * glowIntensity));
        DrawBox(BAR_X, BAR_Y, BAR_X + progressWidth, BAR_Y + BAR_HEIGHT,
            GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �i�s�x�e�L�X�g�i���S�����z�u�j
    std::string progressText = std::to_string((int)(displayProgress * 100)) + "%";
    int textWidth = GetDrawStringWidthToHandle(progressText.c_str(),
        progressText.length(), fontHandle);
    DrawStringToHandle(1920 / 2 - textWidth / 2, BAR_Y + BAR_HEIGHT + 15,
        progressText.c_str(), GetColor(255, 255, 255), fontHandle);
}

void LoadingScene::DrawLoadingIcon()
{
    const int ICON_SIZE = 80;
    const int ICON_X = 1920 / 2;  // ���S�ɒ���
    const int ICON_Y = 1080 / 2 - 80;  // ������ɒ���

    if (loadingIconTexture != -1) {
        // �A�C�R���̉�]�`��
        float scale = 1.0f + sinf(pulsePhase) * 0.1f; // �p���X����
        int scaledSize = (int)(ICON_SIZE * scale);

        // ��]�`��iDxLib�ɂ͒��ړI�ȉ�]�`�悪�Ȃ����߁A�ȈՓI�Ȍ��ʁj
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 200);
        DrawExtendGraph(
            ICON_X - scaledSize / 2, ICON_Y - scaledSize / 2,
            ICON_X + scaledSize / 2, ICON_Y + scaledSize / 2,
            loadingIconTexture, TRUE
        );
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
    else {
        // �A�C�R�����Ȃ��ꍇ�́A��]����~��`��
        float scale = 1.0f + sinf(pulsePhase) * 0.2f;
        int radius = (int)(30 * scale);

        // ��]����~��
        for (int i = 0; i < 8; i++) {
            float angle = iconRotation + (i * DX_PI / 4);
            int x = ICON_X + (int)(cosf(angle) * radius);
            int y = ICON_Y + (int)(sinf(angle) * radius);

            int alpha = 255 - (i * 25);
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
            DrawCircle(x, y, 8, GetColor(100, 150, 255), TRUE);
        }
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::DrawLoadingText()  // ���O�ύX: DrawText �� DrawLoadingText
{
    // �^�C�g���i���S�����z�u�j
    int titleWidth = GetDrawStringWidthToHandle(loadingTitle.c_str(),
        loadingTitle.length(), titleFontHandle);
    DrawStringToHandle(1920 / 2 - titleWidth / 2, 1080 / 2 - 180,  // ������Ɉړ�
        loadingTitle.c_str(), GetColor(255, 255, 255), titleFontHandle);

    // ���݂̃^�X�N�����i���S�����z�u�j
    if (!currentTaskText.empty()) {
        int taskWidth = GetDrawStringWidthToHandle(currentTaskText.c_str(),
            currentTaskText.length(), fontHandle);

        // �_�Ō���
        float alpha = 0.7f + 0.3f * sinf(animationTimer * 2.0f);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * alpha));
        DrawStringToHandle(1920 / 2 - taskWidth / 2, 1080 / 2 + 160,  // �ʒu����
            currentTaskText.c_str(), GetColor(200, 200, 255), fontHandle);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::DrawFade()
{
    if (fadeAlpha > 0.0f) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, (int)(255 * (1.0f - fadeAlpha)));
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void LoadingScene::AddCustomTask(const std::string& description,
    std::function<bool()> task, float weight)
{
    loadingTasks.emplace_back(description, task, weight);
}

void LoadingScene::ResetState()
{
    loadingActive = false;
    loadingComplete = false;
    loadingProgress = 0.0f;
    displayProgress = 0.0f;
    currentTaskIndex = 0;
    currentTaskProgress = 0.0f;
    animationTimer = 0.0f;
    iconRotation = 0.0f;
    pulsePhase = 0.0f;
    fadeAlpha = 0.0f;
    fadeIn = true;
    fadeOut = false;
    currentTaskText = "";

    loadingTasks.clear();
}

float LoadingScene::EaseInOut(float t)
{
    return t * t * (3.0f - 2.0f * t);
}

// ���[�f�B���O�����̎����֐�

bool LoadingScene::LoadGameResources()
{
    static int loadingStep = 0;
    static int frameCounter = 0;

    frameCounter++;

    // �i�K�I�ɓǂݍ��݁i�t���[�����U�j- ��蒷���ҋ@����
    switch (loadingStep) {
    case 0:
        // �X�e�b�v1�F��{���\�[�X�i1.5�b�j
        if (frameCounter >= 90) { // 90�t���[�� = 1.5�b
            loadingStep++;
            frameCounter = 0;
        }
        break;
    case 1:
        // �X�e�b�v2�F�Q�[�����\�[�X�i2�b�j
        if (frameCounter >= 120) { // 120�t���[�� = 2�b
            loadingStep++;
            frameCounter = 0;
        }
        break;
    case 2:
        // �X�e�b�v3�F�ŏI���i1�b�j
        if (frameCounter >= 60) { // 60�t���[�� = 1�b
            loadingStep = 0;
            frameCounter = 0;
            return true;
        }
        break;
    }

    return false;
}

bool LoadingScene::LoadCharacterSprites(int characterIdx)
{
    static int frameCounter = 0;
    frameCounter++;

    // �L�����N�^�[�X�v���C�g�̓ǂݍ��݂��V�~�����[�g�i2�b�j
    if (frameCounter >= 120) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadStageData(int stageIdx)
{
    static int frameCounter = 0;
    frameCounter++;

    // �X�e�[�W�f�[�^�̓ǂݍ��݂��V�~�����[�g�i2.5�b�j
    if (frameCounter >= 150) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadSoundResources()
{
    static int frameCounter = 0;
    frameCounter++;

    // �T�E���h���\�[�X�̓ǂݍ��݂��V�~�����[�g�i3�b�j
    if (frameCounter >= 180) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::LoadUIResources()
{
    static int frameCounter = 0;
    frameCounter++;

    // UI���\�[�X�̓ǂݍ��݂��V�~�����[�g�i1.5�b�j
    if (frameCounter >= 90) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::InitializeGameSystems()
{
    static int frameCounter = 0;
    frameCounter++;

    // �Q�[���V�X�e���̏��������V�~�����[�g�i2�b�j
    if (frameCounter >= 120) {
        frameCounter = 0;
        return true;
    }

    return false;
}

bool LoadingScene::WaitMinimumTime()
{
    static float waitTimer = 0.0f;
    waitTimer += 0.016f;

    // �ŏ��\�����Ԃ��m�ہi3�b�j
    if (waitTimer >= MIN_LOADING_TIME) {
        waitTimer = 0.0f;
        return true;
    }

    return false;
}