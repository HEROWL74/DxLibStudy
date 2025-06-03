#pragma once
#include "DxLib.h"
#include <string>
#include <vector>

class HUDSystem {
public:
    // ハートの状態
    enum HeartState {
        HEART_FULL,      // 満タン
        HEART_HALF,      // 半分
        HEART_EMPTY      // 空
    };

    HUDSystem();
    ~HUDSystem();

    void Initialize();
    void Update();
    void Draw();

    // ライフ管理
    void SetMaxLife(int maxLife) { this->maxLife = maxLife; }
    void SetCurrentLife(int currentLife);
    void AddLife(int amount);
    void SubtractLife(int amount);
    int GetCurrentLife() const { return currentLife; }
    int GetMaxLife() const { return maxLife; }

    // プレイヤーアイコン設定
    void SetPlayerCharacter(int characterIndex);
    void SetPlayerCharacter(const std::string& colorName);

    // コイン管理
    void SetCoins(int coins) { this->coins = coins; }
    void AddCoins(int amount) { coins += amount; }
    void SubtractCoins(int amount) { coins = max(0, coins - amount); }
    int GetCoins() const { return coins; }

    // HUD表示設定
    void SetPosition(int x, int y) { hudX = x; hudY = y; }
    void SetVisible(bool visible) { this->visible = visible; }
    bool IsVisible() const { return visible; }

private:
    // テクスチャハンドル
    struct HeartTextures {
        int full;
        int half;
        int empty;
    } heartTextures;

    struct PlayerIconTextures {
        int beige;
        int green;
        int pink;
        int purple;
        int yellow;
    } playerIconTextures;

    struct CoinTextures {
        int coin;
        int numbers[10]; // 0-9の数字
    } coinTextures;

    // ゲーム状態
    int maxLife;        // 最大ライフ (通常は6 = 3ハート × 2)
    int currentLife;    // 現在のライフ
    int coins;          // コイン数
    int currentPlayerCharacter; // 現在のプレイヤーキャラクター

    // HUD表示設定
    int hudX, hudY;     // HUDの基準位置
    bool visible;       // HUD表示フラグ

    // レイアウト定数（拡大版）
    static const int HEART_SIZE = 64;           // 32 → 64に拡大
    static const int PLAYER_ICON_SIZE = 80;     // 48 → 80に拡大
    static const int COIN_ICON_SIZE = 48;       // 32 → 48に拡大
    static const int NUMBER_SIZE = 36;          // 24 → 36に拡大
    static const int ELEMENT_SPACING = 20;      // 16 → 20に拡大

    // ヘルパー関数
    void LoadTextures();
    void DrawHearts();
    void DrawPlayerIcon();
    void DrawCoins();
    void DrawNumber(int number, int x, int y);
    HeartState GetHeartState(int heartIndex) const;
    int GetPlayerIconHandle() const;
    std::string GetCharacterColorName(int characterIndex) const;
};