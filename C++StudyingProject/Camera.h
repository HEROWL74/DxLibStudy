#pragma once

class Player;
class Stage1Generate;

class Camera {
public:
    Camera();
    ~Camera() = default;

    // �J�����X�V�i�v���C���[�ɒǏ]�j
    void Update(const Player& player, const Stage1Generate& stage);

    // �J�����ʒu�擾
    int GetX() const { return m_x; }
    int GetY() const { return m_y; }

    // �J�����ʒu�ݒ�
    void SetX(int x) { m_x = x; }
    void SetY(int y) { m_y = y; }

private:
    int m_x, m_y;

    // �J�����̐���
    static const int SCREEN_WIDTH = 1920;
    static const int SCREEN_HEIGHT = 1080;
    static const int CAMERA_OFFSET_X = 640;  // �v���C���[����ʂ�1/3�ʒu�ɗ���悤�ȃI�t�Z�b�g
    static const int CAMERA_SMOOTH = 4;      // �J�����̊��炩�Ȉړ��i�������قǍ����Ǐ]�j
};