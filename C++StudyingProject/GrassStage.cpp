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

    // ���C���n��
    CreatePlatform(0, groundLevel, 60, groundHeight);

    // �n�ʂɌ������i�M���b�v�W�����v�p�j
    CreatePlatform(15, groundLevel, 8, groundHeight);  // ���̒n��
    CreatePlatform(28, groundLevel, 8, groundHeight);  // �E�̒n��
    // 23-28�̊Ԃ���

    // �㔼�̒n��
    CreatePlatform(40, groundLevel, 20, groundHeight);
}

void GrassStage::CreatePlatforms() {
    // ���V�v���b�g�t�H�[��
    CreatePlatform(5, 8, 4, 1);   // �����ȃv���b�g�t�H�[��
    CreatePlatform(12, 6, 3, 1);  // �W�����v�p�v���b�g�t�H�[��
    CreatePlatform(20, 4, 5, 1);  // �����v���b�g�t�H�[��

    // �M���b�v��n�邽�߂̃v���b�g�t�H�[��
    CreatePlatform(24, 9, 2, 1);  // ���̏�̃v���b�g�t�H�[��
    CreatePlatform(26, 7, 2, 1);  // ��荂���v���b�g�t�H�[��

    // �㔼�̃v���b�g�t�H�[��
    CreatePlatform(45, 8, 6, 1);  // �傫�ȃv���b�g�t�H�[��
    CreatePlatform(52, 5, 3, 1);  // �����v���b�g�t�H�[��
    CreatePlatform(57, 3, 2, 1);  // �ō��_�̃v���b�g�t�H�[��
}