#include "SnowStage.h"

SnowStage::SnowStage() {
    terrainType = "snow";
}

SnowStage::~SnowStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void SnowStage::Initialize() {
    // �e�N�X�`���ǂݍ���
    LoadTerrainTextures();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void SnowStage::CreateMainTerrain() {
    // ��R�̌������n�`
    int groundHeight = 4;

    // �R�̎Ζʂ�\��
    CreatePlatform(0, 14, 8, groundHeight);    // �[
    CreatePlatform(8, 12, 6, groundHeight);    // ������
    CreatePlatform(14, 10, 6, groundHeight);   // ����
    CreatePlatform(20, 8, 8, groundHeight);    // �R���t��
    CreatePlatform(28, 10, 6, groundHeight);   // �����
    CreatePlatform(34, 12, 8, groundHeight);   // �܂����
    CreatePlatform(42, 9, 18, groundHeight);   // ����
}

void SnowStage::CreatePlatforms() {
    // �X�̑���
    CreatePlatform(6, 10, 1, 1);   // �����ȕX��
    CreatePlatform(12, 7, 2, 1);   // �X�̑���
    CreatePlatform(18, 5, 1, 1);   // �����X��

    // �Ⴞ��܂̑���̂悤�ȉ~�`�ɋ߂��`
    CreatePlatform(25, 6, 1, 1);   // ��
    CreatePlatform(24, 7, 3, 1);   // ��
    CreatePlatform(25, 8, 1, 1);   // ��

    // ��畗�̐����\��
    CreatePlatform(40, 5, 1, 1);
    CreatePlatform(45, 4, 1, 2);
    CreatePlatform(50, 3, 1, 3);
    CreatePlatform(55, 2, 1, 4);
}