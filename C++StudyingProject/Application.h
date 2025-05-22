#pragma once
#include "DxLib.h"
#include <memory>
#include "Game.h"

class Game;

class Application
{
public:
	Application();
	~Application();

	//�R�s�[���֎~������
	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;

	void Run();

private:
	bool Initialize();
	void Update();
	void Release();

	std::unique_ptr<Game> m_game;
};