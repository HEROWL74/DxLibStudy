#include "Stage1Generate.h"
#include "DxLib.h"

Stage1Generate::Stage1Generate()
    : m_cameraX(0)
    , m_startPos(100, 800)
    , m_goalPos(6200, 800)
    , m_grassImg(-1)
    , m_stoneImg(-1)
    , m_spikeImg(-1)
    , m_ladderImg(-1)
    , m_springImg(-1)
    , m_flagImg(-1)
    , m_bgImg(-1)
{
    LoadGraphics();
    GenerateStage();
}

Stage1Generate::~Stage1Generate() {
    // �O���t�B�b�N�n���h���̉��
    if (m_grassImg != -1) DeleteGraph(m_grassImg);
    if (m_stoneImg != -1) DeleteGraph(m_stoneImg);
    if (m_spikeImg != -1) DeleteGraph(m_spikeImg);
    if (m_ladderImg != -1) DeleteGraph(m_ladderImg);
    if (m_springImg != -1) DeleteGraph(m_springImg);
    if (m_flagImg != -1) DeleteGraph(m_flagImg);
    if (m_bgImg != -1) DeleteGraph(m_bgImg);
}

void Stage1Generate::LoadGraphics() {
    // �摜�t�@�C���̓ǂݍ��݁i���݂��Ȃ��ꍇ�͐F�t���̋�`�ő�p�j
    m_grassImg = LoadGraph("Graphics/grass_tile.png");
    m_stoneImg = LoadGraph("Graphics/stone_tile.png");
    m_spikeImg = LoadGraph("Graphics/spike.png");
    m_ladderImg = LoadGraph("Graphics/ladder.png");
    m_springImg = LoadGraph("Graphics/spring.png");
    m_flagImg = LoadGraph("Graphics/flag.png");
    m_bgImg = LoadGraph("Graphics/background.png");
}

void Stage1Generate::GenerateStage() {
    // �X�e�[�W�S�̂̏��𐶐�
    // ��{�̏��i�n�ʁj
    for (int x = 0; x < m_stageWidth; x += TILE_SIZE) {
        m_floorRects.emplace_back(x, 980, TILE_SIZE, 100);  // �����̌�����
    }

    // �Z�N�V����1: �J�n�G���A�i0-800�j
    // ���R�Ȓn�ʂƃR�C��
    for (int i = 0; i < 5; i++) {
        m_coinRects.emplace_back(200 + i * 100, 900, 32, 32);
    }

    // �ŏ��̃v���b�g�t�H�[��
    m_platformRects.emplace_back(300, 850, 128, 32);
    m_platformRects.emplace_back(500, 800, 128, 32);
    m_coinRects.emplace_back(520, 750, 32, 32);

    // �Z�N�V����2: �K�i�G���A�i800-1600�j
    // �K�i��̃v���b�g�t�H�[��
    for (int i = 0; i < 6; i++) {
        m_platformRects.emplace_back(800 + i * 120, 900 - i * 50, 96, 32);
        if (i % 2 == 0) {
            m_coinRects.emplace_back(820 + i * 120, 850 - i * 50, 32, 32);
        }
    }

    // �Z�N�V����3: �X�p�C�N�G���A�i1600-2400�j
    // �X�p�C�N�̔z�u
    for (int i = 0; i < 8; i++) {
        m_spikeRects.emplace_back(1600 + i * 80, 980, 64, 32);
    }

    // �X�p�C�N������邽�߂̍����v���b�g�t�H�[��
    m_platformRects.emplace_back(1650, 750, 128, 32);
    m_platformRects.emplace_back(1900, 700, 128, 32);
    m_platformRects.emplace_back(2150, 650, 128, 32);

    // �����Ƃ���ɃR�C��
    m_coinRects.emplace_back(1670, 700, 32, 32);
    m_coinRects.emplace_back(1920, 650, 32, 32);
    m_coinRects.emplace_back(2170, 600, 32, 32);

    // �Z�N�V����4: �͂����G���A�i2400-3200�j
    // �c�̈ړ��v�f
    m_ladders.emplace_back(2500, 700, 64, 280);  // �������w�ւ̂͂���
    m_platformRects.emplace_back(2400, 700, 200, 32);  // �͂������̃v���b�g�t�H�[��
    m_platformRects.emplace_back(2450, 500, 150, 32);  // �͂�����̃v���b�g�t�H�[��

    // �͂������ӂ̃R�C��
    for (int i = 0; i < 3; i++) {
        m_coinRects.emplace_back(2420 + i * 60, 650, 32, 32);
    }
    m_coinRects.emplace_back(2480, 450, 32, 32);

    // 2�ڂ̂͂���
    m_ladders.emplace_back(2800, 600, 64, 380);
    m_platformRects.emplace_back(2750, 600, 150, 32);
    m_platformRects.emplace_back(2800, 400, 150, 32);

    // �Z�N�V����5: �W�����v�`�������W�G���A�i3200-4000�j
    // �A���W�����v���K�v�ȕ���
    for (int i = 0; i < 6; i++) {
        m_platformRects.emplace_back(3200 + i * 140, 800 - (i % 3) * 80, 96, 32);
        m_coinRects.emplace_back(3220 + i * 140, 750 - (i % 3) * 80, 32, 32);
    }

    // ����x�W�����v�G���A
    m_platformRects.emplace_back(3950, 650, 96, 32);
    m_coinRects.emplace_back(3970, 600, 32, 32);

    // �Z�N�V����6: �X�v�����O�G���A�i4000-4800�j
    // �X�v�����O�ō������ԃG���A
    m_springs.emplace_back(4100, 980);
    m_platformRects.emplace_back(4050, 600, 128, 32);
    m_coinRects.emplace_back(4070, 550, 32, 32);

    m_springs.emplace_back(4400, 980);
    m_platformRects.emplace_back(4350, 500, 128, 32);

    // �����i�̃v���b�g�t�H�[��
    m_platformRects.emplace_back(4600, 800, 96, 32);
    m_platformRects.emplace_back(4700, 700, 96, 32);
    m_platformRects.emplace_back(4800, 600, 96, 32);

    // �Z�N�V����7: �����`�������W�G���A�i4800-5600�j
    // �X�p�C�N�A�͂����A�v���b�g�t�H�[���̑g�ݍ��킹
    for (int i = 0; i < 4; i++) {
        m_spikeRects.emplace_back(4900 + i * 100, 980, 64, 32);
    }

    m_ladders.emplace_back(5000, 700, 64, 280);
    m_platformRects.emplace_back(4950, 700, 150, 32);
    m_platformRects.emplace_back(5100, 550, 150, 32);

    // �����`�F�[��
    for (int i = 0; i < 4; i++) {
        m_platformRects.emplace_back(5300 + i * 120, 750 - i * 30, 96, 32);
        m_coinRects.emplace_back(5320 + i * 120, 700 - i * 30, 32, 32);
    }

    // �Z�N�V����8: �ŏI�G���A�i5600-6400�j
    // �S�[���O�̍Ō�̃`�������W
    m_springs.emplace_back(5700, 980);
    m_platformRects.emplace_back(5650, 650, 128, 32);

    // �ŏI�W�����v
    m_platformRects.emplace_back(5900, 750, 96, 32);
    m_platformRects.emplace_back(6050, 700, 96, 32);

    // �S�[���G���A
    m_platformRects.emplace_back(6150, 850, 200, 32);

    // �{�[�i�X�R�C���i�����Ƃ����B�ꂽ�ꏊ�j
    m_coinRects.emplace_back(100, 750, 32, 32);    // �J�n����̍����ꏊ
    m_coinRects.emplace_back(1200, 600, 32, 32);   // �K�i�̏�
    m_coinRects.emplace_back(3000, 300, 32, 32);   // �͂����̍ŏ㕔
    m_coinRects.emplace_back(4200, 400, 32, 32);   // �X�v�����O���B�n�_
    m_coinRects.emplace_back(5500, 500, 32, 32);   // �����G���A�̉B���R�C��
    m_coinRects.emplace_back(6000, 650, 32, 32);   // �S�[���O
}

void Stage1Generate::DrawMap() {
    DrawTiles();
    DrawObjects();
}

void Stage1Generate::DrawTiles() {
    // �w�i
    if (m_bgImg != -1) {
        // �w�i���X�N���[���ɍ��킹�ĕ`��i�p�����b�N�X���ʁj
        int bgX = -(m_cameraX / 4) % 1920;  // �w�i��1/4�̑��x�ŃX�N���[��
        DrawGraph(bgX, 0, m_bgImg, TRUE);
        DrawGraph(bgX + 1920, 0, m_bgImg, TRUE);
    }
    else {
        // �w�i���Ȃ��ꍇ�͋�F�œh��Ԃ�
        DrawBox(0, 0, 1920, 1080, GetColor(135, 206, 235), TRUE);
    }

    // ���i�n�ʁj�̕`�� - �J�����I�t�Z�b�g��K�p
    for (const auto& floor : m_floorRects) {
        int drawX = floor.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
        int drawY = floor.y;              // Y���W�͂��̂܂�

        // ��ʊO�J�����O
        if (drawX + floor.w >= 0 && drawX < 1920) {
            if (m_grassImg != -1) {
                // �^�C����ɕ`��
                for (int x = 0; x < floor.w; x += TILE_SIZE) {
                    for (int y = 0; y < floor.h; y += TILE_SIZE) {
                        int tileDrawX = drawX + x;
                        int tileDrawY = drawY + y;
                        if (tileDrawX + TILE_SIZE >= 0 && tileDrawX < 1920) {
                            DrawGraph(tileDrawX, tileDrawY, m_grassImg, TRUE);
                        }
                    }
                }
            }
            else {
                DrawBox(drawX, drawY, drawX + floor.w, drawY + floor.h,
                    GetColor(34, 139, 34), TRUE);  // �ΐF
            }
        }
    }

    // �v���b�g�t�H�[���̕`�� - �J�����I�t�Z�b�g��K�p
    for (const auto& platform : m_platformRects) {
        int drawX = platform.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
        int drawY = platform.y;              // Y���W�͂��̂܂�

        // ��ʊO�J�����O
        if (drawX + platform.w >= 0 && drawX < 1920) {
            if (m_stoneImg != -1) {
                for (int x = 0; x < platform.w; x += TILE_SIZE) {
                    int tileDrawX = drawX + x;
                    if (tileDrawX + TILE_SIZE >= 0 && tileDrawX < 1920) {
                        DrawGraph(tileDrawX, drawY, m_stoneImg, TRUE);
                    }
                }
            }
            else {
                DrawBox(drawX, drawY, drawX + platform.w, drawY + platform.h,
                    GetColor(128, 128, 128), TRUE);  // �D�F
            }
        }
    }
}

void Stage1Generate::DrawObjects() {
    // �X�p�C�N�̕`�� - �J�����I�t�Z�b�g��K�p
    for (const auto& spike : m_spikeRects) {
        int drawX = spike.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
        int drawY = spike.y;

        // ��ʊO�J�����O
        if (drawX + spike.w >= 0 && drawX < 1920) {
            if (m_spikeImg != -1) {
                DrawGraph(drawX, drawY, m_spikeImg, TRUE);
            }
            else {
                // �O�p�`�ŃX�p�C�N��\��
                DrawTriangle(drawX, drawY + spike.h,
                    drawX + spike.w / 2, drawY,
                    drawX + spike.w, drawY + spike.h,
                    GetColor(255, 0, 0), TRUE);
            }
        }
    }

    // �͂����̕`�� - �J�����I�t�Z�b�g��K�p
    for (const auto& ladder : m_ladders) {
        int drawX = ladder.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
        int drawY = ladder.y;

        // ��ʊO�J�����O
        if (drawX + ladder.w >= 0 && drawX < 1920) {
            if (m_ladderImg != -1) {
                for (int y = 0; y < ladder.h; y += TILE_SIZE) {
                    DrawGraph(drawX, drawY + y, m_ladderImg, TRUE);
                }
            }
            else {
                // ���ł͂͂�����\��
                DrawBox(drawX + 10, drawY, drawX + 15, drawY + ladder.h,
                    GetColor(139, 69, 19), TRUE);
                DrawBox(drawX + ladder.w - 15, drawY, drawX + ladder.w - 10, drawY + ladder.h,
                    GetColor(139, 69, 19), TRUE);
                // ���_
                for (int y = 0; y < ladder.h; y += 20) {
                    DrawBox(drawX + 10, drawY + y, drawX + ladder.w - 10, drawY + y + 5,
                        GetColor(139, 69, 19), TRUE);
                }
            }
        }
    }

    // �X�v�����O�̕`�� - �J�����I�t�Z�b�g��K�p
    for (const auto& spring : m_springs) {
        int drawX = spring.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
        int drawY = spring.y;

        // ��ʊO�J�����O
        if (drawX + TILE_SIZE >= 0 && drawX < 1920) {
            if (m_springImg != -1) {
                DrawGraph(drawX, drawY - 32, m_springImg, TRUE);
            }
            else {
                // �ȒP�ȃX�v�����O�\��
                int height = spring.isPressed ? 20 : 32;
                DrawBox(drawX + 16, drawY - height, drawX + 48, drawY,
                    GetColor(255, 215, 0), TRUE);  // ���F
                // �R�C���\��
                for (int i = 0; i < 3; i++) {
                    DrawCircle(drawX + 32, drawY - height + i * 8, 12,
                        GetColor(255, 165, 0), FALSE);
                }
            }
        }
    }

    // �S�[���t���O�̕`�� - �J�����I�t�Z�b�g��K�p
    int flagDrawX = m_goalPos.x - m_cameraX;  // �J�����I�t�Z�b�g��K�p
    int flagDrawY = m_goalPos.y;

    // ��ʊO�J�����O
    if (flagDrawX >= -100 && flagDrawX < 1920) {
        if (m_flagImg != -1) {
            DrawGraph(flagDrawX, flagDrawY - 100, m_flagImg, TRUE);
        }
        else {
            // �|�[��
            DrawBox(flagDrawX, flagDrawY - 100, flagDrawX + 8, flagDrawY,
                GetColor(139, 69, 19), TRUE);
            // �t���O
            DrawBox(flagDrawX + 8, flagDrawY - 100, flagDrawX + 68, flagDrawY - 60,
                GetColor(255, 0, 0), TRUE);
        }
    }
}

void Stage1Generate::UpdateSprings() {
    for (auto& spring : m_springs) {
        if (spring.isPressed) {
            spring.animFrame++;
            if (spring.animFrame > 10) {
                spring.isPressed = false;
                spring.animFrame = 0;
            }
        }
    }
}