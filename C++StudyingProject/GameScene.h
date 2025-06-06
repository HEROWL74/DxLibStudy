#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"    
#include "CoinSystem.h"   
#include "GoalSystem.h"   
#include "StarSystem.h"   
#include "ResultUISystem.h" 
#include "EnemyManager.h" 
#include "DoorSystem.h"   // **�V�ǉ��F�h�A�V�X�e��**
#include "SoundManager.h"
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
    bool IsStageChangeRequested() const { return stageChangeRequested; }
    int GetRequestedStageIndex() const { return requestedStageIndex; }

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
    HUDSystem hudSystem;
    CoinSystem coinSystem;
    GoalSystem goalSystem;
    StarSystem starSystem;
    ResultUISystem resultUI;
    EnemyManager enemyManager;
    DoorSystem doorSystem;      // **�V�ǉ��F�h�A�V�X�e��**

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterName;

    // �J�����V�X�e��
    float cameraX;
    float previousPlayerX;

    // **�J�����̊��炩�Ǐ]�萔**
    static constexpr float CAMERA_FOLLOW_SPEED = 0.05f;

    // ��ԊǗ�
    bool exitRequested;

    // �L�[����
    bool escPressed, escPressedPrev;
    bool stageSelectPressed, stageSelectPressedPrev;

    // **�Q�[����ԊǗ�**
    int playerLife;
    int playerCoins;
    int playerStars;
    int currentStageIndex;

    // **�t�F�[�h���o**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // **���U���g�\������**
    bool showingResult;
    bool goalReached;

    // **�V�ǉ��F�h�A�֘A�̏�ԊǗ�**
    bool doorOpened;            // �h�A���J������
    bool playerEnteringDoor;    // �v���C���[���h�A�ɓ����Ă��邩


    // **�V�ǉ��F�x���������s�V�X�e��**
    bool pendingAutoWalk;       // �������s���ҋ@����
    int autoWalkDelayFrames;    // �������s�J�n�܂ł̒x���t���[����
    // **�G�Ƃ̑��ݍ�p�p�Ǘ�**
    bool playerInvulnerable;
    float invulnerabilityTimer;
    static constexpr float INVULNERABILITY_DURATION = 2.0f;

    // **�V�ǉ��F�x���������s�֐��̐錾**
    void UpdateDelayedAutoWalk();

    // �w���p�[�֐�
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateCameraSimple();
    void UpdateGameLogic();
    void DrawUI();
    void DrawSeamlessBackground();

    // **�t�F�[�h�֐�**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **HUD�@�\�֐�**
    void InitializeHUD();
    void UpdateHUD();

    // **���U���g�@�\�֐�**
    void UpdateResult();
    void ShowStageResult();
    void HandleResultButtons();

    // **�G�Ƃ̑��ݍ�p�p�֐�**
    void UpdatePlayerEnemyInteractions();
    void HandlePlayerDamage(int damage);
    void HandlePlayerEnemyCollision();
    void UpdatePlayerInvulnerability();

    bool CheckIfPlayerStompedEnemy();
    void HandleSuccessfulStomp();
    void ApplyStompBounce();

    void UpdatePlayerAutoWalk();

    // **�V�ǉ��F�h�A�֘A�̏����֐�**
    void UpdateDoorInteraction();
    void HandleGoalReached();
    void HandlePlayerEnteredDoor();

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
    float SmoothLerp(float current, float target, float speed);

    bool stageChangeRequested;
    int requestedStageIndex;

};