#include "TitleScene.h"
#include "DxLib.h"

TitleScene::TitleScene() :
    m_fadeOut(false), m_fadeAlpha(0) {}

void TitleScene::Update() {
    if (!m_fadeOut && CheckHitKey(KEY_INPUT_RETURN)) {
        m_fadeOut = true;
    }

    if (m_fadeOut) {
        m_fadeAlpha += 5;  // フェードスピード
        if (m_fadeAlpha > 255) {
            m_fadeAlpha = 255;
        }
    }
}

void TitleScene::Draw() {
    DrawString(160, 100, "Press ENTER to Start", GetColor(255, 255, 255));

    if (m_fadeOut) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, m_fadeAlpha);
        DrawBox(0, 0, 1920, 1080, GetColor(0, 0, 0), TRUE);
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
    }
}

bool TitleScene::IsFadeOutFinished() const {
    return m_fadeOut && m_fadeAlpha >= 255;
}
bool TitleScene::IsStartRequested() const {
    return m_start;
}
