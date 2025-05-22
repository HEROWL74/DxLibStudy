#include "GameScene.h"
#include "DxLib.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
    m_player = std::make_unique<Player>();
}

void GameScene::Update() {
    m_player->Update();
}

void GameScene::Draw() {
    m_generator->DrawMap();
    m_player->Draw();
}
