#include "EnemyManager.h"
#include <algorithm>

EnemyManager::EnemyManager()
    : showDebugInfo(false)
    , totalEnemiesSpawned(0)
    , totalEnemiesDefeated(0)
{
}

EnemyManager::~EnemyManager()
{
    ClearAllEnemies();
}

void EnemyManager::Initialize()
{
    ClearAllEnemies();
    totalEnemiesSpawned = 0;
    totalEnemiesDefeated = 0;
}

void EnemyManager::Update(Player* player, StageManager* stageManager)
{

    // カメラ位置を取得（簡易実装）
    float cameraX = player ? player->GetX() - 960.0f : 0.0f; // 画面中央にプレイヤーを配置

    // 画面外の敵の非アクティブ化
    UpdateEnemyActivation(cameraX);

    // 敵の更新処理
    for (auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->Update(player, stageManager);
        }
    }

    // **修正: プレイヤーと敵の衝突判定を詳細に処理**
    CheckPlayerEnemyCollisions(player);

    // **重要: 死んだ敵の削除を毎フレーム実行**
    RemoveDeadEnemies();
}

void EnemyManager::Draw(float cameraX)
{
    // 敵の描画
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->Draw(cameraX);
        }
    }

    // デバッグ情報の描画
    if (showDebugInfo) {
        DrawDebugInfo(cameraX);
    }
}

void EnemyManager::DrawDebugInfo(float cameraX)
{
    // F2キーでデバッグ情報を切り替え
    static bool f2WasPressed = false;
    bool f2Pressed = CheckHitKey(KEY_INPUT_F2) != 0;

    if (f2Pressed && !f2WasPressed) {
        showDebugInfo = !showDebugInfo;
    }
    f2WasPressed = f2Pressed;

    if (!showDebugInfo) return;

    // 敵の個別デバッグ情報
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->DrawDebugInfo(cameraX);
        }
    }

    // 統計情報
    std::string statsText = "Enemies - Active: " + std::to_string(GetActiveEnemyCount()) +
        " | Spawned: " + std::to_string(totalEnemiesSpawned) +
        " | Defeated: " + std::to_string(totalEnemiesDefeated);

    DrawString(10, 10, statsText.c_str(), GetColor(255, 255, 255));
    DrawString(10, 30, "F2: Toggle Enemy Debug Info", GetColor(200, 200, 200));
}

void EnemyManager::ClearAllEnemies()
{
    enemies.clear();
    totalEnemiesSpawned = 0;
    totalEnemiesDefeated = 0;
}

void EnemyManager::AddEnemy(std::unique_ptr<EnemyBase> enemy)
{
    if (enemy) {
        enemy->Initialize();
        enemies.push_back(std::move(enemy));
        totalEnemiesSpawned++;
    }
}

void EnemyManager::AddNormalSlime(float x, float y)
{
    auto slime = std::make_unique<NormalSlime>(x, y);
    AddEnemy(std::move(slime));
}

void EnemyManager::AddSpikeSlime(float x, float y)
{
    auto spikeSlime = std::make_unique<SpikeSlime>(x, y);
    AddEnemy(std::move(spikeSlime));
}

void EnemyManager::AddBee(float x, float y)
{
    auto bee = std::make_unique<Bee>(x, y);
    AddEnemy(std::move(bee));
}

void EnemyManager::AddFly(float x, float y)
{
    auto fly = std::make_unique<Fly>(x, y);
    AddEnemy(std::move(fly));
}

void EnemyManager::AddLadybug(float x, float y)
{
    auto ladybug = std::make_unique<Ladybug>(x, y);
    AddEnemy(std::move(ladybug));
}

void EnemyManager::AddSaw(float x, float y)
{
    auto saw = std::make_unique<Saw>(x, y);
    AddEnemy(std::move(saw));
}

void EnemyManager::AddSlimeBlock(float x, float y)
{
    auto slimeBlock = std::make_unique<SlimeBlock>(x, y);
    AddEnemy(std::move(slimeBlock));
}
void EnemyManager::GenerateEnemiesForStage(int stageIndex)
{
    ClearAllEnemies();

    switch (stageIndex) {
    case 0: GenerateEnemiesForGrassStage(); break;
    case 1: GenerateEnemiesForStoneStage(); break;
    case 2: GenerateEnemiesForSandStage(); break;
    case 3: GenerateEnemiesForSnowStage(); break;
    case 4: GenerateEnemiesForPurpleStage(); break;
    default: GenerateEnemiesForGrassStage(); break;
    }
}

void EnemyManager::GenerateEnemiesForGrassStage()
{
    // 草ステージ - 初心者向けの敵配置

    // 基本エリア（練習用）
    AddNormalSlime(600.0f, 600.0f);
    AddBee(800.0f, 400.0f);  // 空中敵の導入
    AddNormalSlime(1000.0f, 550.0f);

    // 中盤エリア（混合）
    AddLadybug(1800.0f, 450.0f);  // 歩行+飛行敵
    AddSpikeSlime(2200.0f, 400.0f);
    AddFly(2400.0f, 350.0f);  // 小さく素早い敵
    AddNormalSlime(2600.0f, 500.0f);

    // 終盤エリア（チャレンジ）
    AddSaw(3400.0f, 350.0f);  // 危険な敵
    AddSlimeBlock(3600.0f, 600.0f);  // 特殊な踏みつけ敵
    AddBee(3800.0f, 300.0f);
    AddNormalSlime(4200.0f, 450.0f);
    AddSpikeSlime(4800.0f, 550.0f);

    // ゴール前（最後の練習）
    AddLadybug(5600.0f, 500.0f);
    AddFly(5800.0f, 350.0f);
    AddNormalSlime(6400.0f, 600.0f);
}

void EnemyManager::GenerateEnemiesForStoneStage()
{
    // 石ステージ - 中級難易度

    // 開始エリア
    AddSlimeBlock(800.0f, 650.0f);  // 硬い敵の導入
    AddBee(1000.0f, 400.0f);
    AddNormalSlime(1200.0f, 500.0f);

    // 岩場エリア（複数の敵が近接）
    AddSaw(1600.0f, 300.0f);  // 空中の危険
    AddSpikeSlime(1800.0f, 550.0f);
    AddLadybug(2000.0f, 650.0f);

    // 中間エリア
    AddFly(2800.0f, 250.0f);
    AddSlimeBlock(3000.0f, 600.0f);
    AddBee(3200.0f, 350.0f);
    AddSpikeSlime(3600.0f, 450.0f);

    // 終盤エリア（密集配置）
    AddSaw(4400.0f, 400.0f);
    AddSlimeBlock(4600.0f, 500.0f);
    AddLadybug(4800.0f, 650.0f);
    AddFly(5000.0f, 300.0f);
    AddSpikeSlime(5200.0f, 550.0f);

    // ゴール前
    AddBee(5800.0f, 400.0f);
    AddSaw(6000.0f, 350.0f);
    AddNormalSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSandStage()
{
    // 砂漠ステージ - 砂の中に隠れた敵

    // オアシス近辺
    AddFly(500.0f, 300.0f);  // 砂漠の虫
    AddLadybug(700.0f, 600.0f);
    AddSlimeBlock(1100.0f, 550.0f);

    // 砂丘エリア
    AddSaw(1500.0f, 250.0f);  // 空中を移動
    AddBee(1700.0f, 400.0f);
    AddNormalSlime(1900.0f, 500.0f);
    AddSpikeSlime(2300.0f, 400.0f);

    // ピラミッド近辺（危険エリア）
    AddSlimeBlock(2700.0f, 350.0f);
    AddSaw(2900.0f, 300.0f);
    AddFly(3100.0f, 250.0f);
    AddBee(3300.0f, 450.0f);
    AddLadybug(3500.0f, 550.0f);

    // 後半の砂丘
    AddSpikeSlime(4000.0f, 500.0f);
    AddSlimeBlock(4200.0f, 400.0f);
    AddSaw(4400.0f, 300.0f);
    AddFly(4600.0f, 250.0f);
    AddNormalSlime(4800.0f, 550.0f);

    // ゴール前のオアシス
    AddBee(5400.0f, 350.0f);
    AddLadybug(5600.0f, 500.0f);
    AddNormalSlime(5800.0f, 600.0f);
    AddSpikeSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSnowStage()
{
    // 雪山ステージ - 寒さで動きが鈍い設定

    // 山麓エリア
    AddSlimeBlock(600.0f, 700.0f);  // 氷のように硬い
    AddBee(800.0f, 500.0f);
    AddLadybug(1000.0f, 600.0f);

    // 雪の足場エリア
    AddSaw(1600.0f, 400.0f);
    AddFly(1800.0f, 350.0f);
    AddSlimeBlock(2000.0f, 550.0f);
    AddSpikeSlime(2400.0f, 400.0f);

    // 氷の洞窟エリア
    AddBee(3000.0f, 300.0f);
    AddLadybug(3200.0f, 550.0f);
    AddSaw(3400.0f, 250.0f);
    AddFly(3600.0f, 350.0f);
    AddNormalSlime(3800.0f, 600.0f);

    // 山頂付近（高難度）
    AddSlimeBlock(4400.0f, 350.0f);
    AddSaw(4600.0f, 200.0f);
    AddBee(4800.0f, 300.0f);
    AddSpikeSlime(5000.0f, 450.0f);
    AddLadybug(5200.0f, 400.0f);

    // 雪原の秘密
    AddFly(5800.0f, 250.0f);
    AddSlimeBlock(6000.0f, 350.0f);
    AddNormalSlime(6200.0f, 500.0f);
}

void EnemyManager::GenerateEnemiesForPurpleStage()
{
    // 魔法ステージ - 最高難度

    // 魔法の島序盤
    AddSaw(600.0f, 300.0f);
    AddSlimeBlock(800.0f, 550.0f);
    AddBee(1000.0f, 250.0f);

    // 浮遊する魔法の足場
    AddFly(1400.0f, 200.0f);  // 高速移動
    AddLadybug(1600.0f, 350.0f);
    AddSaw(1800.0f, 250.0f);
    AddSlimeBlock(2000.0f, 400.0f);
    AddSpikeSlime(2200.0f, 400.0f);

    // 魔法の橋（連続バトル）
    AddBee(2800.0f, 300.0f);
    AddFly(3000.0f, 200.0f);
    AddSaw(3200.0f, 250.0f);
    AddSlimeBlock(3400.0f, 450.0f);
    AddLadybug(3600.0f, 350.0f);

    // 高い魔法の島（チャレンジエリア）
    AddSaw(3800.0f, 150.0f);
    AddBee(4000.0f, 200.0f);
    AddFly(4200.0f, 180.0f);
    AddSlimeBlock(4400.0f, 300.0f);
    AddSpikeSlime(4600.0f, 350.0f);

    // 最終エリア（敵が密集）
    AddSaw(5200.0f, 250.0f);
    AddBee(5400.0f, 300.0f);
    AddSlimeBlock(5600.0f, 400.0f);
    AddFly(5800.0f, 200.0f);
    AddLadybug(6000.0f, 350.0f);
    AddSpikeSlime(6200.0f, 300.0f);
    AddNormalSlime(6400.0f, 450.0f);
}

bool EnemyManager::CheckPlayerEnemyCollisions(Player* player)
{
    if (!player) return false;

    bool collisionOccurred = false;

    for (auto& enemy : enemies) {
        if (enemy && enemy->IsActive() && !enemy->IsDead()) {
            if (CheckDetailedPlayerEnemyCollision(player, enemy.get())) {
                collisionOccurred = true;
            }
        }
    }

    return collisionOccurred;
}

bool EnemyManager::CheckDetailedPlayerEnemyCollision(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy || !enemy->IsActive() || enemy->IsDead()) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();
    float enemyX = enemy->GetX();
    float enemyY = enemy->GetY();

    // 基本的な重なり判定
    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float ENEMY_WIDTH = 48.0f;
    const float ENEMY_HEIGHT = 56.0f;

    bool isOverlapping = (playerX - PLAYER_WIDTH / 2 < enemyX + ENEMY_WIDTH / 2 &&
        playerX + PLAYER_WIDTH / 2 > enemyX - ENEMY_WIDTH / 2 &&
        playerY - PLAYER_HEIGHT / 2 < enemyY + ENEMY_HEIGHT / 2 &&
        playerY + PLAYER_HEIGHT / 2 > enemyY - ENEMY_HEIGHT / 2);

    if (!isOverlapping) return false;

    // **修正: 踏みつけ判定を改善**
    bool isStompFromAbove = (
        playerVelY > 1.0f &&                    // 下向きの速度が十分
        playerY < enemyY - 15.0f &&             // プレイヤーが敵より十分上
        playerY + PLAYER_HEIGHT / 2 >= enemyY - ENEMY_HEIGHT / 2 - 8.0f  // 足が敵の頭付近
        );

    if (isStompFromAbove) {
        // **踏みつけ処理**
        HandleStompInteraction(player, enemy);

        char debugMsg[256];
        sprintf_s(debugMsg, "EnemyManager: Stomp detected! Player Y:%.1f, Enemy Y:%.1f, VelY:%.1f\n",
            playerY, enemyY, playerVelY);
        OutputDebugStringA(debugMsg);

        return true;
    }
    else {
        // **横からの衝突処理**
        HandleSideCollisionInteraction(player, enemy);

        OutputDebugStringA("EnemyManager: Side collision detected!\n");

        return true;
    }
}

void EnemyManager::HandleStompInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    if (enemy->GetType() == EnemyBase::NORMAL_SLIME) {
        // **NormalSlime: 踏みつけアニメーションを開始（即座に倒さない）**

        // **敵の衝突処理を呼び出し（アニメーション開始）**
        enemy->OnPlayerCollision(player);

        // **プレイヤーに跳ね返り効果**
        player->ApplyStompBounce(-8.0f);

        OutputDebugStringA("EnemyManager: NormalSlime stomp - starting flatten animation!\n");
    }
    else if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
        // **SpikeSlime: トゲの状態をチェック**
        SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);

        if (spikeSlime->AreSpikesOut()) {
            // **トゲが出ている場合はダメージを受ける**
            OutputDebugStringA("EnemyManager: Stepped on spikes - player takes damage!\n");
            HandleSideCollisionInteraction(player, enemy);
        }
        else {
            // **トゲが引っ込んでいる場合は踏みつけ成功**
            enemy->OnPlayerCollision(player);
            player->ApplyStompBounce(-8.0f);

            OutputDebugStringA("EnemyManager: Successfully stomped SpikeSlime!\n");
        }
    }
    else {
        // **他の敵タイプの場合は通常の踏みつけ処理**
        enemy->TakeDamage(100);
        player->ApplyStompBounce(-8.0f);

        char debugMsg[256];
        sprintf_s(debugMsg, "EnemyManager: Stomped enemy type %d\n", (int)enemy->GetType());
        OutputDebugStringA(debugMsg);
    }
}

void EnemyManager::HandleSideCollisionInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    // **プレイヤーが無敵状態の場合はダメージなし**
    if (player->IsInvulnerable()) {
        OutputDebugStringA("EnemyManager: Player is invulnerable - no damage!\n");
        return;
    }

    // **敵の種類に応じてダメージ量を決定**
    int damageAmount = 1; // デフォルト

    if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
        SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);
        if (spikeSlime->AreSpikesOut()) {
            damageAmount = 2; // トゲダメージは大きい
        }
    }

    // **ノックバック方向を計算**
    float knockbackDirection = (player->GetX() > enemy->GetX()) ? 1.0f : -1.0f;

    // **敵固有の衝突処理を呼び出し（アニメーションなど）**
    enemy->OnPlayerCollision(player);

    // **重要: プレイヤーにダメージとノックバックを直接適用**
    // Player::TakeDamageでノックバック処理
    player->TakeDamage(damageAmount, knockbackDirection);

    // **GameSceneにダメージを通知する方法が必要**
    // この部分は後で修正が必要

    char debugMsg[256];
    sprintf_s(debugMsg, "EnemyManager: Side collision - damage: %d, knockback: %.2f\n",
        damageAmount, knockbackDirection);
    OutputDebugStringA(debugMsg);
}

void EnemyManager::HandlePlayerEnemyCollision(Player* player, EnemyBase* enemy)
{
    // **この関数は非推奨 - CheckDetailedPlayerEnemyCollisionを使用**
    if (!player || !enemy) return;

    enemy->OnPlayerCollision(player);

    float playerY = player->GetY();
    float enemyTop = enemy->GetY() - EnemyBase::ENEMY_HEIGHT / 2;

    // プレイヤーが敵の上から踏んだ場合
    if (playerY < enemyTop - 10 && player->GetVelocityY() > 0) {
        // スパイクスライムの特別処理
        if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
            SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);
            // トゲが出ている場合のダメージ処理は Player側で実装
        }

        // 通常の踏みつけ処理（敵を倒す）
        // プレイヤーの跳ね返り効果も Player側で実装
    }
    else {
        // 横からの接触によるダメージ処理
        // 実際のダメージ処理は Player側で実装
    }
}

int EnemyManager::GetActiveEnemyCount() const
{
    int count = 0;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive() && !enemy->IsDead()) {
            count++;
        }
    }
    return count;
}

int EnemyManager::GetDeadEnemyCount() const
{
    int count = 0;
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsDead()) {
            count++;
        }
    }
    return count;
}

// **完全修正版: 死んだ敵の削除**
void EnemyManager::RemoveDeadEnemies()
{
    auto oldSize = enemies.size();

    // **死んだ敵または非アクティブな敵を削除**
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [this](const std::unique_ptr<EnemyBase>& enemy) {
                if (!enemy) {
                    return true; // nullptrは削除
                }

                // **死亡または非アクティブな敵を削除**
                if (enemy->IsDead() || !enemy->IsActive() || enemy->GetState() == EnemyBase::DEAD) {
                    totalEnemiesDefeated++;

                    // **デバッグ出力**
                    OutputDebugStringA("EnemyManager: Removing dead enemy!\n");

                    return true; // 削除対象
                }

                return false; // 保持
            }),
        enemies.end()
    );

    // **削除された敵の数をデバッグ出力**
    if (enemies.size() != oldSize) {
        char debugMsg[256];
        sprintf_s(debugMsg, "EnemyManager: Removed %d enemies. Remaining: %d\n",
            (int)(oldSize - enemies.size()), (int)enemies.size());
        OutputDebugStringA(debugMsg);
    }
}

void EnemyManager::CleanupInactiveEnemies()
{
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [](const std::unique_ptr<EnemyBase>& enemy) {
                return !enemy || !enemy->IsActive();
            }),
        enemies.end()
    );
}

bool EnemyManager::IsEnemyOnScreen(EnemyBase* enemy, float cameraX)
{
    if (!enemy) return false;

    float enemyX = enemy->GetX();
    float screenX = enemyX - cameraX;

    // 画面幅より少し広めの範囲で判定（敵の予備動作等を考慮）
    return (screenX >= -200.0f && screenX <= 1920.0f + 200.0f);
}

void EnemyManager::UpdateEnemyActivation(float cameraX)
{
    for (auto& enemy : enemies) {
        if (!enemy) continue;

        bool onScreen = IsEnemyOnScreen(enemy.get(), cameraX);

        // 画面外に出た敵は一時的に非アクティブに（メモリ節約）
        // ただし、完全に削除はしない（再度画面に入った際に復活）
        if (!onScreen && enemy->IsActive()) {
            // 敵が画面外に出た場合の処理
            // 必要に応じて一時停止やスリープ状態にする
        }
        else if (onScreen && !enemy->IsActive() && !enemy->IsDead()) {
            // 敵が画面に入った場合の復活処理
            enemy->SetActive(true);
        }
    }
}