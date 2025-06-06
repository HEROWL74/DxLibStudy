#include "SandStage.h"

SandStage::SandStage() {
    terrainType = "sand";
}

SandStage::~SandStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void SandStage::Initialize() {
    // ���C���e�N�X�`���ǂݍ��݁i�n�ʗp�j
    LoadTerrainTextures();

    // **�v���b�g�t�H�[����p�e�N�X�`���ǂݍ��݁iterrain_sand_block.png�j**
    LoadPlatformTexture();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void SandStage::CreateMainTerrain() {
    // �����̋N���̂���n�`
    int groundHeight = 3;

    // �N���̂���n�� - ���o��₷������
    CreatePlatform(0, 13, 15, groundHeight);     // �����n��
    CreatePlatform(15, 11, 10, groundHeight + 2); // ���������n��
    CreatePlatform(25, 14, 12, groundHeight);    // �܂������n��
    CreatePlatform(37, 10, 12, groundHeight + 3); // �����u
    CreatePlatform(49, 12, 15, groundHeight + 1); // �����x�̍���
    CreatePlatform(64, 9, 15, groundHeight + 4);  // ����ɍ����u
    CreatePlatform(79, 11, 15, groundHeight + 2); // �����
    CreatePlatform(94, 13, 26, groundHeight);     // �Ō�̕��n
}

void SandStage::CreatePlatforms() {
    // **�����̃I�A�V�X���̃v���b�g�t�H�[���iterrain_sand_block.png�e�N�X�`���g�p�j**
    CreatePlatformBlock(5, 9, 4, 1);   // �����ȓ�
    CreatePlatformBlock(12, 7, 3, 1);  // �W�����v��
    CreatePlatformBlock(20, 8, 5, 1);  // �����炢�̓�
    CreatePlatformBlock(30, 6, 4, 1);  // ��������

    // �s���~�b�h���̍\�� - ���o��₷��
    CreatePlatformBlock(42, 8, 1, 1);  // ����
    CreatePlatformBlock(41, 9, 3, 1);  // 2�i��
    CreatePlatformBlock(40, 10, 5, 1); // 1�i�ځi���ۂ�3�i�ځj
    CreatePlatformBlock(39, 11, 7, 1); // �

    // ���u��\������v���b�g�t�H�[�� - �W�����v���₷���z�u
    CreatePlatformBlock(55, 7, 3, 1);  // ���u�̒���
    CreatePlatformBlock(52, 9, 2, 1);  // ���u�̒���
    CreatePlatformBlock(58, 9, 3, 1);  // ���u�̔��Α�

    // �ǉ��̑���
    CreatePlatformBlock(68, 6, 4, 1);  // ��������
    CreatePlatformBlock(75, 8, 3, 1);  // ���Ԃ̑���
    CreatePlatformBlock(85, 5, 3, 1);  // ����ɍ�������
    CreatePlatformBlock(92, 9, 2, 1);  // �S�[���O�̑���

    // �T�{�e���̂悤�ȍׂ�����
    CreatePlatformBlock(26, 12, 1, 1);
    CreatePlatformBlock(47, 7, 1, 1);
    CreatePlatformBlock(73, 4, 1, 1);
    CreatePlatformBlock(98, 10, 1, 1);
}