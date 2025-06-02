#include "SandStage.h"

SandStage::SandStage() {
    terrainType = "sand";
}

SandStage::~SandStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void SandStage::Initialize() {
    // �e�N�X�`���ǂݍ���
    LoadTerrainTextures();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void SandStage::CreateMainTerrain() {
    // �����̋N���̂���n�`
    int groundHeight = 3;

    // �N���̂���n��
    CreatePlatform(0, 13, 12, groundHeight);   // �Ⴂ�n��
    CreatePlatform(12, 11, 8, groundHeight + 2); // ���������n��
    CreatePlatform(20, 14, 10, groundHeight);  // �܂��Ⴂ�n��
    CreatePlatform(30, 10, 10, groundHeight + 3); // �����u
    CreatePlatform(40, 12, 20, groundHeight + 1); // �Ō�̒n��
}

void SandStage::CreatePlatforms() {
    // �����̃I�A�V�X����̃v���b�g�t�H�[��
    CreatePlatform(5, 9, 3, 1);   // �����ȓ�
    CreatePlatform(15, 7, 2, 1);  // �W�����v��
    CreatePlatform(25, 8, 4, 1);  // �����炢�̓�

    // �s���~�b�h���̍\��
    CreatePlatform(35, 8, 1, 1);  // ����
    CreatePlatform(34, 9, 3, 1);  // 2�i��
    CreatePlatform(33, 10, 5, 1); // 1�i�ځi���ۂ�3�i�ځj

    // ���u��\������v���b�g�t�H�[��
    CreatePlatform(48, 8, 2, 1);
    CreatePlatform(52, 6, 3, 1);
    CreatePlatform(56, 4, 2, 1);
}