#pragma once
#include "DxLib.h"

class HUD {
public:
    HUD();
    ~HUD();

    // currentHealth：現在HP（1刻み）、maxHealth：最大HP、coins：所持コイン数
    void Draw(int currentHealth, int maxHealth, int coins);

private:
    int imgHeartFull;
    int imgHeartHalf;
    int imgHeartEmpty;
    int imgCoin;
    int imgPlayerIcon;

    static constexpr int ICON_SIZE = 32;  // 画像の描画サイズ
    static constexpr int MARGIN_ICON_HEART = 20; // アイコンとハートの間隔
    static constexpr int MARGIN_HEART = 8;  // ハート同士の間隔
    static constexpr int MARGIN_HEART_COIN = 20; // ハートとコインの間隔
};
