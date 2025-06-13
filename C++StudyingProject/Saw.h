#pragma once
#include "EnemyBase.h"

class Saw : public EnemyBase {
public:
    Saw(float startX, float startY);
    ~Saw() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // �m�R�M���ŗL�̏��
    enum SawState {
        SAW_RESTING,        // �x�e��
        SAW_SPINNING,       // ��]��
        SAW_MOVING,         // �ړ���
        SAW_ATTACKING       // �U����
    };

    SawState sawState;
    float restTimer;            // �x�e�^�C�}�[
    float spinTimer;            // ��]�^�C�}�[
    float moveTimer;            // �ړ��^�C�}�[
    float attackTimer;          // �U���^�C�}�[
    float rotationAngle;        // ��]�p�x
    float attackDirection;      // �U������

    // �m�R�M���̃p�����[�^
    static constexpr float SPIN_SPEED = 0.3f;          // ��]���x
    static constexpr float MOVE_SPEED = 3.0f;          // �ړ����x
    static constexpr float ATTACK_SPEED = 6.0f;        // �U�����x
    static constexpr float REST_DURATION = 1.5f;       // �x�e����
    static constexpr float SPIN_DURATION = 2.0f;       // ��]����
    static constexpr float MOVE_DURATION = 3.0f;       // �ړ�����
    static constexpr float ATTACK_DURATION = 1.0f;     // �U������
    static constexpr float ATTACK_DISTANCE = 100.0f;   // �U������

    // �m�R�M���ŗL�̍s��
    void UpdateSawMovement();
    void UpdateSawAnimation();
    void StartSpinning();
    void StartMoving();
    void StartAttacking();
    void HandlePlayerDetectionSaw(Player* player);

    // �m�R�M���ŗL�̃X�v���C�g�擾
    int GetSawSprite();
    void TakeDamage(int damage);
};