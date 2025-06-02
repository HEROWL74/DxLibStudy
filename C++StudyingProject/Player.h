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

    // �v���C���[�T�C�Y�萔
    static const int PLAYER_WIDTH = 128;
    static const int PLAYER_HEIGHT = 128;

    Player();
    ~Player();

    void Initialize(int characterIndex);
    void Update(StageManager* stageManager);
    void Draw(float cameraX);
    void DrawShadow(float cameraX, StageManager* stageManager);
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

    // �����萔
    static constexpr float GRAVITY = 0.8f;
    static constexpr float JUMP_POWER = -18.0f;
    static constexpr float MOVE_SPEED = 8.0f;
    static constexpr float WALK_ANIM_SPEED = 0.15f;
    static constexpr float MAX_FALL_SPEED = 15.0f;

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

    // ===== �����ȏՓ˔���V�X�e�� =====

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

    // ===== ����݊����̂��߂̋��֐� =====
    bool CheckCollision(float newX, float newY, StageManager* stageManager);
    float GetGroundY(float checkX, StageManager* stageManager);
    bool CheckCollisionRect(float x, float y, float width, float height, StageManager* stageManager);
    int FindGroundTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);
    int FindCeilingTileY(float playerX, float playerY, float playerWidth, StageManager* stageManager);
};