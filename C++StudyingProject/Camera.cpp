#include "Camera.h"
#include "Player.h"
#include "Stage1Generate.h"
#include <algorithm>
using namespace std;
Camera::Camera() : m_x(0), m_y(0) {
}

void Camera::Update(const Player& player, const Stage1Generate& stage) {
    // プレイヤーの位置を取得
    int playerX = player.GetX();
    int playerY = player.GetY();

    // 目標カメラ位置を計算（プレイヤーが画面の1/3地点に来るように）
    int targetX = playerX - CAMERA_OFFSET_X;
    int targetY = playerY - SCREEN_HEIGHT / 2;

    // ステージの境界内にカメラを制限
    int stageWidth = stage.GetStageWidth();
    targetX = max(0, min(targetX, stageWidth - SCREEN_WIDTH));

    // Y軸は基本的に固定（必要に応じて調整）
    targetY = max(0, min(targetY, 0));  // 現在は上下スクロールなし

    // プレイヤーが画面外に出そうになったら即座に追従
    int playerScreenX = playerX - m_x;

    if (playerScreenX < 100) {
        // プレイヤーが左端に近づいた場合
        m_x = max(0, playerX - 100);
    }
    else if (playerScreenX > SCREEN_WIDTH - 100) {
        // プレイヤーが右端に近づいた場合
        m_x = min(stageWidth - SCREEN_WIDTH, playerX - (SCREEN_WIDTH - 100));
    }
    else {
        // 通常時はスムーズに移動（より高速に）
        int diff = targetX - m_x;
        if (abs(diff) > 5) {
            m_x += diff / 2;  // より高速な追従
        }
        else {
            m_x = targetX;
        }
    }

    // Y軸の更新
    int diffY = targetY - m_y;
    if (abs(diffY) > 2) {
        m_y += diffY / 2;
    }
    else {
        m_y = targetY;
    }
}