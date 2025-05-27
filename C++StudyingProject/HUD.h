#pragma once
#include "DxLib.h"
#include <array>
class HUD {
public:
    HUD();
    ~HUD();

    // currentHealth�F����HP�i1���݁j�AmaxHealth�F�ő�HP�Acoins�F�����R�C����
    void Draw(int currentHealth, int maxHealth, int coins);
    
    // �R�C���A�C�R���̕`��ʒu���擾�i���ʂ̃^�[�Q�b�g���W�j
    std::pair<int, int> GetCoinDigitPos(int maxHearts, int coinCount) const;
private:
    int imgHeartFull;
    int imgHeartHalf;
    int imgHeartEmpty;
    int imgCoin;
    int imgPlayerIcon;
    

    //�����X�v���C�g
    std::array<int, 10> imgDigits;
    static constexpr int START_X = 20;
    static constexpr int START_Y = 20;
    static constexpr int DIGIT_WIDTH = 24; //�����̉摜�̕�
    static constexpr int DIGIT_HEIGHT = 32; //�����̉摜�̍���
    static constexpr int MARGIN_DIGIT = 4;//�������m�̊Ԋu

    static constexpr int ICON_SIZE = 32;  // �摜�̕`��T�C�Y
    static constexpr int MARGIN_ICON_HEART = 30; // �A�C�R���ƃn�[�g�̊Ԋu
    static constexpr int MARGIN_HEART = 16;  // �n�[�g���m�̊Ԋu
    static constexpr int MARGIN_HEART_COIN = 30; // �n�[�g�ƃR�C���̊Ԋu
};
