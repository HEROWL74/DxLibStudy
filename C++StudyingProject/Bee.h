#pragma once
#include "EnemyBase.h"

class Bee : public EnemyBase {
public:
    Bee(float startX, float startY);
    ~Bee() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

private:
    // �~�c�o�`�ŗL�̏��
    enum BeeState {
        BEE_RESTING,        // �x�e��
        BEE_HOVERING,       // �z�o�����O
        BEE_ATTACKING,      // �U�����i�}�~���j
        BEE_RETURNING       // ���̈ʒu�ɖ߂�
    };

    BeeState beeState;
    float hoverTimer;           // �z�o�����O�^�C�}�[
    float restTimer;            // �x�e�^�C�}�[
    float attackTimer;          // �U���^�C�}�[
    float hoverPhase;           // �z�o�����O�ʑ�
    float originalY;            // ����Y���W
    float targetY;              // �ڕWY���W
    float attackStartY;         // �U���J�nY���W

    // �~�c�o�`�̃p�����[�^
    static constexpr float HOVER_AMPLITUDE = 20.0f;    // �z�o�����O�U��
    static constexpr float HOVER_SPEED = 0.05f;        // �z�o�����O���x
    static constexpr float ATTACK_SPEED = 8.0f;        // �U�����x
    static constexpr float RETURN_SPEED = 3.0f;        // ���A���x
    static constexpr float REST_DURATION = 2.0f;       // �x�e����
    static constexpr float HOVER_DURATION = 3.0f;      // �z�o�����O����
    static constexpr float ATTACK_RANGE_Y = 150.0f;    // �c�����U���͈�

    // �~�c�o�`�ŗL�̍s��
    void UpdateBeeMovement();
    void UpdateBeeAnimation();
    void StartHovering();
    void StartAttack();
    void StartReturning();
    void HandlePlayerDetectionBee(Player* player);

    // �~�c�o�`�ŗL�̃X�v���C�g�擾
    int GetBeeSprite();
    void UpdateAnimation();
};