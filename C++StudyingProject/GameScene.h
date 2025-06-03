#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"  // HUD�V�X�e����ǉ�
#include "CoinSystem.h" // �R�C���V�X�e����ǉ�
#include "GoalSystem.h" // �S�[���V�X�e����ǉ�
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
    Player gamePlayer;
    HUDSystem hudSystem;  // HUD�V�X�e����ǉ�
    CoinSystem coinSystem; // �R�C���V�X�e����ǉ�
    GoalSystem goalSystem; // �S�[���V�X�e����ǉ�

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterName;

    // �J�����V�X�e���i���炩�ȒǏ]�p�Ɋȑf���j
    float cameraX;              // �J������X���W
    float previousPlayerX;      // �O�t���[���̃v���C���[X���W

    // **�J�����̊��炩�Ǐ]�萔�i�V���v�����j**
    static constexpr float CAMERA_FOLLOW_SPEED = 0.05f;  // �J�����Ǐ]���x�i�x�߂Ŋ��炩�j

    // ��ԊǗ�
    bool exitRequested;

    // �L�[����
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev; // �X�e�[�W�؂�ւ��p

    // **�ǉ��F�Q�[����ԊǗ�**
    int playerLife;      // �v���C���[�̃��C�t
    int playerCoins;     // �v���C���[�̃R�C����
    int currentStageIndex; // ���݂̃X�e�[�W�C���f�b�N�X

    // **�ǉ��F�t�F�[�h���o**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // �w���p�[�֐�
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateCameraSimple();  // �V���v���ŃJ�����i�I�v�V�����j
    void UpdateGameLogic();  // �Q�[�����W�b�N�X�V��ǉ�
    void DrawUI();
    void DrawSeamlessBackground(); // �V�[�����X�w�i�`���ǉ�

    // **�ǉ��F�t�F�[�h�֘A�̊֐�**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **�ǉ��FHUD�֘A�̊֐�**
    void InitializeHUD();
    void UpdateHUD();

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
    float SmoothLerp(float current, float target, float speed);  // ���炩�ȃC�[�W���O�p
};