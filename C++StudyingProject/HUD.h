#pragma once
#include "DxLib.h"
#include <array>
class HUD {
public:
    HUD();
    ~HUD();

    // currentHealth：現在HP（1刻み）、maxHealth：最大HP、coins：所持コイン数
    void Draw(int currentHealth, int maxHealth, int coins);
    
    // コインアイコンの描画位置を取得（効果のターゲット座標）
    std::pair<int, int> GetCoinDigitPos(int maxHearts, int coinCount) const;
private:
    int imgHeartFull;
    int imgHeartHalf;
    int imgHeartEmpty;
    int imgCoin;
    int imgPlayerIcon;
    

    //数字スプライト
    std::array<int, 10> imgDigits;
    static constexpr int START_X = 20;
    static constexpr int START_Y = 20;
    static constexpr int DIGIT_WIDTH = 24; //数字の画像の幅
    static constexpr int DIGIT_HEIGHT = 32; //数字の画像の高さ
    static constexpr int MARGIN_DIGIT = 4;//数字同士の間隔

    static constexpr int ICON_SIZE = 32;  // 画像の描画サイズ
    static constexpr int MARGIN_ICON_HEART = 30; // アイコンとハートの間隔
    static constexpr int MARGIN_HEART = 16;  // ハート同士の間隔
    static constexpr int MARGIN_HEART_COIN = 30; // ハートとコインの間隔
};
