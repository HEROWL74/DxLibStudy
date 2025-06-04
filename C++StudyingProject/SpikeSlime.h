#pragma once
#include "EnemyBase.h"

class SpikeSlime : public EnemyBase {
public:
    SpikeSlime(float startX, float startY);
    ~SpikeSlime() override = default;

    // �������z�֐��̎���
    void Initialize() override;
    void UpdateBehavior(Player* player, StageManager* stageManager) override;
    void LoadSprites() override;

    // �I�[�o�[���C�h
    void OnPlayerCollision(Player* player) override;
    int GetCurrentSprite() override;

    // **�V�@�\: �O������g�Q�̏�Ԃ��m�F**
    bool AreSpikesOut() const { return isSpikesOut; }

    // **�V�@�\: SpikeSlime�̏�Ԃ��擾**
    enum SpikeSlimeState {
        SPIKE_DORMANT,      // �x����
        SPIKE_ALERT,        // �x����
        SPIKE_CHARGING,     // �`���[�W��
        SPIKE_ATTACKING,    // �U����
        SPIKE_COOLDOWN,     // �N�[���_�E��
        SPIKE_STUNNED       // �X�^�����
    };

    SpikeSlimeState GetSpikeState() const { return spikeState; }

private:
    // �g�Q�X���C���ŗL�̏��
    SpikeSlimeState spikeState;
    float chargeTimer;          // �`���[�W�^�C�}�[
    float alertTimer;           // �x���^�C�}�[
    float cooldownTimer;        // �N�[���_�E���^�C�}�[
    float stunTimer;            // �X�^���^�C�}�[
    float pulsePhase;           // �������ʗp
    bool isSpikesOut;           // �g�Q���o�Ă��邩

    // �g�Q�X���C���ŗL�̃p�����[�^
    static constexpr float CHARGE_DURATION = 1.0f;     // �`���[�W����
    static constexpr float ATTACK_DURATION = 0.5f;     // �U������
    static constexpr float COOLDOWN_DURATION = 2.0f;   // �N�[���_�E������
    static constexpr float STUN_DURATION = 2.0f;       // �X�^������
    static constexpr float ALERT_DURATION = 0.8f;      // �x������
    static constexpr float PULSE_SPEED = 0.15f;        // �������x
    static constexpr float CHARGE_SPEED = 4.0f;        // �`���[�W�U���̑��x
    static constexpr float SPIKE_DAMAGE = 15;          // �g�Q�_���[�W

    // �g�Q�X���C���ŗL�̍s��
    void UpdateSpikeSlimeMovement();
    void UpdateSpikeSlimeAnimation();
    void StartChargeAttack(Player* player);
    void ExecuteChargeAttack(Player* player);
    void EnterCooldownState();
    void CheckPlayerInAlertRange(Player* player);
    void HandleStunState();

    // ���o����
    void DrawSpikeEffect(float cameraX);

    // �g�Q�X���C���ŗL�̃X�v���C�g�擾
    int GetSpikeSlimeSprite();
};