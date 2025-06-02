#include "StoneStage.h"

StoneStage::StoneStage() {
    terrainType = "stone";
}

StoneStage::~StoneStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void StoneStage::Initialize() {
    // �e�N�X�`���ǂݍ���
    LoadTerrainTextures();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void StoneStage::CreateMainTerrain() {
    // ��̂悤�ȐΑ���X�e�[�W
    int groundLevel = 13;
    int groundHeight = 3;

    // ��{�n��
    CreatePlatform(0, groundLevel, 15, groundHeight);
    CreatePlatform(20, groundLevel, 15, groundHeight);
    CreatePlatform(40, groundLevel, 20, groundHeight);

    // �K�i��̒n�`
    CreatePlatform(35, 12, 5, 1);
    CreatePlatform(36, 11, 3, 1);
    CreatePlatform(37, 10, 1, 1);
}

void StoneStage::CreatePlatforms() {
    // ��̓��̂悤�ȍ\��
    CreatePlatform(8, 8, 2, 5);   // ���̓�
    CreatePlatform(25, 6, 2, 7);  // �����̍�����
    CreatePlatform(45, 9, 2, 4);  // �E�̓�

    // �����q���v���b�g�t�H�[��
    CreatePlatform(10, 10, 6, 1); // ���̓����牄�т�
    CreatePlatform(27, 8, 8, 1);  // �����̓����牄�т�
    CreatePlatform(47, 11, 6, 1); // �E�̓����牄�т�

    // ���є��̂悤�ȃu���b�N
    CreatePlatform(18, 11, 1, 2);
    CreatePlatform(38, 10, 1, 3);
    CreatePlatform(55, 9, 1, 4);
}