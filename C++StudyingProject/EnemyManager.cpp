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
    // 草原ステージ - 初心者向けの敵配置

    // 序盤エリア（練習用）
    AddNormalSlime(600.0f, 600.0f);
    AddNormalSlime(1000.0f, 550.0f);

    // 中盤エリア（混合）
    AddNormalSlime(1800.0f, 450.0f);
    AddSpikeSlime(2200.0f, 400.0f);
    AddNormalSlime(2600.0f, 500.0f);

    // 終盤エリア（チャレンジ）
    AddSpikeSlime(3400.0f, 350.0f);
    AddNormalSlime(3800.0f, 600.0f);
    AddNormalSlime(4200.0f, 450.0f);
    AddSpikeSlime(4800.0f, 550.0f);

    // ゴール前（最後の練習）
    AddNormalSlime(5600.0f, 500.0f);
    AddSpikeSlime(6000.0f, 450.0f);
    AddNormalSlime(6400.0f, 600.0f);
}

void EnemyManager::GenerateEnemiesForStoneStage()
{
    // 石ステージ - 中級難易度

    // 開始エリア
    AddSpikeSlime(800.0f, 650.0f);
    AddNormalSlime(1200.0f, 500.0f);

    // 岩場エリア（複数の敵が近接）
    AddNormalSlime(1600.0f, 550.0f);
    AddSpikeSlime(1800.0f, 550.0f);
    AddNormalSlime(2000.0f, 650.0f);

    // 中間エリア
    AddSpikeSlime(2800.0f, 500.0f);
    AddNormalSlime(3200.0f, 600.0f);
    AddSpikeSlime(3600.0f, 450.0f);

    // 終盤エリア（密集配置）
    AddNormalSlime(4400.0f, 650.0f);
    AddSpikeSlime(4600.0f, 500.0f);
    AddNormalSlime(4800.0f, 650.0f);
    AddSpikeSlime(5200.0f, 550.0f);

    // ゴール前
    AddSpikeSlime(5800.0f, 650.0f);
    AddNormalSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSandStage()
{
    // 砂漠ステージ - 砂の中に隠れた敵

    // オアシス近辺
    AddNormalSlime(500.0f, 600.0f);
    AddSpikeSlime(1100.0f, 550.0f);

    // 砂丘エリア
    AddSpikeSlime(1500.0f, 450.0f);
    AddNormalSlime(1900.0f, 500.0f);
    AddSpikeSlime(2300.0f, 400.0f);

    // ピラミッド近辺（危険エリア）
    AddSpikeSlime(2700.0f, 350.0f);
    AddSpikeSlime(3100.0f, 450.0f);
    AddNormalSlime(3500.0f, 550.0f);

    // 後半の砂丘
    AddNormalSlime(4000.0f, 500.0f);
    AddSpikeSlime(4400.0f, 300.0f);
    AddNormalSlime(4800.0f, 550.0f);

    // ゴール前のオアシス
    AddSpikeSlime(5400.0f, 500.0f);
    AddNormalSlime(5800.0f, 600.0f);
    AddSpikeSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSnowStage()
{
    // 雪山ステージ - 寒さで動きが鈍い設定

    // 山麓エリア
    AddNormalSlime(600.0f, 700.0f);
    AddSpikeSlime(1000.0f, 600.0f);

    // 雪の足場エリア
    AddSpikeSlime(1600.0f, 500.0f);
    AddNormalSlime(2000.0f, 550.0f);
    AddSpikeSlime(2400.0f, 400.0f);

    // 氷の洞窟エリア
    AddNormalSlime(3000.0f, 550.0f);
    AddSpikeSlime(3400.0f, 450.0f);
    AddNormalSlime(3800.0f, 600.0f);

    // 山頂付近（高難度）
    AddSpikeSlime(4400.0f, 350.0f);
    AddSpikeSlime(4800.0f, 250.0f);
    AddNormalSlime(5200.0f, 400.0f);

    // 雪原の秘密
    AddSpikeSlime(5800.0f, 300.0f);
    AddNormalSlime(6200.0f, 500.0f);
}

void EnemyManager::GenerateEnemiesForPurpleStage()
{
    // 魔法ステージ - 最高難度

    // 魔法の島序盤
    AddSpikeSlime(600.0f, 550.0f);
    AddSpikeSlime(1000.0f, 450.0f);

    // 浮遊する魔法の足場
    AddNormalSlime(1400.0f, 350.0f);
    AddSpikeSlime(1800.0f, 300.0f);
    AddSpikeSlime(2200.0f, 400.0f);

    // 魔法の橋（連続バトル）
    AddSpikeSlime(2800.0f, 450.0f);
    AddNormalSlime(3000.0f, 450.0f);
    AddSpikeSlime(3200.0f, 450.0f);

    // 高い魔法の島（チャレンジエリア）
    AddSpikeSlime(3800.0f, 300.0f);
    AddSpikeSlime(4200.0f, 250.0f);
    AddNormalSlime(4600.0f, 350.0f);

    // 最終エリア（敵が密集）
    AddSpikeSlime(5200.0f, 400.0f);
    AddNormalSlime(5400.0f, 350.0f);
    AddSpikeSlime(5600.0f, 400.0f);
    AddSpikeSlime(6000.0f, 300.0f);
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

// **完全修正版: より詳細な衝突処理**
bool EnemyManager::CheckDetailedPlayerEnemyCollision(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy || !enemy->IsActive() || enemy->IsDead()) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    float enemyX = enemy->GetX();
    float enemyY = enemy->GetY();

    // **プレイヤーと敵の当たり判定ボックス定義**
    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float ENEMY_WIDTH = 48.0f;
    const float ENEMY_HEIGHT = 56.0f;

    // **AABB衝突判定**
    float playerLeft = playerX - PLAYER_WIDTH / 2;
    float playerRight = playerX + PLAYER_WIDTH / 2;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float playerBottom = playerY + PLAYER_HEIGHT / 2;

    float enemyLeft = enemyX - ENEMY_WIDTH / 2;
    float enemyRight = enemyX + ENEMY_WIDTH / 2;
    float enemyTop = enemyY - ENEMY_HEIGHT / 2;
    float enemyBottom = enemyY + ENEMY_HEIGHT / 2;

    // **基本的な重なり判定**
    bool isOverlapping = (playerLeft < enemyRight && playerRight > enemyLeft &&
        playerTop < enemyBottom && playerBottom > enemyTop);

    if (!isOverlapping) return false;

    // **衝突の詳細分析: 踏み判定の厳密な条件**
    bool isStompingFromAbove = false;

    // **踏み判定の条件を修正（より確実に動作するように）**
    // 1. プレイヤーが敵の上方から来ている
    // 2. プレイヤーが下向きに移動している（落下中）
    // 3. プレイヤーの足元が敵の頭部付近にある
    float stompThreshold = 25.0f; // 踏み判定の許容範囲を拡大

    if (playerVelY > 0.5f && // プレイヤーが下向きに移動（しきい値を調整）
        playerY < enemyY - 5.0f && // プレイヤーが敵より上にいる
        playerBottom >= enemyTop - 5.0f && // プレイヤーの足が敵の頭に接触
        playerBottom <= enemyTop + stompThreshold) { // 踏み判定の範囲内

        isStompingFromAbove = true;
    }

    if (isStompingFromAbove) {
        // **敵の種類に応じた踏み処理**
        HandleStompInteraction(player, enemy);
    }
    else {
        // **横や下からの接触: ダメージ処理**
        HandleSideCollisionInteraction(player, enemy);
    }

    return true;
}

// **完全修正版: 踏み付け処理**
void EnemyManager::HandleStompInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    if (enemy->GetType() == EnemyBase::NORMAL_SLIME) {
        // **NormalSlime: 踏むと即座に倒す**

        // **デバッグ出力**
        OutputDebugStringA("EnemyManager: Stomping NormalSlime!\n");

        // **敵の衝突処理を呼び出し（これで敵が死ぬ）**
        enemy->OnPlayerCollision(player);

        // **プレイヤーに跳ね返り効果**
        if (player) {
            player->ApplyStompBounce(-8.0f);
        }

    }
    else if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
        // **SpikeSlime: トゲの状態をチェック**
        SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);

        // スパイクが出ているかチェック（SpikeSlime側の内部状態による）
        // この処理は実際にはSpikeSlime側のOnPlayerCollisionで処理される
        enemy->OnPlayerCollision(player);

        // **プレイヤーへのダメージ処理は実際にはGameScene側で実装**
        // ここでは衝突を検出したことを通知するのみ
    }
}

// **完全修正版: 横・下からの接触処理**
void EnemyManager::HandleSideCollisionInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    // **すべての横・下からの接触はダメージ**
    // 実際のダメージ処理はGameScene側で処理される

    // **敵固有の接触処理を呼び出し**
    enemy->OnPlayerCollision(player);
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