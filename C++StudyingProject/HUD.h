#pragma once
#include "DxLib.h"

class HUD {
public:
    HUD();
    ~HUD();

    // currentHealth�F����HP�i1���݁j�AmaxHealth�F�ő�HP�Acoins�F�����R�C����
    void Draw(int currentHealth, int maxHealth, int coins);

private:
    int imgHeartFull;
    int imgHeartHalf;
    int imgHeartEmpty;
    int imgCoin;
    int imgPlayerIcon;

    static constexpr int ICON_SIZE = 32;  // �摜�̕`��T�C�Y
    static constexpr int MARGIN_ICON_HEART = 20; // �A�C�R���ƃn�[�g�̊Ԋu
    static constexpr int MARGIN_HEART = 8;  // �n�[�g���m�̊Ԋu
    static constexpr int MARGIN_HEART_COIN = 20; // �n�[�g�ƃR�C���̊Ԋu
};
