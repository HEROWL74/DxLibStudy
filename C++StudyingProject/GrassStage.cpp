#include "GrassStage.h"

GrassStage::GrassStage() {
    terrainType = "grass";
}

GrassStage::~GrassStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void GrassStage::Initialize() {
    // ���C���e�N�X�`���ǂݍ��݁i�n�ʗp�j
    LoadTerrainTextures();

    // **�v���b�g�t�H�[����p�e�N�X�`���ǂݍ��݁iterrain_grass_block.png�j**
    LoadPlatformTexture();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void GrassStage::CreateMainTerrain() {
    // ��{�n�ʂ̐����i�S�̂ɂ킽��n�ʁj- ���C���e�N�X�`���g�p
    int groundLevel = 12; // �O���b�h���W
    int groundHeight = 4; // �n�ʂ̌���

    // �g���������C���̕����̃v���b�g�t�H�[���i120�^�C�����g�p�j
    CreatePlatform(0, groundLevel, 120, groundHeight);

    // �n�ʂɌ��Ԃ����i�M���b�v�W�����v�p�j- ���Ԃ����߂ē�Փx��h�~
    // ����1�i�����j- ����3�^�C���Ɋg��
    CreatePlatform(15, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(26, groundLevel, 8, groundHeight);  // �E�̒n��
    // 23-26�̊Ԃ����ԁi3�^�C�����j

    // ����2�i�����j- ����4�^�C���Ɋg��
    CreatePlatform(40, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(52, groundLevel, 8, groundHeight);  // �E�̒n��
    // 48-52�̊Ԃ����ԁi4�^�C�����j

    // ����3�i�㋉�j- ����5�^�C���Ɋg��
    CreatePlatform(70, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(83, groundLevel, 8, groundHeight);  // �E�̒n��
    // 78-83�̊Ԃ����ԁi5�^�C�����j

    // �Ō�̒n��
    CreatePlatform(95, groundLevel, 25, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // **���V�v���b�g�t�H�[���iterrain_grass_block.png�e�N�X�`���g�p�j**
    // �v���C���[���l�܂�Ȃ��悤�z�u

    // ���ՃG���A - �W�����v�̗��K�p�igrass block�e�N�X�`���j
    CreatePlatformBlock(5, 8, 4, 1);   // �����ȃv���b�g�t�H�[��
    CreatePlatformBlock(12, 6, 3, 1);  // �W�����v�p�v���b�g�t�H�[��
    CreatePlatformBlock(18, 4, 5, 1);  // �����v���b�g�t�H�[��

    // �M���b�v1��n�邽�߂̃v���b�g�t�H�[�� - �z�u�𒲐�
    CreatePlatformBlock(20, 9, 3, 1);  // ���Ԓ��O�̑���
    CreatePlatformBlock(24, 7, 2, 1);  // ���Ԃ̍�������

    // ���ՃG���A - ��荂�x�ȃW�����v
    CreatePlatformBlock(32, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatformBlock(45, 6, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatformBlock(50, 3, 2, 1);  // �ō��_�̃v���b�g�t�H�[��

    // �M���b�v2��n�邽�߂̃v���b�g�t�H�[�� - �z�u�𒲐�
    CreatePlatformBlock(48, 9, 2, 1);  // ���ԏ�̃v���b�g�t�H�[��
    CreatePlatformBlock(50, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // ����ՃG���A
    CreatePlatformBlock(60, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatformBlock(67, 5, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatformBlock(73, 3, 2, 1);  // �`�������W�v���b�g�t�H�[��

    // �M���b�v3��n�邽�߂̃v���b�g�t�H�[�� - �z�u�𒲐�
    CreatePlatformBlock(79, 9, 2, 1);  // ���ԏ�̃v���b�g�t�H�[��
    CreatePlatformBlock(81, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // �I�ՃG���A - �S�[���O�̍ŏI�`�������W
    CreatePlatformBlock(88, 8, 4, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatformBlock(95, 5, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatformBlock(100, 3, 2, 1); // �ŏI�`�������W
    CreatePlatformBlock(108, 8, 4, 1); // �S�[���O�v���b�g�t�H�[��
    CreatePlatformBlock(115, 6, 3, 1); // �S�[�����O�̑���
}