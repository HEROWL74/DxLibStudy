#include "Game.h"

Game::Game(){}

Game::~Game() {}
//��ɃQ�[���X�V�����ǉ�
void Game::Update() {};

void Game::Draw() {
	DrawString(100, 100, "Game Running", GetColor(255, 255, 255));
}
