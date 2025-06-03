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
    // ��̂悤�ȐΑ���X�e�[�W - ���o��₷���ȒP��
    int groundLevel = 13;
    int groundHeight = 3;

    // ��{�n�� - �Ԋu�����߂Ĉ��S��
    CreatePlatform(0, groundLevel, 20, groundHeight);    // �X�^�[�g�n�_�i�g���j
    CreatePlatform(22, groundLevel, 18, groundHeight);   // ���Ԓn�_1�i�Ԋu�������j
    CreatePlatform(42, groundLevel, 18, groundHeight);   // ���Ԓn�_2
    CreatePlatform(62, groundLevel, 18, groundHeight);   // ���Ԓn�_3
    CreatePlatform(82, groundLevel, 20, groundHeight);   // �I�Օ�
    CreatePlatform(104, groundLevel, 16, groundHeight);  // �S�[���O

    // �ɂ₩�ȊK�i��̒n�` - ���o��₷���A�i����������
    CreatePlatform(35, 12, 8, 1);   // 1�i�ځi�����L���j
    CreatePlatform(37, 11, 6, 1);   // 2�i��
    CreatePlatform(39, 10, 4, 1);   // 3�i��
    CreatePlatform(40, 9, 3, 1);    // 4�i�ځi����j
    CreatePlatform(41, 10, 2, 1);   // ����1�i��
    CreatePlatform(43, 11, 4, 1);   // ����2�i��
}

void StoneStage::CreatePlatforms() {
    // ��̓��̂悤�ȍ\�� - ������}���ēo��₷��
    CreatePlatform(8, 10, 2, 4);   // ���̓��i�Ⴍ�����j
    CreatePlatform(28, 9, 2, 5);   // �����̓��i������}���j
    CreatePlatform(68, 10, 2, 4);  // �E�̓��i�Ⴍ�����j

    // �����q���v���b�g�t�H�[�� - ��葽���̑���ň��S��
    CreatePlatform(11, 11, 10, 1); // ���̓����牄�т�i�����j
    CreatePlatform(23, 10, 4, 1);  // ���ԑ���1
    CreatePlatform(31, 10, 8, 1);  // �������ւ̋��i�����j
    CreatePlatform(41, 11, 6, 1);  // ���ԑ���2
    CreatePlatform(48, 10, 12, 1); // ������
    CreatePlatform(62, 11, 5, 1);  // ���ԑ���3
    CreatePlatform(71, 11, 8, 1);  // �E���ւ̋�

    // �ǉ��̈��S�ȑ��� - �W�����v���s���̋~�ϑ[�u
    CreatePlatform(15, 8, 4, 1);   // ���Ղ̍�������
    CreatePlatform(25, 7, 3, 1);   // ���Ԃ̑���
    CreatePlatform(45, 8, 4, 1);   // �����̍�������
    CreatePlatform(55, 7, 3, 1);   // �E���̑���
    CreatePlatform(75, 8, 4, 1);   // �I�Ղ̑���
    CreatePlatform(85, 9, 3, 1);   // �S�[���O�̑���

    // �i�K�I�ȏ㏸�p�̑��� - ���ׂ����X�e�b�v
    CreatePlatform(12, 9, 2, 1);   // �X�e�b�v1
    CreatePlatform(19, 8, 2, 1);   // �X�e�b�v2
    CreatePlatform(33, 8, 2, 1);   // �X�e�b�v3
    CreatePlatform(52, 8, 2, 1);   // �X�e�b�v4
    CreatePlatform(65, 9, 2, 1);   // �X�e�b�v5
    CreatePlatform(78, 8, 2, 1);   // �X�e�b�v6

    // �Ⴂ�����I�Ȑ΃u���b�N - �l�܂�̌����ƂȂ鍂����������
    CreatePlatform(18, 12, 1, 1);  // �����p�i�Ⴍ�j
    CreatePlatform(38, 12, 1, 1);  // �����p�i�Ⴍ�j
    CreatePlatform(58, 12, 1, 1);  // �����p�i�Ⴍ�j
    CreatePlatform(88, 12, 1, 1);  // �����p�i�Ⴍ�j

    // �ً}���p�̒Ⴂ�v���b�g�t�H�[��
    CreatePlatform(5, 12, 3, 1);   // �X�^�[�g�߂��̔��ꏊ
    CreatePlatform(35, 13, 4, 1);  // ���Ԃ̔��ꏊ
    CreatePlatform(65, 13, 4, 1);  // �I�Ղ̔��ꏊ
    CreatePlatform(95, 12, 3, 1);  // �S�[���O�̔��ꏊ
}