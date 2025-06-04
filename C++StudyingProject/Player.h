#pragma once
#include "DxLib.h"
#include "StageManager.h"
#include <string>

class Player {
public:
    // �v���C���[���
    enum State {
        IDLE,
        WALKING,
        JUMPING,
        FALLING,
        DUCKING
    };

    // �v���C���[�T�C�Y�萔�i128x128�ɑΉ��j
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

private:
    // �L�����N�^�[�X�v���C�g�\����
    struct CharacterSprites {
        int front, idle, walk_a, walk_b, jump, duck, hit, climb_a, climb_b;
    };

    // �����萔�i128x128�v���C���[�p�ɍœK���j
    static constexpr float GRAVITY = 0.8f;             // �d�͂��������߂ɒ���
    static constexpr float JUMP_POWER = -20.0f;        // �W�����v�͂�128x128�v���C���[�ɍ��킹�Ē���
    static constexpr float MOVE_SPEED = 8.0f;
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float MAX_FALL_SPEED = 16.0f;     // �ő嗎�����x

    // ���ǁF�C�[�W���O�֐��̒萔
    static constexpr float FRICTION = 0.85f;           // ���C�W���i���ړ��̌����j
    static constexpr float ACCELERATION = 1.2f;        // �����x
    static constexpr float MAX_HORIZONTAL_SPEED = 8.0f; // �ő剡�ړ����x

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

    // ===== ��{�I�ȃw���p�[�֐� =====
    void UpdatePhysics(StageManager* stageManager);
    void UpdateAnimation();
    void HandleCollisions(StageManager* stageManager);
    int GetCurrentSprite();
    void LoadCharacterSprites(int characterIndex);
    std::string GetCharacterColorName(int index);

    // ===== �ڍׂȏՓ˔���V�X�e�� =====

    // ���C���Փ˔���֐�
    bool CheckXCollision(float newX, float currentY, float width, float height, StageManager* stageManager);
    void HandleDownwardMovement(float newY, float width, float height, StageManager* stageManager);
    void HandleUpwardMovement(float newY, float width, float height, StageManager* stageManager);

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


   // ��{�I�Ȓn�ʌ��o�i�����j
    float FindNearestGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // �����x�Ȓn�ʌ��o�i�ڍׁj
    float FindPreciseGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // �K���I�n�ʌ��o�i���G�Ȓn�`�Ή��j
    float FindAdaptiveGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // �œK�Ȓn�ʌ��o�i�󋵂ɉ����čœK�ȕ��@��I���j
    float FindOptimalGroundForShadow(float playerX, float playerY, StageManager* stageManager);

    // �e�`��̒萔
    static constexpr float MAX_SHADOW_DISTANCE = 200.0f; // �e���\�������ő勗��
    static constexpr float BASE_SHADOW_SIZE_X = 40.0f;   // ��{�e�T�C�Y�i���j
    static constexpr float BASE_SHADOW_SIZE_Y = 16.0f;   // ��{�e�T�C�Y�i�����j
    static constexpr int BASE_SHADOW_ALPHA = 150;        // ��{�e�̓����x
};