#pragma once
#include "DxLib.h"
#include <string>

class GameScene
{
public:
    GameScene();
    ~GameScene();

    void Initialize(int selectedCharacter);
    void Update();
    void Draw();

    bool IsExitRequested() const { return exitRequested; }

private:
    // 画面サイズ
    static const int SCREEN_W = 1920;
    static const int SCREEN_H = 1080;

    // キャラクター色名
    enum CharacterColor {
        BEIGE = 0,
        GREEN = 1,
        PINK = 2,
        PURPLE = 3,
        YELLOW = 4
    };

    // キャラクター状態
    enum CharacterState {
        IDLE,
        WALKING,
        JUMPING,
        DUCKING
    };

    // キャラクタースプライト構造体
    struct CharacterSprites {
        int front;
        int idle;
        int walk_a;
        int walk_b;
        int jump;
        int duck;
        int hit;
        int climb_a;
        int climb_b;
    };

    // テクスチャハンドル
    int backgroundHandle;
    CharacterSprites characterSprites; // 全スプライト
    int fontHandle;

    // キャラクター情報
    int selectedCharacterIndex;
    std::string characterColorName;
    std::string characterName;

    // プレイヤー状態
    float playerX, playerY;
    float velocityX, velocityY;
    float groundY;               // 地面のY座標
    bool facingRight;           // 向いている方向（true=右、false=左）
    CharacterState currentState;
    bool onGround;              // 地面にいるかどうか

    // アニメーション
    float animationTimer;       // アニメーション用タイマー
    bool walkAnimFrame;         // walk_a(false) or walk_b(true)
    float bobPhase;            // アイドル時の上下の揺れ効果

    // 物理定数
    static constexpr float GRAVITY = 0.8f;
    static constexpr float JUMP_POWER = -18.0f;
    static constexpr float MOVE_SPEED = 5.0f;
    static constexpr float WALK_ANIM_SPEED = 0.2f; // アニメーション速度

    // 状態管理
    bool exitRequested;

    // キー入力
    bool leftPressed, rightPressed, downPressed, spacePressed;
    bool leftPressedPrev, rightPressedPrev, downPressedPrev, spacePressedPrev;
    bool escPressed, escPressedPrev;

    // ヘルパー関数
    std::string GetCharacterColorName(int index);
    std::string GetCharacterDisplayName(int index);
    void LoadAllCharacterSprites(int characterIndex);
    void UpdateInput();
    void UpdatePhysics();
    void UpdateAnimation();
    int GetCurrentSprite();
    void DrawCharacter();

    // ユーティリティ
    float Lerp(float a, float b, float t);
};