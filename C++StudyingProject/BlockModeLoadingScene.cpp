#include "BlockModeLoadingScene.h"
#include "SoundManager.h"
#include <cmath>
#include <algorithm>
#include <random>
using namespace std;

BlockModeLoadingScene::BlockModeLoadingScene()
    : loadingComplete(false)
    , loadingProgress(0.0f)
    , loadingTimer(0.0f)
    , selectedCharacterIndex(0)
    , particleSpawnTimer(0.0f)
    , titlePulsePhase(0.0f)
    , iconFloatPhase(0.0f)
    , glowPulsePhase(0.0f)
    , currentStage(STAGE_INTRO)
    , stageTimer(0.0f)
    , currentMessageIndex(0)
    , messageTimer(0.0f)
{
    // �n���h��������
    backgroundHandle = -1;
    fontHandle = largeFontHandle = titleFontHandle = -1;
    dirtBlockHandle = stoneBlockHandle = crateBlockHandle = -1;
    coinHandle = explosiveHandle = playerIconHandle = -1;
    toolHammerHandle = toolPickaxeHandle = -1;
    progressBarBgHandle = progressBarFillHandle = -1;
    decorFrameHandle = glowEffectHandle = -1;
}

BlockModeLoadingScene::~BlockModeLoadingScene()
{
    // ���\�[�X���
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
    DeleteFontToHandle(largeFontHandle);
    DeleteFontToHandle(titleFontHandle);

    DeleteGraph(dirtBlockHandle);
    DeleteGraph(stoneBlockHandle);
    DeleteGraph(crateBlockHandle);
    DeleteGraph(coinHandle);
    DeleteGraph(explosiveHandle);
    // �L�����N�^�[�p�e�N�X�`�����폜
    DeleteGraph(playerIconHandle);
    DeleteGraph(toolHammerHandle);
    DeleteGraph(toolPickaxeHandle);
    DeleteGraph(progressBarBgHandle);
    DeleteGraph(progressBarFillHandle);
    DeleteGraph(decorFrameHandle);
    DeleteGraph(glowEffectHandle);
}

void BlockModeLoadingScene::Initialize()
{
    LoadTextures();

    // ���[�f�B���O���b�Z�[�W�̏����i�ق̂ڂ̒��ɕύX�j
    loadingMessages = {
        "Setting up your cozy block world...",
        "Preparing friendly building blocks...",
        "Creating a safe play environment...",
        "Loading colorful block pieces...",
        "Setting up fun adventures...",
        "Preparing your building tools...",
        "Almost ready for block fun...",
        "Welcome to Block Athletics!"
    };

    // �ق̂ڂ̂������[�f�B���OTips
    loadingTips = {
        "Take your time and enjoy the adventure",
        "Collect coins at your own pace",
        "Use arrow keys to move around",
        "Jump with the space bar when ready",
        "Look for hidden surprises along the way",
        "Building blocks can be fun to destroy too",
        "Every player finds their own path",
        "Have fun and don't worry about speed"
    };

    OutputDebugStringA("BlockModeLoadingScene: Professional loading system initialized\n");
}

void BlockModeLoadingScene::StartLoading(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;
    characterName = GetCharacterDisplayName(selectedCharacter);

    // ��ԃ��Z�b�g
    loadingComplete = false;
    loadingProgress = 0.0f;
    loadingTimer = 0.0f;
    particleSpawnTimer = 0.0f;
    titlePulsePhase = 0.0f;
    iconFloatPhase = 0.0f;
    glowPulsePhase = 0.0f;
    currentStage = STAGE_INTRO;
    stageTimer = 0.0f;
    currentMessageIndex = 0;
    messageTimer = 0.0f;

    // �G�t�F�N�g�N���A
    particles.clear();
    loadingElements.clear();

    // ���[�f�B���O�v�f������
    InitializeLoadingElements();

    // BGM�J�n�i�Â��Ńv���t�F�b�V���i���ȕ��͋C�j
    SoundManager::GetInstance().PlayBGM(SoundManager::BGM_TITLE);

    char debugMsg[256];
    sprintf_s(debugMsg, "BlockModeLoadingScene: Started loading for character %d (%s)\n",
        selectedCharacter, characterName.c_str());
    OutputDebugStringA(debugMsg);
}

void BlockModeLoadingScene::Update()
{
    const float deltaTime = 0.016f; // 60FPS�z��

    UpdateLoading();
    UpdateParticles();
    UpdateLoadingElements();
    UpdateLoadingStage();

    // �A�j���[�V�����t�F�[�Y�X�V
    titlePulsePhase += TITLE_PULSE_SPEED;
    iconFloatPhase += ICON_FLOAT_SPEED;
    glowPulsePhase += GLOW_PULSE_SPEED;

    // ���b�Z�[�W�X�V
    messageTimer += deltaTime;
    if (messageTimer >= MESSAGE_DURATION) {
        messageTimer = 0.0f;
        currentMessageIndex = (currentMessageIndex + 1) % loadingMessages.size();
    }

    // �p�[�e�B�N������
    particleSpawnTimer += deltaTime;
    if (particleSpawnTimer >= PARTICLE_SPAWN_RATE) {
        particleSpawnTimer = 0.0f;
        CreateParticles();
    }
}

void BlockModeLoadingScene::UpdateLoading()
{
    const float deltaTime = 0.016f;
    loadingTimer += deltaTime;

    // �X���[�Y�ȃv���O���X�v�Z�i�C�[�W���O�K�p�j
    float rawProgress = loadingTimer / LOADING_DURATION;
    if (rawProgress > 1.0f) rawProgress = 1.0f;

    loadingProgress = EaseOutCubic(rawProgress);

    // ���[�f�B���O�����`�F�b�N
    if (loadingTimer >= LOADING_DURATION && !loadingComplete) {
        loadingComplete = true;
        currentStage = STAGE_COMPLETE;

        // �����G�t�F�N�g
        for (int i = 0; i < 30; i++) {
            CreateSparkleParticle(
                SCREEN_W / 2 + (rand() % 400 - 200),
                SCREEN_H / 2 + (rand() % 200 - 100)
            );
        }

        OutputDebugStringA("BlockModeLoadingScene: Loading completed with professional effects\n");
    }
}

void BlockModeLoadingScene::UpdateParticles()
{
    for (auto it = particles.begin(); it != particles.end();) {
        it->life -= 0.016f;
        it->x += it->velocityX;
        it->y += it->velocityY;
        it->velocityY += 0.3f; // �d��
        it->rotation += it->rotationSpeed;
        it->scale = it->life / it->maxLife;

        if (it->life <= 0.0f) {
            it = particles.erase(it);
        }
        else {
            ++it;
        }
    }
}

void BlockModeLoadingScene::UpdateLoadingElements()
{
    for (auto& element : loadingElements) {
        if (!element.isVisible) continue;

        // �^�[�Q�b�g�ʒu�ւ̈ړ�
        element.x = Lerp(element.x, element.targetX, 0.08f);
        element.y = Lerp(element.y, element.targetY, 0.08f);

        // �A�j���[�V�����X�V
        element.animationPhase += 0.05f;
        element.rotation += 0.02f;

        // �t���[�e�B���O����
        element.y += sinf(element.animationPhase) * 2.0f;
    }
}

void BlockModeLoadingScene::UpdateLoadingStage()
{
    stageTimer += 0.016f;
    float stageProgress = loadingProgress * 4.0f; // 4�X�e�[�W�ɕ���

    if (stageProgress >= 1.0f && currentStage == STAGE_INTRO) {
        currentStage = STAGE_BUILDING;
        stageTimer = 0.0f;

        // �\�z�X�e�[�W�p�G�t�F�N�g
        for (int i = 0; i < 10; i++) {
            CreateBlockParticle(
                300 + i * 100,
                SCREEN_H / 2,
                rand() % 3
            );
        }
    }
    else if (stageProgress >= 2.0f && currentStage == STAGE_BUILDING) {
        currentStage = STAGE_FINALIZING;
        stageTimer = 0.0f;
    }
    else if (stageProgress >= 3.0f && currentStage == STAGE_FINALIZING) {
        currentStage = STAGE_COMPLETE;
        stageTimer = 0.0f;
    }
}

void BlockModeLoadingScene::CreateParticles()
{
    if (particles.size() >= MAX_PARTICLES) return;

    // �ق̂ڂ̂����u���b�N���ォ��������~���Ă���
    switch (currentStage) {
    case STAGE_INTRO:
        if (rand() % 4 == 0) {
            CreateBlockParticle(
                rand() % SCREEN_W,
                -50,  // ��ʏ㕔����J�n
                rand() % 3  // �y�A�΁A�N���[�g
            );
        }
        break;

    case STAGE_BUILDING:
        if (rand() % 5 == 0) {
            CreateBlockParticle(
                rand() % SCREEN_W,
                -50,
                rand() % 5  // �S��ނ̃u���b�N
            );
        }
        if (rand() % 8 == 0) {
            CreateCoinParticle(
                rand() % SCREEN_W,
                -50
            );
        }
        break;

    case STAGE_FINALIZING:
        if (rand() % 6 == 0) {
            CreateSparkleParticle(
                rand() % SCREEN_W,
                -30
            );
        }
        break;

    default:
        break;
    }
}

void BlockModeLoadingScene::CreateBlockParticle(float x, float y, int blockType)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 40 - 20) * 0.02f;  // ������肵�����ړ�
    p.velocityY = (rand() % 30 + 20) * 0.03f;  // ������肵������
    p.life = p.maxLife = 8.0f + (rand() % 400) * 0.01f;  // ��������
    p.scale = 0.6f + (rand() % 40) * 0.01f;    // �����߃T�C�Y
    p.rotation = 0.0f;
    p.rotationSpeed = (rand() % 100 - 50) * 0.001f;  // ��������]
    p.type = blockType;

    // �����݂̂���F����
    switch (blockType) {
    case 0: p.color = GetColor(160, 100, 60); break;   // �������y�F
    case 1: p.color = GetColor(150, 140, 130); break;  // �D�����ΐF
    case 2: p.color = GetColor(180, 120, 80); break;   // �������N���[�g�F
    case 3: p.color = GetColor(200, 140, 100); break;  // �D���������N���[�g�F
    case 4: p.color = GetColor(220, 200, 80); break;   // �D�����R�C���F
    }

    particles.push_back(p);
}

void BlockModeLoadingScene::CreateSparkleParticle(float x, float y)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 100 - 50) * 0.08f;
    p.velocityY = (rand() % 100 - 50) * 0.08f;
    p.life = p.maxLife = 2.0f;
    p.scale = 1.2f;
    p.rotation = 0.0f;
    p.rotationSpeed = (rand() % 200 - 100) * 0.01f;
    p.type = 1;
    p.color = GetColor(255, 255, 100 + rand() % 155);

    particles.push_back(p);
}

void BlockModeLoadingScene::CreateCoinParticle(float x, float y)
{
    Particle p;
    p.x = x;
    p.y = y;
    p.velocityX = (rand() % 80 - 40) * 0.06f;
    p.velocityY = (rand() % 60 - 80) * 0.06f;
    p.life = p.maxLife = 2.5f;
    p.scale = 1.0f;
    p.rotation = 0.0f;
    p.rotationSpeed = 0.1f;
    p.type = 2;
    p.color = GetColor(255, 215, 0);

    particles.push_back(p);
}

void BlockModeLoadingScene::InitializeLoadingElements()
{
    loadingElements.clear();

    // ��ʒ����Ƀu���b�N�v�f��z�u
    int centerX = SCREEN_W / 2;
    int centerY = SCREEN_H / 2;

    for (int i = 0; i < 8; i++) {
        LoadingElement element;
        element.x = -100; // ��ʊO����J�n
        element.y = centerY + (i - 4) * 60;
        element.targetX = centerX - 300 + i * 80;
        element.targetY = element.y;
        element.scale = 1.0f;
        element.rotation = 0.0f;
        element.alpha = 1.0f;
        element.animationPhase = i * 0.3f;
        element.isVisible = true;
        element.elementType = i % 4;

        loadingElements.push_back(element);
    }
}

void BlockModeLoadingScene::UpdateElementAnimations()
{
}

void BlockModeLoadingScene::Draw()
{
    DrawBackground();
    DrawLoadingElements();
    DrawBlockConstructionAnimation();
    DrawCharacterInfo();
    DrawTitleAndInfo();
    DrawProgressBar();
    DrawLoadingTips();
    DrawParticles();
    DrawEffects();
}

void BlockModeLoadingScene::DrawBackground()
{
    // �ق̂ڂ̂������F�w�i�i�^�C�����O�j
    if (backgroundHandle != -1) {
        int bgWidth = 512;
        int bgHeight = 512;

        int tilesX = (SCREEN_W / bgWidth) + 2;
        int tilesY = (SCREEN_H / bgHeight) + 2;

        for (int x = 0; x < tilesX; x++) {
            for (int y = 0; y < tilesY; y++) {
                DrawGraph(x * bgWidth, y * bgHeight, backgroundHandle, TRUE);
            }
        }
    }
    else {
        // �t�H�[���o�b�N�F�����݂̂���O���f�[�V����
        for (int y = 0; y < SCREEN_H; y += 4) {
            float t = (float)y / SCREEN_H;
            int r = (int)Lerp(245, 222, t);  // ���F�̃O���f�[�V����
            int g = (int)Lerp(222, 184, t);
            int b = (int)Lerp(179, 135, t);

            DrawLine(0, y, SCREEN_W, y, GetColor(r, g, b));
            DrawLine(0, y + 1, SCREEN_W, y + 1, GetColor(r, g, b));
            DrawLine(0, y + 2, SCREEN_W, y + 2, GetColor(r, g, b));
            DrawLine(0, y + 3, SCREEN_W, y + 3, GetColor(r, g, b));
        }
    }
}

void BlockModeLoadingScene::DrawTitleAndInfo()
{
    // �ق̂ڂ̃^�C�g���i�T���߂ȃp���X���ʁj
    string mainTitle = "Block Athletics Loading...";

    // �D�����^�C�g���`��i�O���[���T���߂Ɂj
    int titleWidth = GetDrawStringWidthToHandle(mainTitle.c_str(), mainTitle.length(), largeFontHandle);
    int titleX = SCREEN_W / 2 - titleWidth / 2;

    // �e���ʁi�D�����F�j
    DrawStringToHandle(titleX + 2, 122, mainTitle.c_str(), GetColor(150, 120, 80), largeFontHandle);
    DrawStringToHandle(titleX, 120, mainTitle.c_str(), GetColor(80, 60, 40), largeFontHandle);

    // �T�u�^�C�g��
    string subtitle = "Preparing your cozy adventure...";
    int subtitleWidth = GetDrawStringWidthToHandle(subtitle.c_str(), subtitle.length(), fontHandle);
    int subtitleX = SCREEN_W / 2 - subtitleWidth / 2;
    DrawStringToHandle(subtitleX, 180, subtitle.c_str(), GetColor(120, 100, 70), fontHandle);

    // ���݂̃��[�f�B���O���b�Z�[�W�i�D�����F���j
    if (currentMessageIndex < loadingMessages.size()) {
        string message = loadingMessages[currentMessageIndex];
        int messageWidth = GetDrawStringWidthToHandle(message.c_str(), message.length(), fontHandle);
        int messageX = SCREEN_W / 2 - messageWidth / 2;

        // ���b�Z�[�W�̔w�i�i�D�����F�j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 120);
        DrawBox(messageX - 20, 700, messageX + messageWidth + 20, 740, GetColor(200, 180, 140), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        DrawStringToHandle(messageX, 710, message.c_str(), GetColor(90, 70, 50), fontHandle);
    }
}

void BlockModeLoadingScene::DrawProgressBar()
{
    DrawAnimatedProgressBar();
}

void BlockModeLoadingScene::DrawAnimatedProgressBar()
{
    int barX = SCREEN_W / 2 - PROGRESS_BAR_WIDTH / 2;
    int barY = SCREEN_H - 200;

    // �ق̂ڂ̔w�i
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(barX - 5, barY - 5, barX + PROGRESS_BAR_WIDTH + 5, barY + PROGRESS_BAR_HEIGHT + 5,
        GetColor(160, 130, 90), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �w�i
    DrawBox(barX, barY, barX + PROGRESS_BAR_WIDTH, barY + PROGRESS_BAR_HEIGHT,
        GetColor(220, 200, 160), TRUE);

    // �v���O���X�����i�������F���j
    int progressWidth = (int)(PROGRESS_BAR_WIDTH * loadingProgress);
    if (progressWidth > 0) {
        // �������O���f�[�V����
        for (int i = 0; i < progressWidth; i += 4) {
            float t = (float)i / progressWidth;
            int r = (int)Lerp(180, 140, t);
            int g = (int)Lerp(140, 110, t);
            int b = (int)Lerp(100, 70, t);

            DrawBox(barX + i, barY + 2, barX + i + 4, barY + PROGRESS_BAR_HEIGHT - 2,
                GetColor(r, g, b), TRUE);
        }

        // �T���߂Ȍ������
        SetDrawBlendMode(DX_BLENDMODE_ADD, 60);
        DrawBox(barX + 2, barY + 4, barX + progressWidth - 2, barY + PROGRESS_BAR_HEIGHT / 2,
            GetColor(255, 220, 180), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �g���i�D�����F�j
    DrawBox(barX, barY, barX + PROGRESS_BAR_WIDTH, barY + PROGRESS_BAR_HEIGHT,
        GetColor(120, 100, 70), FALSE);

    // �p�[�Z���g�\��
    char percentText[32];
    sprintf_s(percentText, "%.1f%%", loadingProgress * 100);
    int percentWidth = GetDrawStringWidthToHandle(percentText, strlen(percentText), fontHandle);
    DrawStringToHandle(barX + PROGRESS_BAR_WIDTH / 2 - percentWidth / 2, barY + 5,
        percentText, GetColor(80, 60, 40), fontHandle);
}

void BlockModeLoadingScene::DrawCharacterInfo()
{
    // �I�����ꂽ�L�����N�^�[���i�ق̂ڂ̒��j
    int infoX = 100;
    int infoY = 300;

    // �L�����N�^�[���p�l���i�������F���j
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 160);
    DrawBox(infoX - 20, infoY - 20, infoX + 400, infoY + 200, GetColor(210, 190, 150), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �p�l���̘g���i�D�����F�j
    DrawBox(infoX - 20, infoY - 20, infoX + 400, infoY + 200, GetColor(140, 120, 80), FALSE);

    // �L�����N�^�[��
    string charTitle = "Your Character:";
    DrawStringToHandle(infoX, infoY, charTitle.c_str(), GetColor(100, 80, 60), fontHandle);
    DrawStringToHandle(infoX, infoY + 40, characterName.c_str(), GetColor(80, 60, 40), largeFontHandle);

    // �L�����N�^�[�X�e�[�^�X�i�ق̂ڂ̕����j
    DrawStringToHandle(infoX, infoY + 100, "Status: Ready for Adventure!", GetColor(90, 130, 70), fontHandle);
    DrawStringToHandle(infoX, infoY + 140, "Specialty: Block Building Fun", GetColor(90, 110, 130), fontHandle);
}

void BlockModeLoadingScene::DrawStageInfo()
{
}

void BlockModeLoadingScene::DrawLoadingTips()
{
    // �ق̂ڂ̂���Tip��\��
    static int currentTipIndex = 0;
    static float tipTimer = 0.0f;

    tipTimer += 0.016f;
    if (tipTimer >= 3.0f) { // 3�b���Ƃ�Tip�ύX�i�������j
        tipTimer = 0.0f;
        currentTipIndex = (currentTipIndex + 1) % loadingTips.size();
    }

    int tipX = SCREEN_W - 500;
    int tipY = 300;

    // Tip�p�l���i�ق̂ڂ̐F���j
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(tipX - 20, tipY - 20, tipX + 480, tipY + 180, GetColor(200, 180, 140), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawBox(tipX - 20, tipY - 20, tipX + 480, tipY + 180, GetColor(120, 100, 70), FALSE);

    // Tip���e�i�D�����F�j
    DrawStringToHandle(tipX, tipY, "Helpful Tips:", GetColor(90, 70, 50), fontHandle);

    if (currentTipIndex < loadingTips.size()) {
        string tip = loadingTips[currentTipIndex];
        DrawStringToHandle(tipX, tipY + 50, tip.c_str(), GetColor(80, 60, 40), fontHandle);
    }
}

void BlockModeLoadingScene::DrawBlockConstructionAnimation()
{
    // �ق̂ڂ̃u���b�N�g�ݗ��ăA�j���[�V����
    int constructionX = SCREEN_W / 2 - 300;
    int constructionY = SCREEN_H / 2 - 50;

    // �\�z�i�x�ɉ����ău���b�N��\���i�������j
    int blocksToShow = (int)(loadingProgress * 8);

    for (int i = 0; i < blocksToShow && i < 8; i++) {
        int blockX = constructionX + (i % 4) * 120;
        int blockY = constructionY + (i / 4) * 120;

        // �u���b�N�̎�ނ��C���f�b�N�X�Ō���
        int blockType = i % 5;
        int blockHandle = -1;

        switch (blockType) {
        case 0: blockHandle = dirtBlockHandle; break;
        case 1: blockHandle = stoneBlockHandle; break;
        case 2: blockHandle = crateBlockHandle; break;
        case 3: blockHandle = explosiveHandle; break;
        case 4: blockHandle = coinHandle; break;
        }

        // �u���b�N�̓o��A�j���[�V�����i�������j
        float appearProgress = (loadingProgress * 8 - i);
        if (appearProgress > 1.0f) appearProgress = 1.0f;
        if (appearProgress < 0.0f) appearProgress = 0.0f;

        float scale = EaseOutCubic(appearProgress);
        int size = (int)(100 * scale);

        if (size > 0 && blockHandle != -1) {
            // �D�����e����
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 80);
            DrawBox(blockX + 5, blockY + 5, blockX + size + 5, blockY + size + 5, GetColor(120, 100, 70), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // ���ۂ̃u���b�N�e�N�X�`����`��
            int drawX = blockX + (100 - size) / 2;
            int drawY = blockY + (100 - size) / 2;

            DrawExtendGraph(drawX, drawY, drawX + size, drawY + size, blockHandle, TRUE);

            // �T���߂Ȍ������
            SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(40 * scale));
            DrawBox(drawX + 5, drawY + 5, drawX + size - 5, drawY + size / 3, GetColor(255, 240, 200), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
        }
    }

    // �I�����ꂽ�L�����N�^�[���ق̂ڂ̕\��
    if (loadingProgress > 0.7f && playerIconHandle != -1) {
        float charScale = EaseOutCubic((loadingProgress - 0.7f) * 3.33f);
        int charSize = (int)(120 * charScale);

        if (charSize > 0) {
            int charX = SCREEN_W / 2 + 150;
            int charY = SCREEN_H / 2 - 40;

            // �D�����e
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
            DrawBox(charX + 5, charY + 5, charX + charSize + 5, charY + charSize + 5, GetColor(120, 100, 70), TRUE);
            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // �L�����N�^�[�e�N�X�`���`��
            DrawExtendGraph(charX, charY, charX + charSize, charY + charSize, playerIconHandle, TRUE);
        }
    }
}

void BlockModeLoadingScene::DrawLoadingElements()
{
    for (const auto& element : loadingElements) {
        if (!element.isVisible) continue;

        // ���V����
        float yOffset = sinf(element.animationPhase) * 5.0f;

        // �v�f�^�C�v�ɉ������`��
        int elementColor;
        switch (element.elementType) {
        case 0: elementColor = GetColor(255, 215, 0); break;  // �R�C��
        case 1: elementColor = GetColor(255, 100, 100); break; // ������
        case 2: elementColor = GetColor(100, 255, 100); break; // �c�[��
        case 3: elementColor = GetColor(100, 200, 255); break; // ���̑�
        }

        int elementX = (int)element.x;
        int elementY = (int)(element.y + yOffset);
        int elementSize = (int)(40 * element.scale);

        // �O���[����
        DrawGlowEffect(elementX - 5, elementY - 5, elementSize + 10, elementSize + 10,
            0.4f, elementColor);

        // �v�f�{�́i��]���ʕt���j
        float rotation = element.rotation;
        DrawCircle(elementX + elementSize / 2, elementY + elementSize / 2, elementSize / 2,
            elementColor, TRUE);

        // �����̏ڍ�
        SetDrawBlendMode(DX_BLENDMODE_ADD, 150);
        DrawCircle(elementX + elementSize / 2, elementY + elementSize / 2, elementSize / 4,
            GetColor(255, 255, 255), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void BlockModeLoadingScene::DrawParticles()
{
    for (const auto& particle : particles) {
        if (particle.life <= 0.0f) continue;

        int alpha = (int)(180 * (particle.life / particle.maxLife));  // �T���߂ȃA���t�@
        int size = (int)(16 * particle.scale);  // �����傫�߂̃p�[�e�B�N��

        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

        switch (particle.type) {
        case 0: // �u���b�N�Ёi�ۂ��`�łق̂ڂ́j
        case 2: // �R�C��
            DrawCircle((int)particle.x, (int)particle.y, size, particle.color, TRUE);
            // �T���߂ȃn�C���C�g
            DrawCircle((int)particle.x - 2, (int)particle.y - 2, size / 3, GetColor(255, 240, 200), TRUE);
            break;

        case 1: // �X�p�[�N���i�D�������^�j
            // �D�������^�̕`��
            for (int i = 0; i < 4; i++) {
                float angle = particle.rotation + i * 1.57f; // 90�x����
                int x1 = (int)(particle.x + cosf(angle) * size * 0.8f);
                int y1 = (int)(particle.y + sinf(angle) * size * 0.8f);

                DrawCircle(x1, y1, size / 4, particle.color, TRUE);
            }
            DrawCircle((int)particle.x, (int)particle.y, size / 2, particle.color, TRUE);
            break;
        }

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void BlockModeLoadingScene::DrawEffects()
{
    // �������̂ق̂ڂ̃G�t�F�N�g
    if (currentStage == STAGE_COMPLETE) {
        float completeGlow = sinf(glowPulsePhase) * 0.3f + 0.7f;  // �T���߂ȃp���X

        // ��ʑS�̗̂D������
        SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(30 * completeGlow));
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(255, 240, 200), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        // �������b�Z�[�W�i�ق̂ڂ́j
        string completeText = "Ready for your cozy adventure!";
        int completeWidth = GetDrawStringWidthToHandle(completeText.c_str(), completeText.length(), largeFontHandle);
        int completeX = SCREEN_W / 2 - completeWidth / 2;

        // �D�����e
        DrawStringToHandle(completeX + 2, SCREEN_H / 2 + 122, completeText.c_str(), GetColor(150, 120, 80), largeFontHandle);
        DrawStringToHandle(completeX, SCREEN_H / 2 + 120, completeText.c_str(), GetColor(80, 60, 40), largeFontHandle);
    }
}

void BlockModeLoadingScene::DrawGlowEffect(int x, int y, int width, int height, float intensity, int color)
{
    if (intensity <= 0.01f) return;

    int alpha = (int)(intensity * 100);
    SetDrawBlendMode(DX_BLENDMODE_ADD, alpha);

    // �������C���[�ŃO���[����
    for (int i = 0; i < 5; i++) {
        int offset = (i + 1) * 3;
        DrawBox(x - offset, y - offset, x + width + offset, y + height + offset, color, FALSE);
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
}

void BlockModeLoadingScene::DrawPulsingText(const std::string& text, int x, int y, int fontHandle, int color, float pulsePhase)
{
    float pulse = 1.0f + sinf(pulsePhase * 2) * 0.1f;

    // �e����
    DrawStringToHandle(x + 3, y + 3, text.c_str(), GetColor(0, 0, 0), fontHandle);

    // �O���[����
    for (int i = 0; i < 3; i++) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 30);
        DrawStringToHandle(x - i, y - i, text.c_str(), color, fontHandle);
        DrawStringToHandle(x + i, y + i, text.c_str(), color, fontHandle);
    }
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // ���C���e�L�X�g
    DrawStringToHandle(x, y, text.c_str(), color, fontHandle);
}

void BlockModeLoadingScene::LoadTextures()
{
    // �ق̂ڂ̔w�i�e�N�X�`��
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_solid_sand.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);
    largeFontHandle = CreateFontToHandle(NULL, 48, 5);
    titleFontHandle = CreateFontToHandle(NULL, 64, 7);

    // �u���b�N�֘A�e�N�X�`���iBlockAthleticsScene�Ɠ����摜���g�p�j
    dirtBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_dirt.png");
    stoneBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_stone.png");
    crateBlockHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate.png");
    coinHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_box_coin.png");
    explosiveHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_crate_explosive.png");

    // �L�����N�^�[�p�e�N�X�`�����ǉ�
    switch (selectedCharacterIndex) {
    case 0: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_d.png"); break; // beige
    case 1: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_a.png"); break; // green
    case 2: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_e.png"); break; // pink
    case 3: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_b.png"); break; // purple
    case 4: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_c.png"); break; // yellow
    default: playerIconHandle = LoadGraph("MiniGamesTiles/PNG/Special/extra_character_d.png"); break;
    }

    OutputDebugStringA("BlockModeLoadingScene: Cozy textures loaded\n");
}

std::string BlockModeLoadingScene::GetCharacterDisplayName(int index)
{
    // BlockAthleticsScene�Ɠ����L�����N�^�[�����g�p
    switch (index) {
    case 0: return "Beige Builder";
    case 1: return "Green Constructor";
    case 2: return "Pink Architect";
    case 3: return "Purple Engineer";
    case 4: return "Yellow Athlete";
    default: return "Block Runner";
    }
}

std::string BlockModeLoadingScene::GetRandomLoadingTip()
{
    if (loadingTips.empty()) return "";
    return loadingTips[rand() % loadingTips.size()];
}

float BlockModeLoadingScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float BlockModeLoadingScene::EaseOutCubic(float t)
{
    return 1 - powf(1 - t, 3);
}

float BlockModeLoadingScene::EaseInOutQuad(float t)
{
    return t < 0.5f ? 2 * t * t : 1 - powf(-2 * t + 2, 2) / 2;
}

int BlockModeLoadingScene::GetRandomColor()
{
    int colors[] = {
        GetColor(255, 100, 100),
        GetColor(100, 255, 100),
        GetColor(100, 100, 255),
        GetColor(255, 255, 100),
        GetColor(255, 100, 255),
        GetColor(100, 255, 255)
    };

    return colors[rand() % 6];
}

void BlockModeLoadingScene::ResetState()
{
    loadingComplete = false;
    loadingProgress = 0.0f;
    loadingTimer = 0.0f;
    particleSpawnTimer = 0.0f;
    titlePulsePhase = 0.0f;
    iconFloatPhase = 0.0f;
    glowPulsePhase = 0.0f;
    currentStage = STAGE_INTRO;
    stageTimer = 0.0f;
    currentMessageIndex = 0;
    messageTimer = 0.0f;

    particles.clear();
    loadingElements.clear();

    OutputDebugStringA("BlockModeLoadingScene: State reset for new loading session\n");
}