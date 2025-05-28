#pragma once

class Player;
class Stage1Generate;

class Camera {
public:
    Camera();
    ~Camera() = default;

    // カメラ更新（プレイヤーに追従）
    void Update(const Player& player, const Stage1Generate& stage);

    // カメラ位置取得
    int GetX() const { return m_x; }
    int GetY() const { return m_y; }

    // カメラ位置設定
    void SetX(int x) { m_x = x; }
    void SetY(int y) { m_y = y; }

private:
    int m_x, m_y;

    // カメラの制約
    static const int SCREEN_WIDTH = 1920;
    static const int SCREEN_HEIGHT = 1080;
    static const int CAMERA_OFFSET_X = 640;  // プレイヤーが画面の1/3位置に来るようなオフセット
    static const int CAMERA_SMOOTH = 4;      // カメラの滑らかな移動（小さいほど高速追従）
};