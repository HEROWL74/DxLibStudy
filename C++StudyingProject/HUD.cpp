#include "HUD.h"
#include <string>

HUD::HUD() {
    imgHeartFull = LoadGraph("Sprites/Tiles/Default/hud_heart.png");
    imgHeartHalf = LoadGraph("Sprites/Tiles/Default/hud_heart_half.png");
    imgHeartEmpty = LoadGraph("Sprites/Tiles/Default/hud_heart_empty.png");
    imgCoin = LoadGraph("Sprites/Tiles/Default/hud_coin.png");
    imgPlayerIcon = LoadGraph("Sprites/Tiles/Default/hud_player_helmet_green.png");

    // 数字スプライト読み込み
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
    int x = START_X, y = START_Y;

    // プレイヤーアイコン
    DrawGraph(x, y, imgPlayerIcon, TRUE);
    x += ICON_SIZE + MARGIN_ICON_HEART;

    // ハート（半ハート単位 currentHealth, maxHealth）
    int numHearts = maxHealth / 2;      // 例: 6/2 = 3 ハート
    int hp = currentHealth;             // HP（半ハート単位）

    for (int i = 0; i < numHearts; ++i) {
        if (hp >= 2) {
            // HPが2以上 → 満タンハート（元のコードが正しかった）
            DrawGraph(x, y, imgHeartFull, TRUE);
            hp -= 2;
        }
        else if (hp == 1) {
            // HPが1 → 半分ハート
            DrawGraph(x, y, imgHeartHalf, TRUE);
            hp = 0;
        }
        else {
            // HPが0 → 空ハート（元のコードが正しかった）
            DrawGraph(x, y, imgHeartEmpty, TRUE);
        }
        x += ICON_SIZE + MARGIN_HEART;
    }

    // コインアイコン
    x += MARGIN_HEART_COIN - MARGIN_HEART;
    DrawGraph(x, y, imgCoin, TRUE);
    x += ICON_SIZE + 4;

    // コイン数（数字スプライト）
    std::string s = std::to_string(coins);
    for (char c : s) {
        int d = c - '0';
        DrawGraph(x, y, imgDigits[d], TRUE);
        x += DIGIT_WIDTH + MARGIN_DIGIT;
    }
}

std::pair<int, int> HUD::GetCoinDigitPos(int maxHearts, int coinCount) const {
    int x = START_X + ICON_SIZE + MARGIN_ICON_HEART
        + maxHearts * (ICON_SIZE + MARGIN_HEART)
        + (MARGIN_HEART_COIN - MARGIN_HEART)
        + ICON_SIZE + 10; // コインアイコンの右側

    // 桁数に応じて右に移動
    int digitCount = std::to_string(coinCount).size();
    x += (DIGIT_WIDTH + MARGIN_DIGIT) * (digitCount - 1);

    int y = START_Y;
    return { x, y };
}