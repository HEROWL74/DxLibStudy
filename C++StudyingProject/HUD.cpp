#include "HUD.h"
#include <string>
HUD::HUD() {
    imgHeartFull = LoadGraph("Sprites/Tiles/Default/hud_heart.png");
    imgHeartHalf = LoadGraph("Sprites/Tiles/Default/hud_heart_half.png");
    imgHeartEmpty = LoadGraph("Sprites/Tiles/Default/hud_heart_empty.png");
    imgCoin = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    imgPlayerIcon = LoadGraph("Sprites/Tiles/Default/hud_player_helmet_green.png");

    //�����X�v���C�g�ǂݍ���
    for (int i = 0; i < 10; ++i) {
        std::string path = "Sprites/Tiles/Default/hud_character_"
            + std::to_string(i) + ".png";
        imgDigits[i] = LoadGraph(path.c_str());
    }
}

HUD::~HUD() {
    DeleteGraph(imgHeartFull);
    DeleteGraph(imgHeartHalf);
    DeleteGraph(imgHeartEmpty);
    DeleteGraph(imgCoin);
    DeleteGraph(imgPlayerIcon);

    for (int h : imgDigits) {
        DeleteGraph(h);
    }
}

void HUD::Draw(int currentHealth, int maxHealth, int coins)
{
    int x = START_X,y =  START_Y;

    

    // �v���C���[�A�C�R��
    DrawGraph(x, y, imgPlayerIcon, TRUE);
    x += ICON_SIZE + MARGIN_ICON_HEART;

    // �n�[�g
    for (int i = 0; i < maxHealth; ++i) {
        DrawGraph(x, y, imgHeartFull, TRUE);
        x += ICON_SIZE + MARGIN_HEART;
    }

    // �R�C���A�C�R��
    x += MARGIN_HEART_COIN - MARGIN_HEART;
    DrawGraph(x, y, imgCoin, TRUE);
    x += ICON_SIZE + 4;

    // �R�C�����i�����X�v���C�g�j
    std::string s = std::to_string(coins);
    for (char c : s) {
        int d = c - '0';
        DrawGraph(x, y, imgDigits[d], TRUE);
        x += DIGIT_WIDTH + MARGIN_DIGIT;
    }
}
std::pair<int, int> HUD::GetCoinIconPos(int maxHearts) const {
    // Draw() �Ƃ܂����������v�Z�ŃA�C�R���̈ʒu��Ԃ�
    int x = START_X + ICON_SIZE + MARGIN_ICON_HEART
        + maxHearts * (ICON_SIZE + MARGIN_HEART)
        + (MARGIN_HEART_COIN - MARGIN_HEART);
    int y = START_Y;
    return { x, y };
}
