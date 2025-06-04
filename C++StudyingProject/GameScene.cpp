#include "GameScene.h"
#include <math.h>
using namespace std;

GameScene::GameScene()
    : selectedCharacterIndex(-1)
    , cameraX(0.0f)
    , previousPlayerX(0.0f)
    , exitRequested(false)
    , escPressed(false), escPressedPrev(false)
    , stageSelectPressed(false), stageSelectPressedPrev(false)
    , playerLife(6)      // �������C�t�F3�n�[�g��
    , playerCoins(0)     // �����R�C����
    , playerStars(0)     // ��������
    , currentStageIndex(0) // �����X�e�[�W
    , fadeState(FADE_NONE) // �t�F�[�h���
    , fadeAlpha(0.0f)    // �t�F�[�h�����x
    , fadeTimer(0.0f)    // �t�F�[�h�^�C�}�[
    , showingResult(false) // ���U���g�\�����
    , goalReached(false)   // �S�[�����B���
{
    backgroundHandle = -1;
    fontHandle = -1;
}

GameScene::~GameScene()
{
    DeleteGraph(backgroundHandle);
    DeleteFontToHandle(fontHandle);
}

void GameScene::Initialize(int selectedCharacter)
{
    selectedCharacterIndex = selectedCharacter;

    // �w�i�ƃt�H���g�ǂݍ���
    backgroundHandle = LoadGraph("Sprites/Backgrounds/background_fade_trees.png");
    fontHandle = CreateFontToHandle(NULL, 32, 3);

    // �L�����N�^�[���ݒ�
    characterName = GetCharacterDisplayName(selectedCharacter);

    // �v���C���[������
    gamePlayer.Initialize(selectedCharacter);

    // �X�e�[�W�V�X�e���������i�f�t�H���g��GrassStage�j
    currentStageIndex = 0;
    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // �J���������ʒu
    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // **HUD�V�X�e��������**
    InitializeHUD();

    // **�R�C���V�X�e���������i�X�e�[�W�����Łj**
    coinSystem.Initialize();
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);

    // **���V�X�e���������i�V�@�\�j**
    starSystem.Initialize();
    starSystem.GenerateStarsForStageIndex(currentStageIndex);

    // **���U���gUI�������i�V�@�\�j**
    resultUI.Initialize();

    // **�S�[���V�X�e��������**
    goalSystem.Initialize();
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);

    // ��ԃ��Z�b�g
    showingResult = false;
    goalReached = false;
}

void GameScene::InitializeHUD()
{
    // HUD�V�X�e���̏�����
    hudSystem.Initialize();

    // �v���C���[�L�����N�^�[��HUD�ɐݒ�
    hudSystem.SetPlayerCharacter(selectedCharacterIndex);

    // �������C�t�ƃR�C����ݒ�
    hudSystem.SetMaxLife(6);        // �ő僉�C�t�F3�n�[�g �~ 2 = 6
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.SetCoins(playerCoins);
    hudSystem.SetCollectedStars(playerStars);
    hudSystem.SetTotalStars(3);

    // HUD�\���ʒu��ݒ�i���ォ��30�s�N�Z���A�g��Łj
    hudSystem.SetPosition(30, 30);
    hudSystem.SetVisible(true);
}

void GameScene::Update()
{
    // ���U���g�\�����̏ꍇ�́A���U���g�̂ݍX�V
    if (showingResult) {
        UpdateResult();
        return;
    }

    // ���͍X�V
    UpdateInput();

    // �X�e�[�W�؂�ւ��i���U���g��\�����̂݁j
    if (stageSelectPressed && !stageSelectPressedPrev && fadeState == FADE_NONE && !showingResult) {
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        // �v���C���[�ʒu�����Z�b�g
        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // **�R�C���A���A�S�[�����Ĕz�u�i���V�X�e���Ή��j**
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);

        // ���W�J�E���g�����Z�b�g
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // ��Ԃ����S���Z�b�g
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **���U���gUI�����S���Z�b�g**
        resultUI.ResetState();
    }

    // �v���C���[�X�V
    gamePlayer.Update(&stageManager);

    // **�R�C���V�X�e���X�V�i�X�N���[�����W��HUD�̃R�C���A�C�R���ʒu��n���j**
    // HUD�R�C���A�C�R���̐��m�ȃX�N���[�����W���v�Z
    float hudCoinIconScreenX = 30 + 80 + 20 + 48 / 2; // �X�N���[�����W�i�J�����̉e���Ȃ��j
    float hudCoinIconScreenY = 30 + 64 + 20 + 48 / 2; // �X�N���[�����W�i�J�����̉e���Ȃ��j

    // �X�N���[�����W�����[���h���W�ɕϊ�
    float hudCoinIconWorldX = hudCoinIconScreenX + cameraX;
    float hudCoinIconWorldY = hudCoinIconScreenY; // Y���W�̓J�����̉e���Ȃ�

    coinSystem.Update(&gamePlayer, hudCoinIconWorldX, hudCoinIconWorldY);

    // **���V�X�e���X�V�i�V�@�\�j**
    starSystem.Update(&gamePlayer);

    // **�S�[���V�X�e���X�V**
    goalSystem.Update(&gamePlayer);

    // **�S�[���^�b�`���̃��U���g�\���i�ύX�j**
    if (goalSystem.IsGoalTouched() && !goalReached) {
        goalReached = true;
        ShowStageResult();
    }

    // **�t�F�[�h����**
    UpdateFade();

    // �J�����X�V�i���炩�ȃV�X�e�����g�p�j
    UpdateCamera();

    // �X�e�[�W�X�V
    stageManager.Update(cameraX);

    // **�Q�[�����W�b�N�X�V**
    UpdateGameLogic();

    // **HUD�X�V**
    UpdateHUD();

    // ESC�L�[�Ń^�C�g���ɖ߂�
    if (escPressed && !escPressedPrev) {
        exitRequested = true;
    }
}

void GameScene::UpdateGameLogic()
{
    // **�R�C�����W�����Q�[����Ԃɔ��f�i���A���^�C���X�V�j**
    int newCoinCount = coinSystem.GetCollectedCoinsCount();
    if (newCoinCount != playerCoins) {
        playerCoins = newCoinCount;
        // �R�C���l�����̃T�E���h��G�t�F�N�g�������ɒǉ��\
    }

    // **�����W�����Q�[����Ԃɔ��f�i�V�@�\�j**
    int newStarCount = starSystem.GetCollectedStarsCount();
    if (newStarCount != playerStars) {
        playerStars = newStarCount;
        // ���l�����̃T�E���h��G�t�F�N�g�������ɒǉ��\
    }

    // �����ŃQ�[�����W�b�N���X�V
    // ��F�_���[�W�����A�A�C�e���l���Ȃ�

    // **�e�X�g�p�F�L�[���͂Ń��C�t�𒲐�**
    static bool key1Pressed = false, key1PressedPrev = false;
    static bool key2Pressed = false, key2PressedPrev = false;
    static bool key3Pressed = false, key3PressedPrev = false;

    key1PressedPrev = key1Pressed;
    key2PressedPrev = key2Pressed;
    key3PressedPrev = key3Pressed;

    key1Pressed = CheckHitKey(KEY_INPUT_1) != 0;
    key2Pressed = CheckHitKey(KEY_INPUT_2) != 0;
    key3Pressed = CheckHitKey(KEY_INPUT_3) != 0;

    // �e�X�g�p����
    if (key1Pressed && !key1PressedPrev) {
        // 1�L�[�Ń��C�t����
        playerLife--;
        if (playerLife < 0) playerLife = 0;
    }

    if (key2Pressed && !key2PressedPrev) {
        // 2�L�[�Ń��C�t��
        playerLife++;
        if (playerLife > 6) playerLife = 6;
    }

    if (key3Pressed && !key3PressedPrev) {
        // 3�L�[�ŃR�C���E���S�z�u�i�e�X�g�p�j
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
    }
}

void GameScene::UpdateHUD()
{
    // HUD�V�X�e�����X�V
    hudSystem.Update();

    // �Q�[����Ԃ�HUD�ɔ��f
    hudSystem.SetCurrentLife(playerLife);
    hudSystem.SetCoins(playerCoins);
    hudSystem.SetCollectedStars(playerStars);  // �����𔽉f
    hudSystem.SetTotalStars(3);               // ��������ݒ�
}

void GameScene::UpdateFade()
{
    const float FADE_SPEED = 0.02f; // �t�F�[�h���x

    switch (fadeState) {
    case FADE_OUT:
        fadeTimer += 0.016f; // 60FPS�z��
        fadeAlpha = fadeTimer * (1.0f / FADE_SPEED);

        if (fadeAlpha >= 1.0f) {
            fadeAlpha = 1.0f;
            fadeState = FADE_IN;
            fadeTimer = 0.0f;

            // �X�e�[�W�؂�ւ�
            StartNextStage();
        }
        break;

    case FADE_IN:
        fadeTimer += 0.016f;
        fadeAlpha = 1.0f - (fadeTimer * (1.0f / FADE_SPEED));

        if (fadeAlpha <= 0.0f) {
            fadeAlpha = 0.0f;
            fadeState = FADE_NONE;
            fadeTimer = 0.0f;
        }
        break;

    case FADE_NONE:
    default:
        break;
    }
}

void GameScene::StartNextStage()
{
    // ���̃X�e�[�W�Ɉڍs
    currentStageIndex = (currentStageIndex + 1) % 5;

    // �X�e�[�W���ō��i4�Ԗځj�̏ꍇ�̓��ʏ���
    if (currentStageIndex >= 4) {
        // �ō��̃X�e�[�W - ���ʂȏ����������ɒǉ��\
        // ���݂͒ʏ�ʂ�X�e�[�W��ǂݍ���
    }

    stageManager.LoadStage((StageManager::StageType)currentStageIndex);

    // �v���C���[�ʒu�����Z�b�g
    gamePlayer.ResetPosition();
    cameraX = 0.0f;
    previousPlayerX = gamePlayer.GetX();

    // �R�C���A���A�S�[�����Ĕz�u
    coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
    starSystem.GenerateStarsForStageIndex(currentStageIndex);
    goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
    goalSystem.ResetGoal(); // �S�[����Ԃ����Z�b�g

    // ���W�J�E���g�����Z�b�g
    coinSystem.ResetCollectedCount();
    starSystem.ResetCollectedCount();

    // ��Ԃ����Z�b�g
    playerStars = 0;
    playerCoins = 0;
}

void GameScene::UpdateResult()
{
    resultUI.Update();

    // **���U���g�����S�ɔ�\���ɂȂ�����A�ʏ탂�[�h�ɖ߂�**
    if (resultUI.IsHidden()) {
        showingResult = false;
    }

    HandleResultButtons();
}

void GameScene::ShowStageResult()
{
    showingResult = true;
    playerStars = starSystem.GetCollectedStarsCount();
    resultUI.ShowResult(playerStars, 3, currentStageIndex + 1);
}

void GameScene::HandleResultButtons()
{
    ResultUISystem::ButtonAction action = resultUI.GetClickedButton();

    switch (action) {
    case ResultUISystem::BUTTON_RETRY:
        // �X�e�[�W�����g���C
        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // �V�X�e�������Z�b�g
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        goalSystem.ResetGoal();

        // ���W�J�E���g�����Z�b�g
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // ��Ԃ����S���Z�b�g
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **���U���gUI�����S�Ƀ��Z�b�g**
        resultUI.ResetState();
        break;

    case ResultUISystem::BUTTON_NEXT_STAGE:
        // ���̃X�e�[�W��
        currentStageIndex = (currentStageIndex + 1) % 5;
        stageManager.LoadStage((StageManager::StageType)currentStageIndex);

        gamePlayer.ResetPosition();
        cameraX = 0.0f;
        previousPlayerX = gamePlayer.GetX();

        // �V�X�e�[�W�̃V�X�e��������
        coinSystem.GenerateCoinsForStageIndex(currentStageIndex);
        starSystem.GenerateStarsForStageIndex(currentStageIndex);
        goalSystem.PlaceGoalForStage(currentStageIndex, &stageManager);
        goalSystem.ResetGoal();

        // ���W�J�E���g�����Z�b�g
        coinSystem.ResetCollectedCount();
        starSystem.ResetCollectedCount();

        // ��Ԃ����S���Z�b�g
        playerStars = 0;
        playerCoins = 0;
        showingResult = false;
        goalReached = false;

        // **���U���gUI�����S�Ƀ��Z�b�g**
        resultUI.ResetState();
        break;

    default:
        break;
    }
}

void GameScene::Draw()
{
    // **�w�i�`��i�V�[�����X�ȃ^�C�����O�j**
    DrawSeamlessBackground();

    // �X�e�[�W�`��
    stageManager.Draw(cameraX);

    // **�R�C���`��**
    coinSystem.Draw(cameraX);

    // **���`��i�V�@�\�j**
    starSystem.Draw(cameraX);

    // **�S�[���`��**
    goalSystem.Draw(cameraX);

    // �v���C���[�`��
    gamePlayer.Draw(cameraX);

    // �v���C���[�̉e��`��
    gamePlayer.DrawShadow(cameraX, &stageManager);

    // **HUD�V�X�e���`��i�őO�ʁj**
    hudSystem.Draw();

    // UI�`��i�f�o�b�O���Ȃǁj
    if (!showingResult) {  // ���U���g�\�����̓f�o�b�O�����B��
        DrawUI();
    }

    // **���U���gUI�`��i�V�@�\�j**
    resultUI.Draw();

    // **�t�F�[�h�`��i�őO�ʁj**
    DrawFade();
}

void GameScene::DrawSeamlessBackground()
{
    // �w�i�摜�̃T�C�Y���擾
    int bgWidth, bgHeight;
    GetGraphSize(backgroundHandle, &bgWidth, &bgHeight);

    // �p�����b�N�X���ʁi�w�i�͑O�i���x���ړ��j
    float parallaxSpeed = 0.3f; // �w�i�̈ړ����x�i0.3�{�j
    float bgOffsetX = cameraX * parallaxSpeed;

    // �w�i���^�C����ɕ`�悷�邽�߂̊J�n�ʒu���v�Z
    int startTileX = (int)(bgOffsetX / bgWidth) - 1; // �]�T��������1���O����
    int endTileX = startTileX + (SCREEN_W / bgWidth) + 3; // �]�T��������3����܂�

    // �w�i���V�[�����X�ɕ`��
    for (int tileX = startTileX; tileX <= endTileX; tileX++) {
        float drawX = tileX * bgWidth - bgOffsetX;

        // �w�i�摜��`��
        DrawExtendGraph(
            (int)drawX, 0,
            (int)drawX + bgWidth, SCREEN_H,
            backgroundHandle, TRUE
        );
    }
}

void GameScene::DrawFade()
{
    if (fadeState != FADE_NONE && fadeAlpha > 0.0f) {
        int alpha = (int)(255 * fadeAlpha);
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
        DrawBox(0, 0, SCREEN_W, SCREEN_H, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void GameScene::UpdateInput()
{
    // �O�t���[���̓��͂�ۑ�
    escPressedPrev = escPressed;
    stageSelectPressedPrev = stageSelectPressed;

    // ���݂̓��͂��擾
    escPressed = CheckHitKey(KEY_INPUT_ESCAPE) != 0;
    stageSelectPressed = CheckHitKey(KEY_INPUT_TAB) != 0;
}

void GameScene::UpdateCamera()
{
    float currentPlayerX = gamePlayer.GetX();

    // �v���C���[����ʂ̏��������ɔz�u�i��̎��E���m�ہj
    float targetCameraX = currentPlayerX - SCREEN_W * 0.35f;

    // �͈͐���
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) {
        targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;
    }

    // ���Ɋ��炩�ȒǏ]�i�x�߂̐ݒ�Ńu����h�~�j
    cameraX = Lerp(cameraX, targetCameraX, CAMERA_FOLLOW_SPEED);

    // �O�t���[���̃v���C���[�ʒu���X�V
    previousPlayerX = currentPlayerX;
}

void GameScene::UpdateCameraSimple()
{
    float currentPlayerX = gamePlayer.GetX();

    // �v���C���[����ʂ̏��������ɔz�u
    float targetCameraX = currentPlayerX - SCREEN_W * 0.35f;

    // �͈͐���
    if (targetCameraX < 0) targetCameraX = 0;
    if (targetCameraX > Stage::STAGE_WIDTH - SCREEN_W) {
        targetCameraX = Stage::STAGE_WIDTH - SCREEN_W;
    }

    // ���Ɋ��炩�ȒǏ]�i���x�߂̐ݒ�j
    cameraX = Lerp(cameraX, targetCameraX, 0.03f); // 0.03f�ł�芊�炩��
}

void GameScene::DrawUI()
{
    // �Q�[���^�C�g��
    string gameTitle = "STAGE ADVENTURE";
    int titleWidth = GetDrawStringWidthToHandle(gameTitle.c_str(), (int)gameTitle.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - titleWidth / 2, 20, gameTitle.c_str(), GetColor(255, 255, 255), fontHandle);

    // �I�����ꂽ�L�����N�^�[���\��
    string characterInfo = "Playing as: " + characterName;
    int infoWidth = GetDrawStringWidthToHandle(characterInfo.c_str(), (int)characterInfo.length(), fontHandle);
    DrawStringToHandle(SCREEN_W / 2 - infoWidth / 2, 60, characterInfo.c_str(), GetColor(255, 215, 0), fontHandle);

    // ��������i�������w�i�t���j
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 180);
    DrawBox(20, SCREEN_H - 220, 700, SCREEN_H - 20, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    DrawStringToHandle(30, SCREEN_H - 210, "Controls:", GetColor(255, 255, 255), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 170, "Left/Right: Move, Space: Jump, Down: Duck", GetColor(200, 200, 200), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 130, "TAB: Change Stage, ESC: Return to title", GetColor(200, 200, 200), fontHandle);

    // **�e�X�g�p��������i�g���j**
    DrawStringToHandle(30, SCREEN_H - 90, "Test: 1/2: Life -/+, 3: Reset Items", GetColor(150, 150, 150), fontHandle);

    // �f�o�b�O��������
    string debugInfo = "=== DEBUG INFO ===";
    DrawStringToHandle(30, SCREEN_H - 200, debugInfo.c_str(), GetColor(255, 255, 0), fontHandle);

    // �v���C���[�ʒu���
    string posInfo = "Position: (" + to_string((int)gamePlayer.GetX()) + ", " + to_string((int)gamePlayer.GetY()) + ")";
    DrawStringToHandle(30, SCREEN_H - 170, posInfo.c_str(), GetColor(150, 150, 150), fontHandle);

    // **HUD��ԏ��ƃJ�����f�o�b�O���**
    string hudInfo = "Life: " + to_string(playerLife) + "/6, Coins: " + to_string(playerCoins) + ", Stars: " + to_string(playerStars) + "/3";
    DrawStringToHandle(30, SCREEN_H - 60, hudInfo.c_str(), GetColor(100, 200, 255), fontHandle);

    // ���͏�ԕ\��
    string inputInfo = "Input: ";
    if (CheckHitKey(KEY_INPUT_LEFT)) inputInfo += "[LEFT] ";
    if (CheckHitKey(KEY_INPUT_RIGHT)) inputInfo += "[RIGHT] ";
    if (CheckHitKey(KEY_INPUT_SPACE)) inputInfo += "[SPACE] ";
    if (CheckHitKey(KEY_INPUT_DOWN)) inputInfo += "[DOWN] ";
    if (inputInfo == "Input: ") inputInfo += "NONE";
    DrawStringToHandle(30, SCREEN_H - 140, inputInfo.c_str(), GetColor(200, 200, 100), fontHandle);

    // �v���C���[�̓������
    string stateDetail = "OnGround: " + string(gamePlayer.GetState() != Player::JUMPING && gamePlayer.GetState() != Player::FALLING ? "YES" : "NO");
    DrawStringToHandle(30, SCREEN_H - 110, stateDetail.c_str(), GetColor(100, 200, 100), fontHandle);

    // ���݂̏�ԕ\��
    string stateInfo = "State: ";
    switch (gamePlayer.GetState()) {
    case Player::IDLE: stateInfo += "IDLE"; break;
    case Player::WALKING: stateInfo += "WALKING"; break;
    case Player::JUMPING: stateInfo += "JUMPING"; break;
    case Player::FALLING: stateInfo += "FALLING"; break;
    case Player::DUCKING: stateInfo += "DUCKING"; break;
    }
    stateInfo += ", Direction: " + string(gamePlayer.IsFacingRight() ? "Right" : "Left");
    DrawStringToHandle(SCREEN_W - 500, 30, stateInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // �J�����ʒu�\��
    string cameraInfo = "Camera: " + to_string((int)cameraX);
    DrawStringToHandle(SCREEN_W - 500, 70, cameraInfo.c_str(), GetColor(100, 100, 100), fontHandle);

    // **�S�[���ƃX�e�[�W�f�o�b�O���**
    string stageInfo = "Stage: " + to_string(currentStageIndex + 1) + "/5";
    string goalInfo = "Goal: " + string(goalSystem.IsGoalTouched() ? "TOUCHED!" : "Active") + ", Result: " + string(showingResult ? "SHOWING" : "HIDDEN");
    DrawStringToHandle(30, SCREEN_H - 40, stageInfo.c_str(), GetColor(255, 200, 100), fontHandle);
    DrawStringToHandle(30, SCREEN_H - 20, goalInfo.c_str(), GetColor(100, 255, 200), fontHandle);
}

std::string GameScene::GetCharacterDisplayName(int index)
{
    switch (index) {
    case 0: return "Beige Knight";
    case 1: return "Green Ranger";
    case 2: return "Pink Warrior";
    case 3: return "Purple Mage";
    case 4: return "Yellow Hero";
    default: return "Unknown Hero";
    }
}

float GameScene::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

float GameScene::SmoothLerp(float current, float target, float speed)
{
    // SmoothDamp���̊��炩�ȕ��
    float distance = target - current;

    // ���������ɏ������ꍇ�͒��ڐݒ�
    if (fabsf(distance) < 0.1f) {
        return target;
    }

    // �C�[�W���O�A�E�g����
    float t = 1.0f - powf(1.0f - speed, 60.0f * 0.016f); // 60FPS�z��
    return current + distance * t;
}