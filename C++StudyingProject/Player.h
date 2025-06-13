#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include <string>

class BlockSystem;

class Player {
public:
    // �v���C���[���
    enum State {
        IDLE,
        WALKING,
        JUMPING,
        FALLING,
        DUCKING,
        SLIDING,
        HIT
    };

    // �v���C���[�T�C�Y�萔
    static const int PLAYER_WIDTH = 128;
    static const int PLAYER_HEIGHT = 128;

    Player();
    ~Player();

    void Initialize(int characterIndex);
    void Update(StageManager* stageManager);
    void Draw(float cameraX);
    void DrawShadow(float cameraX, StageManager* stageManager);
    void DrawShadowDebugInfo(float cameraX, int shadowX, int shadowY, float distanceToGround, float normalizedDistance);
    void DrawDebugInfo(float cameraX);

    // �Q�b�^�[
    float GetX() const { return x; }
    float GetY() const { return y; }
    bool IsFacingRight() const { return facingRight; }
    State GetState() const { return currentState; }
    float GetVelocityX() const { return velocityX; }
    float GetVelocityY() const { return velocityY; }
    bool IsOnGround() const { return onGround; }

    // �Z�b�^�[
    void SetPosition(float newX, float newY);
    void ResetPosition();

    // �V�K�ǉ�: �u���b�N�V�X�e���g���p�̊֐�
    void SetOnGround(bool ground) { onGround = ground; }
    void SetState(State newState) { currentState = newState; }

    // ���݂�����
    void ApplyStompBounce(float bounceVelocity = -8.0f);

    // �_���[�W�V�X�e��
    void TakeDamage(int damage, float knockbackDirection = 0.0f);
    void ApplyGravityOnly(StageManager* stageManager);
    void HandleGroundCollisionOnly(StageManager* stageManager);
    void UpdateAnimationOnly();
    bool IsInvulnerable() const { return invulnerabilityTimer > 0.0f; }
    float GetInvulnerabilityTimer() const { return invulnerabilityTimer; }

    // �f�o�b�O�p
    bool IsStomping() const { return wasStomping; }

    // �������s�p
    void SetFacingRight(bool facing) { facingRight = facing; }
    void SetAutoWalking(bool autoWalk) { isAutoWalking = autoWalk; }
    bool IsAutoWalking() const { return isAutoWalking; }
    void UpdateAutoWalkPhysics(StageManager* stageManager);

    // �X���C�f�B���O�֘A
    bool IsSliding() const { return currentState == SLIDING; }
    float GetSlidingProgress() const { return slidingTimer / SLIDING_DURATION; }

    // ���x�ݒ�i�u���b�N�Փˌ�p�j
    void SetVelocityX(float velX) { velocityX = velX; }
    void SetVelocityY(float velY) { velocityY = velY; }

    // GameScene����Ăяo����镡���Փˏ���
    void HandleCollisionsWithBlocks(StageManager* stageManager, class BlockSystem* blockSystem);
    void UpdatePhysics(StageManager* stageManager);
    void UpdateAnimation();
    void HandleCollisions(StageManager* stageManager);
private:
    // �L�����N�^�[�X�v���C�g�\����
    struct CharacterSprites {
        int front, idle, walk_a, walk_b, jump, duck, hit, climb_a, climb_b;
    };

    // �����ǂ��ꂽ�����萔 - �ӂ���ƃW�����v
    static constexpr float GRAVITY = 0.4f;              // �d�͂��キ�i�ӂ���ƌ��ʁj
    static constexpr float JUMP_POWER = -14.0f;         // �W�����v�͂�����
    static constexpr float MOVE_SPEED = 4.0f;
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float MAX_FALL_SPEED = 12.0f;      // �������x�����������

    // ���ǉ�: �W�����v�֘A�̏ڍג萔
    static constexpr float JUMP_RELEASE_MULTIPLIER = 0.5f;  // �W�����v�{�^�����������̌�����
    static constexpr float APEX_GRAVITY_REDUCTION = 0.6f;   // �W�����v���_�t�߂ł̏d�͌y��
    static constexpr float APEX_THRESHOLD = 2.0f;           // ���_����̑��x臒l

    // �i�K�I�����x�V�X�e���̒萔
    static constexpr float FRICTION = 0.85f;
    static constexpr float ACCELERATION = 0.8f;
    static constexpr float MAX_HORIZONTAL_SPEED = 10.0f;

    // �X���C�f�B���O�V�X�e���萔
    static constexpr float SLIDING_DURATION = 1.2f;
    static constexpr float SLIDING_INITIAL_SPEED = 12.0f;
    static constexpr float SLIDING_DECELERATION = 0.92f;
    static constexpr float SLIDING_MIN_SPEED = 1.0f;
    static constexpr float SLIDING_HEIGHT_REDUCTION = 0.6f;

    // �_���[�W�V�X�e���萔
    static constexpr float HIT_DURATION = 0.8f;
    static constexpr float INVULNERABILITY_DURATION = 2.0f;
    static constexpr float KNOCKBACK_FORCE = 6.0f;
    static constexpr float KNOCKBACK_VERTICAL = -4.0f;

    // �v���C���[���
    float x, y;
    float velocityX, velocityY;
    bool facingRight;
    State currentState;
    bool onGround;

    // �A�j���[�V����
    float animationTimer;
    bool walkAnimFrame;
    float bobPhase;

    // �L�����N�^�[���
    int characterIndex;
    CharacterSprites sprites;
    std::string characterColorName;

    // ���݂���ԊǗ��p
    bool wasStomping;
    float stompCooldown;
    static constexpr float STOMP_COOLDOWN_DURATION = 0.2f;

    // �_���[�W�V�X�e���ϐ�
    float hitTimer;
    float invulnerabilityTimer;
    float knockbackDecay;

    // �X���C�f�B���O�V�X�e���ϐ�
    float slidingTimer;
    float slidingSpeed;
    bool wasRunningWhenSlideStarted;

    // �������s�t���O
    bool isAutoWalking = false;

    // ===== ��{�I�ȃw���p�[�֐� =====
    void HandleDownwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager);
    bool IsOnStageGround(float playerX, float playerY, float width, float height, StageManager* stageManager);
    int GetCurrentSprite();
    void LoadCharacterSprites(int characterIndex);
    std::string GetCharacterColorName(int index);

    // �X���C�f�B���O�V�X�e���֐�
    void StartSliding();
    void UpdateSliding();
    void EndSliding();
    float GetSlidingCollisionHeight() const;
    void DrawSlidingEffect(float cameraX);

    // �_���[�W�V�X�e���֐�
    void UpdateDamageState();
    void UpdateInvulnerability();

    // ===== �ڍׂȏՓ˔���V�X�e�� =====

    void HandleUpwardMovementStageOnly(float newY, float width, float height, StageManager* stageManager);

    // ���C���Փ˔���֐�
    bool CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager);
    void HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager);
    void HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager);

    void SetLandingState();

    // �ڍׂȏՓ˔���
    bool CheckPointCollision(float centerX, float centerY, float width, float height, StageManager* stageManager);
    bool IsOnGround(float playerX, float playerY, float width, float height, StageManager* stageManager);

    // �ʒu�����@�\
    float AdjustXPosition(float currentX, bool movingRight, float width, StageManager* stageManager);
    float FindPreciseGroundY(float playerX, float playerY, float width, StageManager* stageManager);
    float FindPreciseCeilingY(float playerX, float playerY, float width, StageManager* stageManager);

    // ===== ��ʌ݊����̂��߂̋��֐� =====
    bool CheckCollision(float newX, float newY, StageManager* stageManager);
    float GetGroundY(float checkX, StageManager* stageManager);
    bool CheckCollisionRect(float x, float y, float width, float height, StageManager* stageManager);
    int FindGroundTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);
    int FindCeilingTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);

    // �e�`��p�̒n�ʌ����֐�
    float FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager);
    float FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // �e�`��̒萔
    static constexpr float MAX_SHADOW_DISTANCE = 200.0f;
    static constexpr float BASE_SHADOW_SIZE_X = 40.0f;
    static constexpr float BASE_SHADOW_SIZE_Y = 16.0f;
    static constexpr int BASE_SHADOW_ALPHA = 150;

};