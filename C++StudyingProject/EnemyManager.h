#pragma once
#include "DxLib.h"
#include "EnemyBase.h"
#include "NormalSlime.h"
#include "SpikeSlime.h"
#include "Bee.h"
#include "Fly.h"
#include "Ladybug.h"
#include "Saw.h"
#include "SlimeBlock.h"
#include "Player.h"
#include "StageManager.h"
#include <vector>
#include <memory>

class EnemyManager {
public:
    EnemyManager();
    ~EnemyManager();

    void Initialize();
    void Update(Player* player, StageManager* stageManager);
    void Draw(float cameraX);
    void DrawDebugInfo(float cameraX);

    // 敵の生成と管理
    void ClearAllEnemies();
    void AddEnemy(std::unique_ptr<EnemyBase> enemy);
    void AddNormalSlime(float x, float y);
    void AddSpikeSlime(float x, float y);
    void AddBee(float x, float y);
    void AddFly(float x, float y);
    void AddLadybug(float x, float y);
    void AddSaw(float x, float y);
    void AddSlimeBlock(float x, float y);

    // ステージごとの敵配置
    void GenerateEnemiesForStage(int stageIndex);
    void GenerateEnemiesForGrassStage();
    void GenerateEnemiesForStoneStage();
    void GenerateEnemiesForSandStage();
    void GenerateEnemiesForSnowStage();
    void GenerateEnemiesForPurpleStage();

    // **修正: より詳細な衝突判定システム**
    bool CheckPlayerEnemyCollisions(Player* player);
    bool CheckDetailedPlayerEnemyCollision(Player* player, EnemyBase* enemy);
    void HandleStompInteraction(Player* player, EnemyBase* enemy);
    void HandleSideCollisionInteraction(Player* player, EnemyBase* enemy);

    // **旧システム（互換性のため残存）**
    void HandlePlayerEnemyCollision(Player* player, EnemyBase* enemy);

    // 状態取得
    int GetActiveEnemyCount() const;
    int GetDeadEnemyCount() const;
    void RemoveDeadEnemies();

    // **新機能: アクティブな敵のリストを取得**
    const std::vector<std::unique_ptr<EnemyBase>>& GetEnemies() const { return enemies; }

private:
    std::vector<std::unique_ptr<EnemyBase>> enemies;

    // デバッグ用
    bool showDebugInfo;
    int totalEnemiesSpawned;
    int totalEnemiesDefeated;

    // ヘルパー関数
    void CleanupInactiveEnemies();
    bool IsEnemyOnScreen(EnemyBase* enemy, float cameraX);
    void UpdateEnemyActivation(float cameraX); // 画面外の敵は非アクティブに
};