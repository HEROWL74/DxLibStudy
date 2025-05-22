#include "Game.h"

Game::Game(){}

Game::~Game() {}
//Œã‚ÉƒQ[ƒ€XVˆ—’Ç‰Á
void Game::Update() {};

void Game::Draw() {
	DrawString(100, 100, "Game Running", GetColor(255, 255, 255));
}
