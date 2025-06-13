#pragma once
#include "EnemyBase.h"

class Ladybug : public EnemyBase {
public:
    Ladybug(float startX, float startY);
    ~Ladybug() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // �e���g�E���V�ŗL�̏��
    enum LadybugState {
        LADYBUG_RESTING,    // �x�e��
        LADYBUG_WALKING,    // ���s��
        LADYBUG_FLYING,     // ��s��
        LADYBUG_LANDING     // ���n��
    };

    LadybugState ladybugState;
    float restTimer;            // �x�e�^�C�}�[
    float walkTimer;            // ���s�^�C�}�[
    float flyTimer;             // ��s�^�C�}�[
    float flyPhase;             // ��s�ʑ�
    float targetY;              // �ڕWY���W
    float originalY;            // �n��Y���W

    // �e���g�E���V�̃p�����[�^
    static constexpr float FLY_AMPLITUDE = 25.0f;      // ��s�U��
    static constexpr float FLY_SPEED_PHASE = 0.1f;     // ��s�ʑ����x
    static constexpr float FLY_SPEED = 3.0f;           // ��s���x
    static constexpr float WALK_SPEED = 1.5f;          // ���s���x
    static constexpr float REST_DURATION = 2.0f;       // �x�e����
    static constexpr float WALK_DURATION = 3.0f;       // ���s����
    static constexpr float FLY_DURATION = 4.0f;        // ��s����
    static constexpr float FLY_HEIGHT = 60.0f;         // ��s���x

    // �e���g�E���V�ŗL�̍s��
    void UpdateLadybugMovement();
    void UpdateLadybugAnimation();
    void StartWalking();
    void StartFlying();
    void StartLanding();
    void HandleGroundCollisionLadybug(StageManager* stageManager);

    // �e���g�E���V�ŗL�̃X�v���C�g�擾
    int GetLadybugSprite();
};