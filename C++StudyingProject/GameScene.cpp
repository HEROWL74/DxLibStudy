#include "GameScene.h"
#include "DxLib.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();

    // ���� �G�����i���̂܂܁j ����
    m_enemies.push_back(std::make_unique<NormalSlime>(200, 800));
    m_enemies.push_back(std::make_unique<SpikeSlime>(400, 800));

    // ���� �󒆃v���b�g�t�H�[���Ƃ��ău���b�N��z�u ����
    const int tileSize = 64;
    const int mapH = m_generator->GetMapHeight();
    const int floorY = 1080 - mapH * tileSize;       // �n�㏰��Y���W
    const int airLevel = floorY - 3 * tileSize;        // ������3�s����

    // X ���W�͂��D�݂ŁB�����ł͉����т� 5 �z�u�����
    int startX = 500;
    int count = 5;
    for (int i = 0; i < count; ++i) {
        int bx = startX + i * tileSize;   // �^�C�������E��
        m_blocks.push_back(std::make_unique<Block>(bx, airLevel));
    }
    m_fadeIn = true;
    m_fadeAlpha = 255;
}

void GameScene::Update() {
    // �v���C���[�O�t���[���ʒu�E�T�C�Y�擾
    int prevX = m_player->GetX();
    int prevY = m_player->GetY();
    int pw = m_player->GetW();
    int ph = m_player->GetH();

    // �v���C���[�X�V�i���́E�����j
    m_player->Update();

    {
        float vy = m_player->GetVY();
        // �������Ɉړ�����������
        if (vy > 0.0f) {
            int px = m_player->GetX();
            int py = m_player->GetY();
            int pw = m_player->GetW();
            int ph = m_player->GetH();
            float oldBottom = (py - vy) + ph;
            float newBottom = py + ph;

            for (auto& b : m_blocks) {
                int bx = b->GetX();
                int by = b->GetY();
                int bw = b->GetW();
                int bh = b->GetH();
                // �����͈͏d�Ȃ�
                if (px + pw > bx && px < bx + bw) {
                    // ����ӂ��u���b�N�̏�ʂ���i���j�ŁA�V��ӂ����ʂ�艺�i>=�j�Ȃ璅�n
                    if (oldBottom <= by && newBottom >= by) {
                        // ���n����
                        m_player->SetY(static_cast<float>(by - ph));
                        m_player->SetVY(0.0f);
                        m_player->SetOnGround(true);
                        break;
                    }
                }
            }
        }
    }

    // 4) ���i�}�b�v���[�j�Ƃ̓����蔻��
    {
        const int tileSize = 64;
        const int mapH = m_generator->GetMapHeight();
        const int floorY = 1080 - mapH * tileSize;
        int       py = m_player->GetY();
        float     vy = m_player->GetVY();

        if (vy > 0 && py + ph > floorY) {
            m_player->SetY(floorY - ph);
            m_player->SetVY(0);
            m_player->SetOnGround(true);
        }
    }

    // 5) �G�X�V�{�Փ˔���
    for (auto& e : m_enemies) {
        e->Update();
        int px = m_player->GetX();
        int py = m_player->GetY();
        int pw = m_player->GetW();
        int ph = m_player->GetH();
        if (e->IsColliding(px, py, pw, ph)) {
            // TODO: �Փˎ��̏����i�_���[�W�A�m�b�N�o�b�N�ASE�Ȃǁj
        }
    }

    for (auto& b : m_blocks) {
        b->Update(*m_player);
    }

    // 6) �t�F�[�h�C������
    if (m_fadeIn) {
        m_fadeAlpha -= 5;
        if (m_fadeAlpha <= 0) {
            m_fadeAlpha = 0;
            m_fadeIn = false;
        }
    }
}


void GameScene::Draw() {
    m_generator->DrawMap();
    m_player->Draw();
    //�u���b�N�͏��̏�ɕ`��
    for (auto& b : m_blocks) {
        b->Draw();
    }
    // �t�F�[�h�C���`��
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
    for (auto& e : m_enemies) {
        e->Draw();
    }

}
