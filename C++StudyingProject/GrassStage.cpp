#include "GrassStage.h"

GrassStage::GrassStage() {
    terrainType = "grass";
}

GrassStage::~GrassStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void GrassStage::Initialize() {
    // �e�N�X�`���ǂݍ���
    LoadTerrainTextures();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void GrassStage::CreateMainTerrain() {
    // ��{�n�ʂ̐����i�S�̂ɂ킽��n�ʁj
    int groundLevel = 12; // �O���b�h���W
    int groundHeight = 4; // �n�ʂ̌���

    // �g�����ꂽ���C���n�ʁi120�^�C�����j
    CreatePlatform(0, groundLevel, 120, groundHeight);

    // �n�ʂɌ������i�M���b�v�W�����v�p�j
    // ��1�i�����j
    CreatePlatform(15, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(28, groundLevel, 8, groundHeight);  // �E�̒n��
    // 23-28�̊Ԃ���

    // ��2�i�����j
    CreatePlatform(45, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(58, groundLevel, 8, groundHeight);  // �E�̒n��
    // 53-58�̊Ԃ���

    // ��3�i�㋉�j
    CreatePlatform(75, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(90, groundLevel, 8, groundHeight);  // �E�̒n��
    // 83-90�̊Ԃ���

    // �Ō�̒n��
    CreatePlatform(98, groundLevel, 22, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // ���V�v���b�g�t�H�[���i�g���Łj

    // ���ՃG���A
    CreatePlatform(5, 8, 4, 1);   // �����ȃv���b�g�t�H�[��
    CreatePlatform(12, 6, 3, 1);  // �W�����v�p�v���b�g�t�H�[��
    CreatePlatform(20, 4, 5, 1);  // �����v���b�g�t�H�[��

    // �M���b�v��n�邽�߂̃v���b�g�t�H�[���i��1�j
    CreatePlatform(24, 9, 2, 1);  // ���̏�̃v���b�g�t�H�[��
    CreatePlatform(26, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // ���ՃG���A
    CreatePlatform(35, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatform(42, 5, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatform(48, 3, 2, 1);  // �ō��_�̃v���b�g�t�H�[��

    // �M���b�v��n�邽�߂̃v���b�g�t�H�[���i��2�j
    CreatePlatform(54, 9, 2, 1);  // ��2�̏�̃v���b�g�t�H�[��
    CreatePlatform(56, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // ����ՃG���A
    CreatePlatform(65, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatform(72, 5, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatform(78, 3, 2, 1);  // �`�������W�v���b�g�t�H�[��

    // �M���b�v��n�邽�߂̃v���b�g�t�H�[���i��3�j
    CreatePlatform(85, 9, 2, 1);  // ��3�̏�̃v���b�g�t�H�[��
    CreatePlatform(87, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // �I�ՃG���A
    CreatePlatform(95, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatform(102, 5, 3, 1); // �����v���b�g�t�H�[��
    CreatePlatform(108, 3, 2, 1); // �ŏI�`�������W
    CreatePlatform(115, 8, 4, 1); // �S�[���O�v���b�g�t�H�[��
}