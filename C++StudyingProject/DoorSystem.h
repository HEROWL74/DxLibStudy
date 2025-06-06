#pragma once
#include "DxLib.h"
#include "Player.h"
#include "StageManager.h"
#include <string>

class DoorSystem {
public:
    // ドアの状態
    enum DoorState {
        DOOR_CLOSED,        // 閉まっている
        DOOR_OPENING,       // 開いている途中
        DOOR_OPEN,          // 完全に開いている
        DOOR_PLAYER_ENTERING // プレイヤーが入っている途中
    };

    DoorSystem();
    ~DoorSystem();

    void Initialize();
    void Update(Player* player);
    void Draw(float cameraX);

    // ドア管理
    void SetDoor(float x, float y);
    void ClearDoor();
    void OpenDoor(); // ゴール時にドアを開く
    bool IsPlayerEntering() const { return doorState == DOOR_PLAYER_ENTERING; }
    bool IsPlayerFullyEntered() const;
    bool IsDoorExists() const { return doorExists; }

    // ステージに応じたドア配置
    void PlaceDoorForStage(int stageIndex, StageManager* stageManager);

private:
    // テクスチャハンドル
    struct DoorTextures {
        int closedBottom;     // door_closed.png
        int closedTop;        // door_closed_top.png
        int openBottom;       // door_open.png
        int openTop;          // door_open_top.png
    } doorTextures;

    // ドア情報
    float doorX, doorY;         // ドア位置
    DoorState doorState;        // ドアの状態
    bool doorExists;            // ドアが存在するか

    // アニメーション
    float openingProgress;      // 開放進行度 (0.0f～1.0f)
    float enteringProgress;     // プレイヤー進入進行度 (0.0f～1.0f)
    float animationTimer;       // アニメーションタイマー

    // プレイヤー進入演出
    float playerStartX;         // プレイヤーの開始X座標
    float playerTargetX;        // プレイヤーの目標X座標
    float playerWalkSpeed;      // ドアへの歩行速度

    // 定数
    static constexpr float DOOR_WIDTH = 64.0f;          // ドアの幅
    static constexpr float DOOR_HEIGHT = 128.0f;        // ドアの高さ（2タイル分）
    static constexpr float DETECTION_WIDTH = 80.0f;     // プレイヤー検出幅
    static constexpr float OPENING_DURATION = 1.5f;     // ドア開放時間
    static constexpr float ENTERING_DURATION = 2.0f;    // プレイヤー進入時間
    static constexpr float PLAYER_WALK_SPEED = 60.0f;   // ドアへの歩行速度

    // ヘルパー関数
    void LoadTextures();
    void UpdateDoorAnimation();
    void UpdatePlayerEntering(Player* player);
    bool CheckPlayerNearDoor(Player* player);
    void StartPlayerEntering(Player* player);
    void DrawDoorSprites(float cameraX);
    void DrawDoorEffects(float cameraX);
    float GetDistance(float x1, float y1, float x2, float y2);

    // ステージに応じた地面レベル検索
    float FindGroundLevel(float x, StageManager* stageManager);
    float FindGroundLevelAccurate(float x, StageManager* stageManager); // **新追加：より正確な地面検索**
};