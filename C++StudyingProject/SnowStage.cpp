#include "SnowStage.h"

SnowStage::SnowStage() {
    terrainType = "snow";
}

SnowStage::~SnowStage() {
    // ���N���X�̃f�X�g���N�^���Ă΂��
}

void SnowStage::Initialize() {
    // ���C���e�N�X�`���ǂݍ��݁i�n�ʗp�j
    LoadTerrainTextures();

    // **�v���b�g�t�H�[����p�e�N�X�`���ǂݍ��݁iterrain_snow_block.png�j**
    LoadPlatformTexture();

    // �n�`����
    CreateMainTerrain();
    CreatePlatforms();
}

void SnowStage::CreateMainTerrain() {
    // ��R�̌������n�` - ���o��₷������
    int groundHeight = 4;

    // �R�̎Ζʂ�\�� - �i����������
    CreatePlatform(0, 14, 10, groundHeight);    // �[
    CreatePlatform(10, 13, 8, groundHeight);    // ������
    CreatePlatform(18, 12, 8, groundHeight);    // ����
    CreatePlatform(26, 11, 8, groundHeight);    // �R���t��
    CreatePlatform(34, 10, 8, groundHeight);    // ����
    CreatePlatform(42, 11, 8, groundHeight);    // �����
    CreatePlatform(50, 12, 10, groundHeight);   // ����
    CreatePlatform(60, 13, 12, groundHeight);   // �[�ɖ߂�
    CreatePlatform(72, 12, 15, groundHeight);   // ����ȕ���
    CreatePlatform(87, 10, 20, groundHeight);   // ����
    CreatePlatform(107, 12, 13, groundHeight);  // �S�[���O
}

void SnowStage::CreatePlatforms() {
    // **��̑��� - terrain_snow_block.png�e�N�X�`���g�p**
    // �W�����v���₷���z�u
    CreatePlatformBlock(6, 11, 2, 1);   // �����Ȑ��
    CreatePlatformBlock(14, 9, 3, 1);   // ��̑���
    CreatePlatformBlock(22, 8, 2, 1);   // �������
    CreatePlatformBlock(30, 7, 3, 1);   // �R���߂��̑���

    // �Ⴞ��܂̑���̂悤�ȉ~�`�ɋ߂��`
    CreatePlatformBlock(38, 8, 1, 1);   // ��
    CreatePlatformBlock(37, 9, 3, 1);   // ��
    CreatePlatformBlock(38, 10, 1, 1);  // ��

    // �X�����̓˂��o���\�� - �o��₷������
    CreatePlatformBlock(48, 9, 1, 1);   // �ŏ�i
    CreatePlatformBlock(46, 10, 1, 1);  // 2�i��
    CreatePlatformBlock(50, 10, 1, 1);  // 2�i�ځi�E�j
    CreatePlatformBlock(48, 11, 1, 1);  // 3�i��

    // �ጴ�̕���
    CreatePlatformBlock(58, 9, 4, 1);   // �傫�ȕ���
    CreatePlatformBlock(65, 7, 3, 1);   // ��������
    CreatePlatformBlock(75, 8, 4, 1);   // ���Ԃ̕���
    CreatePlatformBlock(83, 6, 3, 1);   // ����ɍ�������

    // ��R�̓ˋN
    CreatePlatformBlock(92, 7, 2, 1);   // �ˋN1
    CreatePlatformBlock(98, 5, 2, 1);   // �ˋN2
    CreatePlatformBlock(104, 8, 3, 1);  // �ˋN3

    // ��畗�ׂ̍�����
    CreatePlatformBlock(16, 6, 1, 1);
    CreatePlatformBlock(35, 5, 1, 1);
    CreatePlatformBlock(55, 6, 1, 1);
    CreatePlatformBlock(78, 4, 1, 1);
    CreatePlatformBlock(101, 9, 1, 1);
}