#include "PurpleStage.h"

PurpleStage::PurpleStage() {
    terrainType = "purple";
}

PurpleStage::~PurpleStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void PurpleStage::Initialize() {
    // �e�N�X�`���ǂݍ���
    LoadTerrainTextures();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void PurpleStage::CreateMainTerrain() {
    // ���@�̐��E���̕��V����n�`
    int groundHeight = 2;

    // ���V���铇�X
    CreatePlatform(0, 13, 10, groundHeight + 1);   // �X�^�[�g�n�_
    CreatePlatform(15, 11, 8, groundHeight);     // �ŏ��̕��V��
    CreatePlatform(28, 9, 6, groundHeight);      // �������V��
    CreatePlatform(38, 12, 10, groundHeight + 1);  // �傫�ȕ��V��
    CreatePlatform(52, 7, 8, groundHeight + 2);    // �Ō�̍�����
}

void PurpleStage::CreatePlatforms() {
    // ���@�I�ȃv���b�g�t�H�[��
    CreatePlatform(8, 9, 2, 1);    // �����Ȗ��@�̑���
    CreatePlatform(12, 7, 1, 1);   // �P�Ƃ̖��@�u���b�N
    CreatePlatform(20, 8, 3, 1);   // �����炢�̑���
    CreatePlatform(25, 6, 1, 1);   // �������@�u���b�N

    // ���@�̊K�i
    CreatePlatform(32, 10, 1, 1);  // 1�i��
    CreatePlatform(33, 9, 1, 1);   // 2�i��
    CreatePlatform(34, 8, 1, 1);   // 3�i��
    CreatePlatform(35, 7, 1, 1);   // 4�i��

    // ���V���閂�@�̗�
    CreatePlatform(42, 8, 1, 1);
    CreatePlatform(45, 6, 1, 1);
    CreatePlatform(48, 4, 1, 1);
    CreatePlatform(55, 3, 2, 1);   // �S�[���t�߂̑���
}