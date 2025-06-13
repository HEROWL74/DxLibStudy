#pragma once
#pragma once
#include "EnemyBase.h"

class Fly : public EnemyBase {
public:
    Fly(float startX, float startY);
    ~Fly() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // �n�G�ŗL�̏��
    enum FlyState {
        FLY_RESTING,        // �x�e��
        FLY_BUZZING,        // �u���u����щ��
        FLY_CHASING,        // �v���C���[��ǐ�
        FLY_FLEEING         // ������
    };

    FlyState flyState;
    float buzzTimer;            // �u���u���^�C�}�[
    float restTimer;            // �x�e�^�C�}�[
    float chaseTimer;           // �ǐՃ^�C�}�[
    float buzzPhaseX;           // X�����̃u���u���ʑ�
    float buzzPhaseY;           // Y�����̃u���u���ʑ�
    float originalX;            // ����X���W
    float originalY;            // ����Y���W
    float fleeTimer;            // �����^�C�}�[

    // �n�G�̃p�����[�^
    static constexpr float BUZZ_AMPLITUDE_X = 30.0f;   // X�����u���u���U��
    static constexpr float BUZZ_AMPLITUDE_Y = 15.0f;   // Y�����u���u���U��
    static constexpr float BUZZ_SPEED_X = 0.08f;       // X�����u���u�����x
    static constexpr float BUZZ_SPEED_Y = 0.12f;       // Y�����u���u�����x
    static constexpr float CHASE_SPEED = 4.0f;         // �ǐՑ��x
    static constexpr float FLEE_SPEED = 6.0f;          // �������x
    static constexpr float REST_DURATION = 1.0f;       // �x�e����
    static constexpr float BUZZ_DURATION = 4.0f;       // �u���u������
    static constexpr float CHASE_DURATION = 3.0f;      // �ǐՎ���
    static constexpr float FLEE_DURATION = 2.0f;       // ��������

    // �n�G�ŗL�̍s��
    void UpdateFlyMovement();
    void UpdateFlyAnimation();
    void StartBuzzing();
    void StartChasing();
    void StartFleeing();
    void HandlePlayerDetectionFly(Player* player);

    // �n�G�ŗL�̃X�v���C�g�擾
    int GetFlySprite();
    void UpdateAnimation();
};