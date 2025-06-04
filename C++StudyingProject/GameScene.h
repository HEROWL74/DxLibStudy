#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include "HUDSystem.h"    // HUD�V�X�e����ǉ�
#include "CoinSystem.h"   // �R�C���V�X�e����ǉ�
#include "GoalSystem.h"   // �S�[���V�X�e����ǉ�
#include "StarSystem.h"   // **���V�X�e����ǉ��i�V�@�\�j**
#include "ResultUISystem.h" // **���U���gUI�V�X�e����ǉ��i�V�@�\�j**
#include "EnemyManager.h" // **�G�Ǘ��V�X�e����ǉ��i�V�@�\�j**
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
    HUDSystem hudSystem;        // HUD�V�X�e����ǉ�
    CoinSystem coinSystem;      // �R�C���V�X�e����ǉ�
    GoalSystem goalSystem;      // �S�[���V�X�e����ǉ�
    StarSystem starSystem;      // **���V�X�e����ǉ��i�V�@�\�j**
    ResultUISystem resultUI;    // **���U���gUI�V�X�e����ǉ��i�V�@�\�j**
    EnemyManager enemyManager;  // **�G�Ǘ��V�X�e����ǉ��i�V�@�\�j**

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterName;

    // �J�����V�X�e���i���炩�ȒǏ]�p�ɊȒP���j
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
    int playerStars;     // **�v���C���[�̐����i�V�@�\�j**
    int currentStageIndex; // ���݂̃X�e�[�W�C���f�b�N�X

    // **�ǉ��F�t�F�[�h���o**
    enum FadeState { FADE_NONE, FADE_OUT, FADE_IN };
    FadeState fadeState;
    float fadeAlpha;
    float fadeTimer;

    // **���U���g�\������i�V�@�\�j**
    bool showingResult;
    bool goalReached;

    // **�G�Ƃ̑��ݍ�p�Ǘ��i�V�@�\�j**
    bool playerInvulnerable;     // �v���C���[�̖��G���
    float invulnerabilityTimer;  // ���G���ԃ^�C�}�[
    static constexpr float INVULNERABILITY_DURATION = 2.0f; // ���G����

    // �w���p�[�֐�
    std::string GetCharacterDisplayName(int index);
    void UpdateInput();
    void UpdateCamera();
    void UpdateCameraSimple();  // �V���v���ŃJ�����i�I�v�V�����j
    void UpdateGameLogic();  // �Q�[�����W�b�N�X�V��ǉ�
    void DrawUI();
    void DrawSeamlessBackground(); // �V�[�����X�w�i�`���ǉ�

    // **�ǉ��F�t�F�[�h�֐�**
    void UpdateFade();
    void DrawFade();
    void StartNextStage();

    // **�ǉ��FHUD�֘A�֐�**
    void InitializeHUD();
    void UpdateHUD();

    // **�ǉ��F���U���g�֘A�֐��i�V�@�\�j**
    void UpdateResult();
    void ShowStageResult();
    void HandleResultButtons();

    // **�ǉ��F�G�Ƃ̑��ݍ�p�֐��i�V�@�\�j**
    void UpdatePlayerEnemyInteractions();
    void HandlePlayerDamage(int damage);
    void HandlePlayerEnemyCollision();
    void UpdatePlayerInvulnerability();

    bool CheckIfPlayerStompedEnemy();
    void HandleSuccessfulStomp();
    void ApplyStompBounce();

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
    float SmoothLerp(float current, float target, float speed);  // ���炩�ȃC�[�W���O�p
};