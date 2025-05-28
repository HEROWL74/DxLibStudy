#include "Camera.h"
#include "Player.h"
#include "Stage1Generate.h"
#include <algorithm>
using namespace std;
Camera::Camera() : m_x(0), m_y(0) {
}

void Camera::Update(const Player& player, const Stage1Generate& stage) {
    // �v���C���[�̈ʒu���擾
    int playerX = player.GetX();
    int playerY = player.GetY();

    // �ڕW�J�����ʒu���v�Z�i�v���C���[����ʂ�1/3�n�_�ɗ���悤�Ɂj
    int targetX = playerX - CAMERA_OFFSET_X;
    int targetY = playerY - SCREEN_HEIGHT / 2;

    // �X�e�[�W�̋��E���ɃJ�����𐧌�
    int stageWidth = stage.GetStageWidth();
    targetX = max(0, min(targetX, stageWidth - SCREEN_WIDTH));

    // Y���͊�{�I�ɌŒ�i�K�v�ɉ����Ē����j
    targetY = max(0, min(targetY, 0));  // ���݂͏㉺�X�N���[���Ȃ�

    // �v���C���[����ʊO�ɏo�����ɂȂ����瑦���ɒǏ]
    int playerScreenX = playerX - m_x;

    if (playerScreenX < 100) {
        // �v���C���[�����[�ɋ߂Â����ꍇ
        m_x = max(0, playerX - 100);
    }
    else if (playerScreenX > SCREEN_WIDTH - 100) {
        // �v���C���[���E�[�ɋ߂Â����ꍇ
        m_x = min(stageWidth - SCREEN_WIDTH, playerX - (SCREEN_WIDTH - 100));
    }
    else {
        // �ʏ펞�̓X���[�Y�Ɉړ��i��荂���Ɂj
        int diff = targetX - m_x;
        if (abs(diff) > 5) {
            m_x += diff / 2;  // ��荂���ȒǏ]
        }
        else {
            m_x = targetX;
        }
    }

    // Y���̍X�V
    int diffY = targetY - m_y;
    if (abs(diffY) > 2) {
        m_y += diffY / 2;
    }
    else {
        m_y = targetY;
    }
}