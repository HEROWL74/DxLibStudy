#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include "Player.h"
#include <string>
#include <memory>

class EnemyBase {
public:
    // �G�̏��
    enum EnemyState {
        IDLE,           // �ҋ@
        WALKING,        // ���s
        ATTACKING,      // �U��
        DAMAGED,        // �_���[�W
        DYING,          // ���S�A�j���[�V����
        DEAD            // ���S����
    };

    // �G�̃^�C�v - ���C��: �V�����G�^�C�v��ǉ�
    enum EnemyType {
        NORMAL_SLIME,
        SPIKE_SLIME,
        BEE,            // ���ǉ�
        FLY,            // ���ǉ�  
        LADYBUG,        // ���ǉ�
        SAW,            // ���ǉ�
        SLIME_BLOCK,    // ���ǉ�
        // �����I�ɑ��̓G���ǉ��\
        GOBLIN,
        SKELETON
    };

    // �G�̃T�C�Y�萔
    static const int ENEMY_WIDTH = 64;
    static const int ENEMY_HEIGHT = 64;

    EnemyBase(float startX, float startY, EnemyType type);
    virtual ~EnemyBase();

    // �������z�֐��i�p����Ŏ����K�{�j
    virtual void Initialize() = 0;
    virtual void UpdateBehavior(Player* player, StageManager* stageManager) = 0;
    virtual void LoadSprites() = 0;

    // ���ʏ���
    virtual void Update(Player* player, StageManager* stageManager);
    virtual void Draw(float cameraX);
    virtual void DrawDebugInfo(float cameraX);

    // �����蔻��
    virtual bool CheckPlayerCollision(Player* player);
    virtual void OnPlayerCollision(Player* player);

    // �Q�b�^�[
    float GetX() const { return x; }
    float GetY() const { return y; }
    bool IsActive() const { return active; }
    bool IsDead() const { return currentState == DEAD; }
    EnemyState GetState() const { return currentState; }
    EnemyType GetType() const { return enemyType; }

    // �Z�b�^�[
    void SetPosition(float newX, float newY);
    void SetActive(bool isActive) { active = isActive; }
    void TakeDamage(int damage);

protected:
    // ��{�p�����[�^
    float x, y;                     // �ʒu
    float velocityX, velocityY;     // ���x
    bool facingRight;               // ����
    EnemyState currentState;        // ���݂̏��
    EnemyType enemyType;           // �G�̃^�C�v
    bool active;                   // �A�N�e�B�u�t���O
    bool onGround;                 // �n�ʐڐG�t���O

    // �X�e�[�^�X
    int health;                    // �̗�
    int maxHealth;                 // �ő�̗�
    float moveSpeed;               // �ړ����x
    float detectionRange;          // �v���C���[���o�͈�
    float attackRange;             // �U���͈�
    int attackPower;               // �U����

    // �A�j���[�V����
    float animationTimer;          // �A�j���[�V�����^�C�}�[
    bool animationFrame;           // �A�j���[�V�����t���[��
    float stateTimer;              // ��ԃ^�C�}�[

    // �ړ��֘A
    float patrolStartX;            // �p�g���[���J�n�ʒu
    float patrolEndX;              // �p�g���[���I���ʒu
    float patrolDistance;          // �p�g���[������
    bool patrolDirection;          // �p�g���[�������itrue=�E�Afalse=���j

    // �X�v���C�g - ���C��: jump�����o�[��ǉ�
    struct EnemySprites {
        int idle;
        int walk_a;
        int walk_b;
        int attack;
        int damaged;
        int dead;
        int flat;      // �ׂꂽ��ԁi�X���C���p�j
        int jump;      // ���ǉ�: �W�����v/��s��ԗp
    } sprites;

    // �����萔
    static constexpr float GRAVITY = 0.8f;
    static constexpr float MAX_FALL_SPEED = 15.0f;
    static constexpr float COLLISION_WIDTH = 48.0f;
    static constexpr float COLLISION_HEIGHT = 56.0f;

    // ���ʏ����֐�
    virtual void UpdatePhysics(StageManager* stageManager);
    virtual void UpdateAnimation();
    virtual void HandleCollisions(StageManager* stageManager);
    virtual void UpdatePatrol();
    virtual void HandlePlayerDetection(Player* player);

    // �����蔻��w���p�[
    bool CheckGroundCollision(float checkX, float checkY, StageManager* stageManager);
    bool CheckWallCollision(float checkX, float checkY, StageManager* stageManager);
    float FindGroundY(float checkX, StageManager* stageManager);
    float GetDistanceToPlayer(Player* player);

    // �X�v���C�g�֘A
    virtual int GetCurrentSprite();
    void DrawEnemySprite(float cameraX, int spriteHandle);

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
    bool IsPlayerInRange(Player* player, float range);
    void FacePlayer(Player* player);
};