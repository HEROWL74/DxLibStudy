#include "TitleScene.h"
#include "DxLib.h"

TitleScene::TitleScene() : m_start(false) {}

void TitleScene::Update() {
    if (CheckHitKey(KEY_INPUT_RETURN)) {
        m_start = true;
    }
}

void TitleScene::Draw() {
    DrawString(160, 100, "Press ENTER to Start", GetColor(255, 255, 255));
}

bool TitleScene::IsStartRequested() const {
    return m_start;
}
