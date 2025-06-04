#pragma once
#include "EnemyBase.h"

class NormalSlime : public EnemyBase {
public:
    NormalSlime(float startX, float startY);
    ~NormalSlime() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    void Draw(float cameraX) override; // **�V�ǉ��F�ׂ�A�j���[�V�����p**
    int GetCurrentSprite() override;

private:
    // �m�[�}���X���C���ŗL�̏��
    enum SlimeState {
        SLIME_RESTING,      // �x�e��
        SLIME_ACTIVE,       // �A�N�e�B�u
        SLIME_JUMPING,      // �W�����v��
        SLIME_FLATTENED     // �ׂꂽ���
    };

    SlimeState slimeState;
    float jumpTimer;            // �W�����v�^�C�}�[
    float restTimer;            // �x�e�^�C�}�[
    float flattenedTimer;       // �ׂ��ԃ^�C�}�[
    float bobPhase;             // �㉺�h��p

    // **�V�ǉ��F�ׂ�A�j���[�V�����p�ϐ�**
    float flattenScale;         // �ׂꂽ���̃X�P�[��
    float flattenProgress;      // �ׂ�A�j���[�V�����̐i�s�x

    // �X���C���ŗL�̃p�����[�^
    static constexpr float JUMP_POWER = -12.0f;
    static constexpr float JUMP_INTERVAL = 2.0f;    // �W�����v�Ԋu
    static constexpr float REST_DURATION = 1.5f;    // �x�e����
    static constexpr float FLATTENED_DISPLAY_DURATION = 1.5f; // **�ׂ�\�����ԁi�����j**
    static constexpr float BOB_SPEED = 0.08f;        // �h�ꑬ�x
    static constexpr float BOB_AMPLITUDE = 3.0f;    // �h�ꕝ

    // �X���C���ŗL�̍s��
    void UpdateSlimeMovement();
    void UpdateSlimeAnimation();
    void StartJump();
    void HandleJumpMovement(Player* player);
    void CheckFlattenedRecovery();

    // **�V�ǉ��F�ׂ�A�j���[�V�����X�V**
    void UpdateFlattenedAnimation();

    // �X���C���ŗL�̃X�v���C�g�擾
    int GetSlimeSprite();
};