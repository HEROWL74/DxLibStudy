#include "HUD.h"

HUD::HUD() {
    imgHeartFull = LoadGraph("Sprites/Tiles/Default/hud_heart.png");
    imgHeartHalf = LoadGraph("Sprites/Tiles/Default/hud_heart_half.png");
    imgHeartEmpty = LoadGraph("Sprites/Tiles/Default/hud_heart_empty.png");
    imgCoin = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    imgPlayerIcon = LoadGraph("Sprites/Tiles/Default/hud_player_helmet_green.png");
}

HUD::~HUD() {
    DeleteGraph(imgHeartFull);
    DeleteGraph(imgHeartHalf);
    DeleteGraph(imgHeartEmpty);
    DeleteGraph(imgCoin);
    DeleteGraph(imgPlayerIcon);
}

void HUD::Draw(int currentHealth, int maxHealth, int coins) {
    const int startX = 20, startY = 20;
    int x = startX;

    // 1) プレイヤーアイコン
    DrawGraph(x, startY, imgPlayerIcon, TRUE);
    x += ICON_SIZE + MARGIN_ICON_HEART;

    // 2) ライフ（ハート）表示
    int totalHearts = (maxHealth + 1) / 2;
    int hp = currentHealth;
    for (int i = 0; i < totalHearts; ++i) {
        int handle;
        if (hp >= 2) {
            handle = imgHeartFull;  hp -= 2;
        }
        else if (hp == 1) {
            handle = imgHeartHalf;  hp = 0;
        }
        else {
            handle = imgHeartEmpty;
        }
        DrawGraph(x, startY, handle, TRUE);
        x += ICON_SIZE + MARGIN_HEART;
    }

    // 3) ハートとコインの間隔調整
    x += MARGIN_HEART_COIN - MARGIN_HEART;

    // 4) コインアイコン＋数値
    DrawGraph(x, startY, imgCoin, TRUE);
    x += ICON_SIZE + 4;
    DrawFormatString(x, startY + (ICON_SIZE / 4), GetColor(255, 255, 255), "%d", coins);
}
