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

    // �G�̐����ƊǗ�
    void ClearAllEnemies();
    void AddEnemy(std::unique_ptr<EnemyBase> enemy);
    void AddNormalSlime(float x, float y);
    void AddSpikeSlime(float x, float y);
    void AddBee(float x, float y);
    void AddFly(float x, float y);
    void AddLadybug(float x, float y);
    void AddSaw(float x, float y);
    void AddSlimeBlock(float x, float y);

    // �X�e�[�W���Ƃ̓G�z�u
    void GenerateEnemiesForStage(int stageIndex);
    void GenerateEnemiesForGrassStage();
    void GenerateEnemiesForStoneStage();
    void GenerateEnemiesForSandStage();
    void GenerateEnemiesForSnowStage();
    void GenerateEnemiesForPurpleStage();

    // **�C��: ���ڍׂȏՓ˔���V�X�e��**
    bool CheckPlayerEnemyCollisions(Player* player);
    bool CheckDetailedPlayerEnemyCollision(Player* player, EnemyBase* enemy);
    void HandleStompInteraction(Player* player, EnemyBase* enemy);
    void HandleSideCollisionInteraction(Player* player, EnemyBase* enemy);

    // **���V�X�e���i�݊����̂��ߎc���j**
    void HandlePlayerEnemyCollision(Player* player, EnemyBase* enemy);

    // ��Ԏ擾
    int GetActiveEnemyCount() const;
    int GetDeadEnemyCount() const;
    void RemoveDeadEnemies();

    // **�V�@�\: �A�N�e�B�u�ȓG�̃��X�g���擾**
    const std::vector<std::unique_ptr<EnemyBase>>& GetEnemies() const { return enemies; }

private:
    std::vector<std::unique_ptr<EnemyBase>> enemies;

    // �f�o�b�O�p
    bool showDebugInfo;
    int totalEnemiesSpawned;
    int totalEnemiesDefeated;

    // �w���p�[�֐�
    void CleanupInactiveEnemies();
    bool IsEnemyOnScreen(EnemyBase* enemy, float cameraX);
    void UpdateEnemyActivation(float cameraX); // ��ʊO�̓G�͔�A�N�e�B�u��
};