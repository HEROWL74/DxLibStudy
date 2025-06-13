#pragma once
#include "EnemyBase.h"

class SlimeBlock : public EnemyBase {
public:
    SlimeBlock(float startX, float startY);
    ~SlimeBlock() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    void Draw(float cameraX) override;
    void TakeDamage(int damage);
    int GetCurrentSprite() override;

private:
    // �u���b�N�X���C���ŗL�̏��
    enum SlimeBlockState {
        BLOCK_RESTING,      // �x�e��
        BLOCK_ACTIVE,       // �A�N�e�B�u
        BLOCK_JUMPING,      // �W�����v��
        BLOCK_HARDENED,     // �d�����
        BLOCK_BOUNCING      // �o�E���X��
    };

    SlimeBlockState blockState;
    float restTimer;            // �x�e�^�C�}�[
    float jumpTimer;            // �W�����v�^�C�}�[
    float hardenTimer;          // �d���^�C�}�[
    float bounceTimer;          // �o�E���X�^�C�}�[
    float bobPhase;             // �㉺�h��ʑ�
    bool isHardened;            // �d���t���O

    // �u���b�N�X���C���̃p�����[�^
    static constexpr float JUMP_POWER = -14.0f;        // �W�����v��
    static constexpr float JUMP_INTERVAL = 3.0f;       // �W�����v�Ԋu
    static constexpr float REST_DURATION = 2.0f;       // �x�e����
    static constexpr float HARDEN_DURATION = 1.5f;     // �d������
    static constexpr float BOUNCE_FORCE = -15.0f;      // �o�E���X��
    static constexpr float BOB_SPEED = 0.06f;          // �h�ꑬ�x
    static constexpr float BOB_AMPLITUDE = 2.0f;       // �h��U��

    // �u���b�N�X���C���ŗL�̍s��
    void UpdateSlimeBlockMovement();
    void UpdateSlimeBlockAnimation();
    void StartJumping();
    void StartHardening();
    void HandlePlayerBounce(Player* player);

    // �u���b�N�X���C���ŗL�̃X�v���C�g�擾
    int GetSlimeBlockSprite();
};