#include "GameScene.h"
#include "DxLib.h"

GameScene::GameScene() {
    m_generator = std::make_unique<TitleGenerate>();
}

void GameScene::Update() {
    
}

void GameScene::Draw() {
    m_generator->DrawMap();
}
