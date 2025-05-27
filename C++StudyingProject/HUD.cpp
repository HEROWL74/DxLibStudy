#include "HUD.h"
#include <string>
HUD::HUD() {
    imgHeartFull = LoadGraph("Sprites/Tiles/Default/hud_heart.png");
    imgHeartHalf = LoadGraph("Sprites/Tiles/Default/hud_heart_half.png");
    imgHeartEmpty = LoadGraph("Sprites/Tiles/Default/hud_heart_empty.png");
    imgCoin = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    imgPlayerIcon = LoadGraph("Sprites/Tiles/Default/hud_player_helmet_green.png");

    //数字スプライト読み込み
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

void HUD::Draw(int currentHealth, int maxHealth = 3, int coins) {
    const int startX = 20, startY = 20;
    int x = startX;

    // 1) プレイヤーアイコン
    DrawGraph(x, startY, imgPlayerIcon, TRUE);
    x += ICON_SIZE + MARGIN_ICON_HEART;

    // 2) ハートを maxHearts 個並べる
    for (int i = 0; i < maxHealth; ++i) {
        DrawGraph(x, startY, imgHeartFull, TRUE);
        x += ICON_SIZE + MARGIN_HEART;
    }

    // 3) ハートとコインの間隔調整
    x += MARGIN_HEART_COIN - MARGIN_HEART;

    {
        std::string s = std::to_string(coins);
        for (char c : s) {
            int d = c - '0';
            DrawGraph(x, startY, imgDigits[d], TRUE);
            x += DIGIT_WIDTH + MARGIN_DIGIT;
        }
    }
}
