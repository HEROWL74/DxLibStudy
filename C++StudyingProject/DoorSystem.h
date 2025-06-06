#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <string>

class DoorSystem {
public:
    // �h�A�̏��
    enum DoorState {
        DOOR_CLOSED,        // �܂��Ă���
        DOOR_OPENING,       // �J���Ă���r��
        DOOR_OPEN,          // ���S�ɊJ���Ă���
        DOOR_PLAYER_ENTERING // �v���C���[�������Ă���r��
    };

    DoorSystem();
    ~DoorSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // �h�A�Ǘ�
    void SetDoor(float x, float y);
    void ClearDoor();
    void OpenDoor(); // �S�[�����Ƀh�A���J��
    bool IsPlayerEntering() const { return doorState == DOOR_PLAYER_ENTERING; }
    bool IsPlayerFullyEntered() const;
    bool IsDoorExists() const { return doorExists; }

    // �X�e�[�W�ɉ������h�A�z�u
    void PlaceDoorForStage(int stageIndex, StageManager* stageManager);

private:
    // �e�N�X�`���n���h��
    struct DoorTextures {
        int closedBottom;     // door_closed.png
        int closedTop;        // door_closed_top.png
        int openBottom;       // door_open.png
        int openTop;          // door_open_top.png
    } doorTextures;

    // �h�A���
    float doorX, doorY;         // �h�A�ʒu
    DoorState doorState;        // �h�A�̏��
    bool doorExists;            // �h�A�����݂��邩

    // �A�j���[�V����
    float openingProgress;      // �J���i�s�x (0.0f�`1.0f)
    float enteringProgress;     // �v���C���[�i���i�s�x (0.0f�`1.0f)
    float animationTimer;       // �A�j���[�V�����^�C�}�[

    // �v���C���[�i�����o
    float playerStartX;         // �v���C���[�̊J�nX���W
    float playerTargetX;        // �v���C���[�̖ڕWX���W
    float playerWalkSpeed;      // �h�A�ւ̕��s���x

    // �萔
    static constexpr float DOOR_WIDTH = 64.0f;          // �h�A�̕�
    static constexpr float DOOR_HEIGHT = 128.0f;        // �h�A�̍����i2�^�C�����j
    static constexpr float DETECTION_WIDTH = 80.0f;     // �v���C���[���o��
    static constexpr float OPENING_DURATION = 1.5f;     // �h�A�J������
    static constexpr float ENTERING_DURATION = 2.0f;    // �v���C���[�i������
    static constexpr float PLAYER_WALK_SPEED = 60.0f;   // �h�A�ւ̕��s���x

    // �w���p�[�֐�
    void LoadTextures();
    void UpdateDoorAnimation();
    void UpdatePlayerEntering(Player* player);
    bool CheckPlayerNearDoor(Player* player);
    void StartPlayerEntering(Player* player);
    void DrawDoorSprites(float cameraX);
    void DrawDoorEffects(float cameraX);
    float GetDistance(float x1, float y1, float x2, float y2);

    // �X�e�[�W�ɉ������n�ʃ��x������
    float FindGroundLevel(float x, StageManager* stageManager);
    float FindGroundLevelAccurate(float x, StageManager* stageManager); // **�V�ǉ��F��萳�m�Ȓn�ʌ���**
};