#include "UIManager.h"
#include <math.h>

UIManager::UIManager()
{
    playButtonHandle = -1;
    optionsButtonHandle = -1;
    exitButtonHandle = -1;
    checkmarkHandle = -1;
    rectangleHandle = -1;
    sliderHandle = -1;
    homeHandle = -1;

    isInOptionsMenu = false;
    startButtonPressed = false;
    frameCounter = 0;
    titlePulse = 0;

    masterVolume = 0.8f;
    sfxVolume = 0.7f;
    fullScreen = false;

    mouseX = mouseY = 0;
    mousePressed = false;
    mousePressedPrev = false;
}

UIManager::~UIManager()
{
    Finalize();
}

void UIManager::Initialize()
{
    LoadUIImages();
    SetupButtons();
}

void UIManager::LoadUIImages()
{
    // UI�摜�̓ǂݍ���
    playButtonHandle = LoadGraph("UI/PNG/Yellow/buttonLong_yellow.png");
    rectangleHandle = LoadGraph("UI/PNG/Yellow/buttonSquare_yellow.png");
    checkmarkHandle = LoadGraph("UI/PNG/Yellow/check_square_color_checkmark.png");
    sliderHandle = LoadGraph("UI/PNG/Yellow/slideHorlzontal.png");
    homeHandle = LoadGraph("UI/PNG/Yellow/home.png");

    // �����摜�𕡐��̃{�^���Ŏg�p
    optionsButtonHandle = playButtonHandle;
    exitButtonHandle = playButtonHandle;
}

void UIManager::SetupButtons()
{
    // �X�^�[�g�{�^��
    startButton.x = 540;
    startButton.y = 350;
    startButton.width = 200;
    startButton.height = 60;
    startButton.normalHandle = playButtonHandle;

    // �I�v�V�����{�^��
    optionsButton.x = 540;
    optionsButton.y = 430;
    optionsButton.width = 200;
    optionsButton.height = 60;
    optionsButton.normalHandle = optionsButtonHandle;

    // �I���{�^��
    exitButton.x = 540;
    exitButton.y = 510;
    exitButton.width = 200;
    exitButton.height = 60;
    exitButton.normalHandle = exitButtonHandle;

    // �߂�{�^���i�I�v�V������ʗp�j
    backButton.x = 50;
    backButton.y = 50;
    backButton.width = 60;
    backButton.height = 60;
    backButton.normalHandle = homeHandle;
}

void UIManager::UpdateStartScreen()
{
    frameCounter++;
    titlePulse = sin(frameCounter * 0.05f);

    // �}�E�X���͂̎擾
    GetMousePoint(&mouseX, &mouseY);
    mousePressedPrev = mousePressed;
    mousePressed = (GetMouseInput() & MOUSE_INPUT_LEFT) != 0;

    if (!isInOptionsMenu)
    {
        // ���C�����j���[�̍X�V
        UpdateButton(startButton);
        UpdateButton(optionsButton);
        UpdateButton(exitButton);

        // �{�^���N���b�N����
        if (IsButtonClicked(startButton))
        {
            startButtonPressed = true;
        }
        else if (IsButtonClicked(optionsButton))
        {
            isInOptionsMenu = true;
        }
        else if (IsButtonClicked(exitButton))
        {
            // �I�������i���ۂ̃Q�[���ł͓K�؂ȏI�������������j
        }
    }
    else
    {
        // �I�v�V�������j���[�̍X�V
        UpdateButton(backButton);

        if (IsButtonClicked(backButton))
        {
            isInOptionsMenu = false;
        }

        // �I�v�V�������ڂ̍X�V�i�ȈՎ����j
        // ���ۂ̃Q�[���ł͂��ڍׂȎ������K�v
    }
}

void UIManager::UpdateButton(Button& button)
{
    // �}�E�X���{�^����ɂ��邩�`�F�b�N
    bool wasHovered = button.isHovered;
    button.isHovered = (mouseX >= button.x && mouseX <= button.x + button.width &&
        mouseY >= button.y && mouseY <= button.y + button.height);

    // �z�o�[�G�t�F�N�g�̃X�P�[������
    if (button.isHovered)
    {
        button.targetScale = 1.1f;
    }
    else
    {
        button.targetScale = 1.0f;
    }

    // �X���[�Y�ȃX�P�[���ω�
    float scaleDiff = button.targetScale - button.scale;
    button.scale += scaleDiff * 0.15f;

    // �N���b�N����
    button.isPressed = button.isHovered && mousePressed && !mousePressedPrev;
}

void UIManager::DrawStartScreen()
{
    if (!isInOptionsMenu)
    {
        // �^�C�g�����S�̕`��
        int titleColor = GetColor(255, 255, 255);
        int titleSize = 72;
        float pulse = 1.0f + titlePulse * 0.1f;

        // �^�C�g���e�L�X�g�i�e�t���j
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, 100);
        DrawStringToHandle(642, 152, "EPIC GAME", GetColor(0, 0, 0), CreateFontToHandle(NULL, (int)(titleSize * pulse), 3));
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

        DrawStringToHandle(640, 150, "EPIC GAME", titleColor, CreateFontToHandle(NULL, (int)(titleSize * pulse), 3));

        // �T�u�^�C�g��
        DrawStringToHandle(570, 220, "The Ultimate Adventure", GetColor(200, 200, 200), CreateFontToHandle(NULL, 24, 2));

        // �{�^���̕`��
        DrawButton(startButton);
        DrawButton(optionsButton);
        DrawButton(exitButton);

        // �{�^���̃e�L�X�g
        DrawStringToHandle(startButton.x + 75, startButton.y + 20, "START", GetColor(50, 50, 50), CreateFontToHandle(NULL, 24, 3));
        DrawStringToHandle(optionsButton.x + 65, optionsButton.y + 20, "OPTIONS", GetColor(50, 50, 50), CreateFontToHandle(NULL, 24, 3));
        DrawStringToHandle(exitButton.x + 85, exitButton.y + 20, "EXIT", GetColor(50, 50, 50), CreateFontToHandle(NULL, 24, 3));

        // �o�[�W�������
        DrawStringToHandle(20, 680, "Version 1.0.0", GetColor(150, 150, 150), CreateFontToHandle(NULL, 16, 1));
    }
    else
    {
        DrawOptionsMenu();
    }
}

void UIManager::DrawOptionsMenu()
{
    // �w�i�̈É�
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 150);
    DrawBox(0, 0, 1280, 720, GetColor(0, 0, 0), TRUE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

    // �I�v�V�������j���[�̔w�i
    DrawBox(300, 150, 980, 570, GetColor(40, 40, 40), TRUE);
    DrawBox(300, 150, 980, 570, GetColor(255, 255, 255), FALSE);

    // �^�C�g��
    DrawStringToHandle(570, 180, "OPTIONS", GetColor(255, 255, 255), CreateFontToHandle(NULL, 36, 3));

    // ���ʐݒ�
    DrawSlider(350, 250, 200, masterVolume, "Master Volume");
    DrawSlider(350, 320, 200, sfxVolume, "SFX Volume");

    // �t���X�N���[���ݒ�
    DrawCheckbox(350, 390, fullScreen, "Fullscreen");

    // �߂�{�^��
    DrawButton(backButton);

    // �ݒ�l�̕\��
    char volumeText[32];
    sprintf_s(volumeText, "%.0f%%", masterVolume * 100);
    DrawStringToHandle(570, 250, volumeText, GetColor(255, 255, 255), CreateFontToHandle(NULL, 20, 2));

    sprintf_s(volumeText, "%.0f%%", sfxVolume * 100);
    DrawStringToHandle(570, 320, volumeText, GetColor(255, 255, 255), CreateFontToHandle(NULL, 20, 2));
}

void UIManager::DrawButton(const Button& button)
{
    if (button.normalHandle == -1) return;

    // �{�^���̃X�P�[���K�p
    int scaledWidth = (int)(button.width * button.scale);
    int scaledHeight = (int)(button.height * button.scale);
    int offsetX = (scaledWidth - button.width) / 2;
    int offsetY = (scaledHeight - button.height) / 2;

    // �z�o�[���̌������
    if (button.isHovered)
    {
        SetDrawBlendMode(DX_BLENDMODE_ADD, 100);
        DrawExtendGraph(button.x - offsetX - 5, button.y - offsetY - 5,
            button.x + scaledWidth + 5, button.y + scaledHeight + 5,
            button.normalHandle, TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }

    // �ʏ�̃{�^���`��
    DrawExtendGraph(button.x - offsetX, button.y - offsetY,
        button.x + scaledWidth, button.y + scaledHeight,
        button.normalHandle, TRUE);
}

bool UIManager::IsButtonClicked(const Button& button)
{
    return button.isPressed;
}

void UIManager::DrawSlider(int x, int y, int width, float value, const char* label)
{
    // ���x��
    DrawStringToHandle(x, y - 25, label, GetColor(255, 255, 255), CreateFontToHandle(NULL, 18, 2));

    // �X���C�_�[�̔w�i
    DrawBox(x, y, x + width, y + 20, GetColor(100, 100, 100), TRUE);
    DrawBox(x, y, x + width, y + 20, GetColor(200, 200, 200), FALSE);

    // �X���C�_�[�̒l����
    int valueWidth = (int)(width * value);
    DrawBox(x, y, x + valueWidth, y + 20, GetColor(255, 215, 0), TRUE);

    // �X���C�_�[�̃n���h��
    int handleX = x + valueWidth - 10;
    if (rectangleHandle != -1)
    {
        DrawExtendGraph(handleX, y - 5, handleX + 20, y + 25, rectangleHandle, TRUE);
    }
    else
    {
        DrawBox(handleX, y - 5, handleX + 20, y + 25, GetColor(255, 255, 255), TRUE);
    }
}

void UIManager::DrawCheckbox(int x, int y, bool checked, const char* label)
{
    // �`�F�b�N�{�b�N�X�̔w�i
    if (rectangleHandle != -1)
    {
        DrawExtendGraph(x, y, x + 30, y + 30, rectangleHandle, TRUE);
    }
    else
    {
        DrawBox(x, y, x + 30, y + 30, GetColor(200, 200, 200), TRUE);
        DrawBox(x, y, x + 30, y + 30, GetColor(100, 100, 100), FALSE);
    }

    // �`�F�b�N�}�[�N
    if (checked)
    {
        if (checkmarkHandle != -1)
        {
            DrawExtendGraph(x + 5, y + 5, x + 25, y + 25, checkmarkHandle, TRUE);
        }
        else
        {
            DrawLine(x + 8, y + 15, x + 12, y + 20, GetColor(0, 255, 0), 3);
            DrawLine(x + 12, y + 20, x + 22, y + 10, GetColor(0, 255, 0), 3);
        }
    }

    // ���x��
    DrawStringToHandle(x + 40, y + 5, label, GetColor(255, 255, 255), CreateFontToHandle(NULL, 18, 2));
}

void UIManager::Draw()
{
    DrawStartScreen();
}

void UIManager::Finalize()
{
    // �摜�n���h���̉��
    if (playButtonHandle != -1)
    {
        DeleteGraph(playButtonHandle);
        playButtonHandle = -1;
    }
    if (checkmarkHandle != -1)
    {
        DeleteGraph(checkmarkHandle);
        checkmarkHandle = -1;
    }
    if (rectangleHandle != -1)
    {
        DeleteGraph(rectangleHandle);
        rectangleHandle = -1;
    }
    if (sliderHandle != -1)
    {
        DeleteGraph(sliderHandle);
        sliderHandle = -1;
    }
    if (homeHandle != -1)
    {
        DeleteGraph(homeHandle);
        homeHandle = -1;
    }

    // �����n���h�����Q�Ƃ��Ă�����̂�NULL�ɐݒ�
    optionsButtonHandle = -1;
    exitButtonHandle = -1;
}