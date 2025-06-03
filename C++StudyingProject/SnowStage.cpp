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
    // ��̑��� - �W�����v���₷���z�u
    CreatePlatform(6, 11, 2, 1);   // �����Ȑ��
    CreatePlatform(14, 9, 3, 1);   // ��̑���
    CreatePlatform(22, 8, 2, 1);   // �������
    CreatePlatform(30, 7, 3, 1);   // �R���߂��̑���

    // �Ⴞ��܂̑���̂悤�ȉ~�`�ɋ߂��`
    CreatePlatform(38, 8, 1, 1);   // ��
    CreatePlatform(37, 9, 3, 1);   // ��
    CreatePlatform(38, 10, 1, 1);  // ��

    // �X�����̐����\�� - �o��₷������
    CreatePlatform(48, 9, 1, 1);   // �ŏ�i
    CreatePlatform(46, 10, 1, 1);  // 2�i��
    CreatePlatform(50, 10, 1, 1);  // 2�i�ځi�E�j
    CreatePlatform(48, 11, 1, 1);  // 3�i��

    // �ጴ�̕���
    CreatePlatform(58, 9, 4, 1);   // �傫�ȕ���
    CreatePlatform(65, 7, 3, 1);   // ��������
    CreatePlatform(75, 8, 4, 1);   // ���Ԃ̕���
    CreatePlatform(83, 6, 3, 1);   // ����ɍ�������

    // ��R�̓ˋN
    CreatePlatform(92, 7, 2, 1);   // �ˋN1
    CreatePlatform(98, 5, 2, 1);   // �ˋN2
    CreatePlatform(104, 8, 3, 1);  // �ˋN3

    // ��畗�ׂ̍�����
    CreatePlatform(16, 6, 1, 1);
    CreatePlatform(35, 5, 1, 1);
    CreatePlatform(55, 6, 1, 1);
    CreatePlatform(78, 4, 1, 1);
    CreatePlatform(101, 9, 1, 1);
}