#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include <string>

class GameScene
{
public:
    GameScene();
    ~GameScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }

private:
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �e�N�X�`���n���h��
    int backgroundHandle;
    int fontHandle;

    // �Q�[���I�u�W�F�N�g
    StageManager stageManager;
    Player gamePlayer;  // 'player'����'gamePlayer'�ɕύX

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterName;

    // �J�����V�X�e��
    float cameraX;              // �J������X���W
    float targetCameraX;        // �J�����̖ڕWX���W

    // �����萔
    static constexpr float CAMERA_LERP = 0.08f;     // �J�����̒Ǐ]���x

    // ��ԊǗ�
    bool exitRequested;

    // �L�[����
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev; // �X�e�[�W�؂�ւ��p

    // �w���p�[�֐�
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void DrawUI();

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
};