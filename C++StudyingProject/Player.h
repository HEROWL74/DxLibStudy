#pragma once

class Player {
public:
	Player();
	~Player();

	void Update();
	void Draw();

private:
	int m_handle;
	float m_x, m_y;
	float m_vx, m_vy;
	bool m_onGround;

	const float gravity = 0.5f;
	const float moveSpeed = 4.0f;
	const float jumpPower = -10.0f;
};
