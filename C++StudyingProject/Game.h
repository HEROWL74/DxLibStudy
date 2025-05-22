#pragma once

class Game {
public:
	Game();
	~Game();

	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;

	void Update();
	void Draw();
};
