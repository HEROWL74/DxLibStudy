#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <string>

class GoalSystem {
public:
    // ���̐F
    enum FlagColor {
        FLAG_BLUE,
        FLAG_YELLOW,
        FLAG_GREEN,
        FLAG_RED
    };

    // �S�[���̏��
    enum GoalState {
        GOAL_ACTIVE,    // �A�N�e�B�u���
        GOAL_TOUCHED,   // �v���C���[���G�ꂽ
        GOAL_COMPLETED  // �S�[������
    };

    GoalSystem();
    ~GoalSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // �S�[���Ǘ�
    void SetGoal(float x, float y, FlagColor color);
    void ClearGoal();
    bool IsGoalTouched() const { return goalState == GOAL_TOUCHED || goalState == GOAL_COMPLETED; }
    bool IsGoalCompleted() const { return goalState == GOAL_COMPLETED; }
    void ResetGoal() { goalState = GOAL_ACTIVE; goalTouchTimer = 0.0f; }

    // ���݂̃X�e�[�W�ɃS�[���������z�u
    void PlaceGoalForStage(int stageIndex, StageManager* stageManager);

private:
    // �e�N�X�`���n���h��
    struct FlagTextures {
        int blue_a, blue_b;
        int yellow_a, yellow_b;
        int green_a, green_b;
        int red_a, red_b;
    } flagTextures;

    // �S�[�����
    float goalX, goalY;         // �S�[���ʒu
    FlagColor currentColor;     // ���݂̊��̐F
    GoalState goalState;        // �S�[���̏��
    bool goalExists;            // �S�[�������݂��邩

    // �A�j���[�V����
    float animationTimer;       // �A�j���[�V�����^�C�}�[
    bool currentFrame;          // ���݂̃t���[���ifalse=a, true=b�j
    float goalTouchTimer;       // �S�[���^�b�`��̃^�C�}�[

    // �G�t�F�N�g
    float glowIntensity;        // �O���[���ʂ̋��x
    float bobPhase;             // �㉺���V����

    // �萔
    static constexpr float FLAG_WIDTH = 64.0f;          // ���̕�
    static constexpr float FLAG_HEIGHT = 96.0f;         // ���̍���
    static constexpr float GOAL_DETECTION_WIDTH = 80.0f; // �S�[�����蕝
    static constexpr float GOAL_DETECTION_HEIGHT = 120.0f; // �S�[�����荂��
    static constexpr float ANIMATION_SPEED = 0.02f;     // �A�j���[�V�������x�i�������j
    static constexpr float BOB_SPEED = 0.03f;           // ���V���x
    static constexpr float BOB_AMPLITUDE = 4.0f;        // ���V�U��
    static constexpr float GOAL_TOUCH_DURATION = 2.0f;  // �S�[���^�b�`���o����

    // �w���p�[�֐�
    void LoadTextures();
    int GetCurrentFlagTexture();
    void UpdateAnimation();
    void UpdateEffects();
    bool CheckPlayerCollision(Player* player);
    void DrawGoalFlag(float cameraX);
    void DrawGoalEffects(float cameraX);
    float GetDistance(float x1, float y1, float x2, float y2);

    // �u���b�N�̏�ɔz�u���邽�߂̃w���p�[
    float FindGroundLevel(float x, StageManager* stageManager);
};