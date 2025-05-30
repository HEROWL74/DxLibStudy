#pragma once
#include "DxLib.h"
#include <string>

class GameScene
{
public:
    GameScene();
    ~GameScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }

private:
    // ��ʃT�C�Y
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // �L�����N�^�[�F��
    enum CharacterColor {
        BEIGE = 0,
        GREEN = 1,
        PINK = 2,
        PURPLE = 3,
        YELLOW = 4
    };

    // �L�����N�^�[���
    enum CharacterState {
        IDLE,
        WALKING,
        JUMPING,
        DUCKING
    };

    // �L�����N�^�[�X�v���C�g�\����
    struct CharacterSprites {
        int front;
        int idle;
        int walk_a;
        int walk_b;
        int jump;
        int duck;
        int hit;
        int climb_a;
        int climb_b;
    };

    // �e�N�X�`���n���h��
    int backgroundHandle;
    CharacterSprites characterSprites; // �S�X�v���C�g
    int fontHandle;

    // �L�����N�^�[���
    int selectedCharacterIndex;
    std::string characterColorName;
    std::string characterName;

    // �v���C���[���
    float playerX, playerY;
    float velocityX, velocityY;
    float groundY;               // �n�ʂ�Y���W
    bool facingRight;           // �����Ă�������itrue=�E�Afalse=���j
    CharacterState currentState;
    bool onGround;              // �n�ʂɂ��邩�ǂ���

    // �A�j���[�V����
    float animationTimer;       // �A�j���[�V�����p�^�C�}�[
    bool walkAnimFrame;         // walk_a(false) or walk_b(true)
    float bobPhase;            // �A�C�h�����̏㉺�̗h�����

    // �����萔
    static constexpr float GRAVITY = 0.8f;
    static constexpr float JUMP_POWER = -18.0f;
    static constexpr float MOVE_SPEED = 5.0f;
    static constexpr float WALK_ANIM_SPEED = 0.2f; // �A�j���[�V�������x

    // ��ԊǗ�
    bool exitRequested;

    // �L�[����
    bool leftPressed, rightPressed, downPressed, spacePressed;
    bool leftPressedPrev, rightPressedPrev, downPressedPrev, spacePressedPrev;
    bool escPressed, escPressedPrev;

    // �w���p�[�֐�
    std::string GetCharacterColorName(int index);
    std::string GetCharacterDisplayName(int index);
    void LoadAllCharacterSprites(int characterIndex);
    void UpdateInput();
    void UpdatePhysics();
    void UpdateAnimation();
    int GetCurrentSprite();
    void DrawCharacter();

    // ���[�e�B���e�B
    float Lerp(float a, float b, float t);
};