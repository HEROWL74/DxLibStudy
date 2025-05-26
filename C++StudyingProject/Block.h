#pragma once
#include "DxLib.h"
#include "Player.h"

class Block {
public:
    enum class State { Rest, Idle, Fall };  // Fall は「触れられたときの表示」用

    Block(int x, int y);
    ~Block();

    // 毎フレームの状態更新（位置は変えない）
    void Update(const Player& player);
    void Draw() const;

    int GetX() const { return m_x; }
    int GetY() const { return m_y; }
    int GetW() const { return m_width; }
    int GetH() const { return m_height; }

private:
    int   m_x, m_y;
    int   m_width, m_height;
    State m_state;

    int   m_handleRest;
    int   m_handleIdle;
    int   m_handleFall;

    static constexpr int idleDist = 100;  // プレイヤー接近判定距離
};
