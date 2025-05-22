#include "GameScene.h"
#include "DxLib.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();

    m_fadeIn = true;
    m_fadeAlpha = 255;
}

void GameScene::Update() {
    m_player->Update();
    // �t�F�[�h�C������
    if (m_fadeIn) {
        m_fadeAlpha -= 5;  // ���X�ɓ����Ɂi�X�s�[�h�����j
        if (m_fadeAlpha <= 0) {
            m_fadeAlpha = 0;
            m_fadeIn = false;
        }
    }

}

void GameScene::Draw() {
    m_generator->DrawMap();
    m_player->Draw();
    // �t�F�[�h�C���`��
    if (m_fadeIn) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }

}
