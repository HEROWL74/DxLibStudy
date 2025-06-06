#include "PurpleStage.h"

PurpleStage::PurpleStage() {
    terrainType = "purple";
}

PurpleStage::~PurpleStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void PurpleStage::Initialize() {
    // ���C���e�N�X�`���ǂݍ��݁i�n�ʗp�j
    LoadTerrainTextures();

    // **�v���b�g�t�H�[����p�e�N�X�`���ǂݍ��݁iterrain_purple_block.png�j**
    LoadPlatformTexture();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void PurpleStage::CreateMainTerrain() {
    // ���@�ِ̈��E���̕��V����n�`
    int groundHeight = 2;

    // ���V���铇�X - ���n��₷������
    CreatePlatform(0, 13, 12, groundHeight + 1);   // �X�^�[�g�n�_
    CreatePlatform(15, 11, 10, groundHeight + 1);  // �ŏ��̕��V��
    CreatePlatform(28, 9, 8, groundHeight + 1);    // �������V��
    CreatePlatform(39, 12, 12, groundHeight + 1);  // �傫�ȕ��V��
    CreatePlatform(54, 8, 10, groundHeight + 2);   // �ō��̍�����
    CreatePlatform(67, 10, 12, groundHeight + 1);  // ���Ԃ̓�
    CreatePlatform(82, 12, 15, groundHeight + 1);  // �傫�ȓ�
    CreatePlatform(100, 9, 20, groundHeight + 1);  // �S�[���O�̓�
}

void PurpleStage::CreatePlatforms() {
    // **���@�I�ȃv���b�g�t�H�[�� - terrain_purple_block.png�e�N�X�`���g�p**
    // �W�����v���₷���z�u
    CreatePlatformBlock(8, 9, 3, 1);    // �����Ȗ��@�̑���
    CreatePlatformBlock(12, 7, 2, 1);   // �P�Ƃ̖��@�u���b�N
    CreatePlatformBlock(20, 8, 4, 1);   // �����炢�̑���
    CreatePlatformBlock(26, 6, 2, 1);   // �������@�u���b�N

    // ���@�̊K�i - ���o��₷��
    CreatePlatformBlock(33, 10, 2, 1);  // 1�i��
    CreatePlatformBlock(35, 9, 2, 1);   // 2�i��
    CreatePlatformBlock(37, 8, 2, 1);   // 3�i��
    CreatePlatformBlock(39, 7, 2, 1);   // 4�i��

    // ���V���閂�@�̗� - �W�����v���₷���Ԋu
    CreatePlatformBlock(45, 8, 2, 1);
    CreatePlatformBlock(48, 6, 2, 1);
    CreatePlatformBlock(51, 4, 2, 1);
    CreatePlatformBlock(58, 6, 3, 1);   // �S�[���t�߂̑���

    // ���@�̋�
    CreatePlatformBlock(72, 8, 6, 1);   // �������@�̋�
    CreatePlatformBlock(85, 6, 4, 1);   // �������@�̑���
    CreatePlatformBlock(92, 9, 3, 1);   // ���Ԃ̑���
    CreatePlatformBlock(105, 7, 4, 1);  // �S�[���O�̖��@����

    // ���@�̓�
    CreatePlatformBlock(24, 11, 1, 1);
    CreatePlatformBlock(43, 5, 1, 1);
    CreatePlatformBlock(62, 8, 1, 1);
    CreatePlatformBlock(88, 4, 1, 1);
    CreatePlatformBlock(110, 10, 1, 1);
}