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
    // �J�����ʒu���擾�i�ȈՎ����j
    float cameraX = player ? player->GetX() - 960.0f : 0.0f; // ��ʒ����Ƀv���C���[��z�u

    // ��ʊO�̓G�̔�A�N�e�B�u��
    UpdateEnemyActivation(cameraX);

    // �G�̍X�V����
    for (auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->Update(player, stageManager);
        }
    }

    // **�C��: �v���C���[�ƓG�̏Փ˔�����ڍׂɏ���**
    CheckPlayerEnemyCollisions(player);

    // **�d�v: ���񂾓G�̍폜�𖈃t���[�����s**
    RemoveDeadEnemies();
}

void EnemyManager::Draw(float cameraX)
{
    // �G�̕`��
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->Draw(cameraX);
        }
    }

    // �f�o�b�O���̕`��
    if (showDebugInfo) {
        DrawDebugInfo(cameraX);
    }
}

void EnemyManager::DrawDebugInfo(float cameraX)
{
    // F2�L�[�Ńf�o�b�O����؂�ւ�
    static bool f2WasPressed = false;
    bool f2Pressed = CheckHitKey(KEY_INPUT_F2) != 0;

    if (f2Pressed && !f2WasPressed) {
        showDebugInfo = !showDebugInfo;
    }
    f2WasPressed = f2Pressed;

    if (!showDebugInfo) return;

    // �G�̌ʃf�o�b�O���
    for (const auto& enemy : enemies) {
        if (enemy && enemy->IsActive()) {
            enemy->DrawDebugInfo(cameraX);
        }
    }

    // ���v���
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
    // �����X�e�[�W - ���S�Ҍ����̓G�z�u

    // ���ՃG���A�i���K�p�j
    AddNormalSlime(600.0f, 600.0f);
    AddNormalSlime(1000.0f, 550.0f);

    // ���ՃG���A�i�����j
    AddNormalSlime(1800.0f, 450.0f);
    AddSpikeSlime(2200.0f, 400.0f);
    AddNormalSlime(2600.0f, 500.0f);

    // �I�ՃG���A�i�`�������W�j
    AddSpikeSlime(3400.0f, 350.0f);
    AddNormalSlime(3800.0f, 600.0f);
    AddNormalSlime(4200.0f, 450.0f);
    AddSpikeSlime(4800.0f, 550.0f);

    // �S�[���O�i�Ō�̗��K�j
    AddNormalSlime(5600.0f, 500.0f);
    AddSpikeSlime(6000.0f, 450.0f);
    AddNormalSlime(6400.0f, 600.0f);
}

void EnemyManager::GenerateEnemiesForStoneStage()
{
    // �΃X�e�[�W - ������Փx

    // �J�n�G���A
    AddSpikeSlime(800.0f, 650.0f);
    AddNormalSlime(1200.0f, 500.0f);

    // ���G���A�i�����̓G���ߐځj
    AddNormalSlime(1600.0f, 550.0f);
    AddSpikeSlime(1800.0f, 550.0f);
    AddNormalSlime(2000.0f, 650.0f);

    // ���ԃG���A
    AddSpikeSlime(2800.0f, 500.0f);
    AddNormalSlime(3200.0f, 600.0f);
    AddSpikeSlime(3600.0f, 450.0f);

    // �I�ՃG���A�i���W�z�u�j
    AddNormalSlime(4400.0f, 650.0f);
    AddSpikeSlime(4600.0f, 500.0f);
    AddNormalSlime(4800.0f, 650.0f);
    AddSpikeSlime(5200.0f, 550.0f);

    // �S�[���O
    AddSpikeSlime(5800.0f, 650.0f);
    AddNormalSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSandStage()
{
    // �����X�e�[�W - ���̒��ɉB�ꂽ�G

    // �I�A�V�X�ߕ�
    AddNormalSlime(500.0f, 600.0f);
    AddSpikeSlime(1100.0f, 550.0f);

    // ���u�G���A
    AddSpikeSlime(1500.0f, 450.0f);
    AddNormalSlime(1900.0f, 500.0f);
    AddSpikeSlime(2300.0f, 400.0f);

    // �s���~�b�h�ߕӁi�댯�G���A�j
    AddSpikeSlime(2700.0f, 350.0f);
    AddSpikeSlime(3100.0f, 450.0f);
    AddNormalSlime(3500.0f, 550.0f);

    // �㔼�̍��u
    AddNormalSlime(4000.0f, 500.0f);
    AddSpikeSlime(4400.0f, 300.0f);
    AddNormalSlime(4800.0f, 550.0f);

    // �S�[���O�̃I�A�V�X
    AddSpikeSlime(5400.0f, 500.0f);
    AddNormalSlime(5800.0f, 600.0f);
    AddSpikeSlime(6200.0f, 550.0f);
}

void EnemyManager::GenerateEnemiesForSnowStage()
{
    // ��R�X�e�[�W - �����œ������݂��ݒ�

    // �R�[�G���A
    AddNormalSlime(600.0f, 700.0f);
    AddSpikeSlime(1000.0f, 600.0f);

    // ��̑���G���A
    AddSpikeSlime(1600.0f, 500.0f);
    AddNormalSlime(2000.0f, 550.0f);
    AddSpikeSlime(2400.0f, 400.0f);

    // �X�̓��A�G���A
    AddNormalSlime(3000.0f, 550.0f);
    AddSpikeSlime(3400.0f, 450.0f);
    AddNormalSlime(3800.0f, 600.0f);

    // �R���t�߁i����x�j
    AddSpikeSlime(4400.0f, 350.0f);
    AddSpikeSlime(4800.0f, 250.0f);
    AddNormalSlime(5200.0f, 400.0f);

    // �ጴ�̔閧
    AddSpikeSlime(5800.0f, 300.0f);
    AddNormalSlime(6200.0f, 500.0f);
}

void EnemyManager::GenerateEnemiesForPurpleStage()
{
    // ���@�X�e�[�W - �ō���x

    // ���@�̓�����
    AddSpikeSlime(600.0f, 550.0f);
    AddSpikeSlime(1000.0f, 450.0f);

    // ���V���閂�@�̑���
    AddNormalSlime(1400.0f, 350.0f);
    AddSpikeSlime(1800.0f, 300.0f);
    AddSpikeSlime(2200.0f, 400.0f);

    // ���@�̋��i�A���o�g���j
    AddSpikeSlime(2800.0f, 450.0f);
    AddNormalSlime(3000.0f, 450.0f);
    AddSpikeSlime(3200.0f, 450.0f);

    // �������@�̓��i�`�������W�G���A�j
    AddSpikeSlime(3800.0f, 300.0f);
    AddSpikeSlime(4200.0f, 250.0f);
    AddNormalSlime(4600.0f, 350.0f);

    // �ŏI�G���A�i�G�����W�j
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

// **���S�C����: ���ڍׂȏՓˏ���**
bool EnemyManager::CheckDetailedPlayerEnemyCollision(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy || !enemy->IsActive() || enemy->IsDead()) return false;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    float enemyX = enemy->GetX();
    float enemyY = enemy->GetY();

    // **�v���C���[�ƓG�̓����蔻��{�b�N�X��`**
    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float ENEMY_WIDTH = 48.0f;
    const float ENEMY_HEIGHT = 56.0f;

    // **AABB�Փ˔���**
    float playerLeft = playerX - PLAYER_WIDTH / 2;
    float playerRight = playerX + PLAYER_WIDTH / 2;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float playerBottom = playerY + PLAYER_HEIGHT / 2;

    float enemyLeft = enemyX - ENEMY_WIDTH / 2;
    float enemyRight = enemyX + ENEMY_WIDTH / 2;
    float enemyTop = enemyY - ENEMY_HEIGHT / 2;
    float enemyBottom = enemyY + ENEMY_HEIGHT / 2;

    // **��{�I�ȏd�Ȃ蔻��**
    bool isOverlapping = (playerLeft < enemyRight && playerRight > enemyLeft &&
        playerTop < enemyBottom && playerBottom > enemyTop);

    if (!isOverlapping) return false;

    // **�Փ˂̏ڍו���: ���ݔ���̌����ȏ���**
    bool isStompingFromAbove = false;

    // **���ݔ���̏������C���i���m���ɓ��삷��悤�Ɂj**
    // 1. �v���C���[���G�̏�����痈�Ă���
    // 2. �v���C���[���������Ɉړ����Ă���i�������j
    // 3. �v���C���[�̑������G�̓����t�߂ɂ���
    float stompThreshold = 25.0f; // ���ݔ���̋��e�͈͂��g��

    if (playerVelY > 0.5f && // �v���C���[���������Ɉړ��i�������l�𒲐��j
        playerY < enemyY - 5.0f && // �v���C���[���G����ɂ���
        playerBottom >= enemyTop - 5.0f && // �v���C���[�̑����G�̓��ɐڐG
        playerBottom <= enemyTop + stompThreshold) { // ���ݔ���͈͓̔�

        isStompingFromAbove = true;
    }

    if (isStompingFromAbove) {
        // **�G�̎�ނɉ��������ݏ���**
        HandleStompInteraction(player, enemy);
    }
    else {
        // **���≺����̐ڐG: �_���[�W����**
        HandleSideCollisionInteraction(player, enemy);
    }

    return true;
}

// **���S�C����: ���ݕt������**
void EnemyManager::HandleStompInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    if (enemy->GetType() == EnemyBase::NORMAL_SLIME) {
        // **NormalSlime: ���ނƑ����ɓ|��**

        // **�f�o�b�O�o��**
        OutputDebugStringA("EnemyManager: Stomping NormalSlime!\n");

        // **�G�̏Փˏ������Ăяo���i����œG�����ʁj**
        enemy->OnPlayerCollision(player);

        // **�v���C���[�ɒ��˕Ԃ����**
        if (player) {
            player->ApplyStompBounce(-8.0f);
        }

    }
    else if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
        // **SpikeSlime: �g�Q�̏�Ԃ��`�F�b�N**
        SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);

        // �X�p�C�N���o�Ă��邩�`�F�b�N�iSpikeSlime���̓�����Ԃɂ��j
        // ���̏����͎��ۂɂ�SpikeSlime����OnPlayerCollision�ŏ��������
        enemy->OnPlayerCollision(player);

        // **�v���C���[�ւ̃_���[�W�����͎��ۂɂ�GameScene���Ŏ���**
        // �����ł͏Փ˂����o�������Ƃ�ʒm����̂�
    }
}

// **���S�C����: ���E������̐ڐG����**
void EnemyManager::HandleSideCollisionInteraction(Player* player, EnemyBase* enemy)
{
    if (!player || !enemy) return;

    // **���ׂẲ��E������̐ڐG�̓_���[�W**
    // ���ۂ̃_���[�W������GameScene���ŏ��������

    // **�G�ŗL�̐ڐG�������Ăяo��**
    enemy->OnPlayerCollision(player);
}

void EnemyManager::HandlePlayerEnemyCollision(Player* player, EnemyBase* enemy)
{
    // **���̊֐��͔񐄏� - CheckDetailedPlayerEnemyCollision���g�p**
    if (!player || !enemy) return;

    enemy->OnPlayerCollision(player);

    float playerY = player->GetY();
    float enemyTop = enemy->GetY() - EnemyBase::ENEMY_HEIGHT / 2;

    // �v���C���[���G�̏ォ�瓥�񂾏ꍇ
    if (playerY < enemyTop - 10 && player->GetVelocityY() > 0) {
        // �X�p�C�N�X���C���̓��ʏ���
        if (enemy->GetType() == EnemyBase::SPIKE_SLIME) {
            SpikeSlime* spikeSlime = static_cast<SpikeSlime*>(enemy);
            // �g�Q���o�Ă���ꍇ�̃_���[�W������ Player���Ŏ���
        }

        // �ʏ�̓��݂������i�G��|���j
        // �v���C���[�̒��˕Ԃ���ʂ� Player���Ŏ���
    }
    else {
        // ������̐ڐG�ɂ��_���[�W����
        // ���ۂ̃_���[�W������ Player���Ŏ���
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

// **���S�C����: ���񂾓G�̍폜**
void EnemyManager::RemoveDeadEnemies()
{
    auto oldSize = enemies.size();

    // **���񂾓G�܂��͔�A�N�e�B�u�ȓG���폜**
    enemies.erase(
        std::remove_if(enemies.begin(), enemies.end(),
            [this](const std::unique_ptr<EnemyBase>& enemy) {
                if (!enemy) {
                    return true; // nullptr�͍폜
                }

                // **���S�܂��͔�A�N�e�B�u�ȓG���폜**
                if (enemy->IsDead() || !enemy->IsActive() || enemy->GetState() == EnemyBase::DEAD) {
                    totalEnemiesDefeated++;

                    // **�f�o�b�O�o��**
                    OutputDebugStringA("EnemyManager: Removing dead enemy!\n");

                    return true; // �폜�Ώ�
                }

                return false; // �ێ�
            }),
        enemies.end()
    );

    // **�폜���ꂽ�G�̐����f�o�b�O�o��**
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

    // ��ʕ���菭���L�߂͈̔͂Ŕ���i�G�̗\�����쓙���l���j
    return (screenX >= -200.0f && screenX <= 1920.0f + 200.0f);
}

void EnemyManager::UpdateEnemyActivation(float cameraX)
{
    for (auto& enemy : enemies) {
        if (!enemy) continue;

        bool onScreen = IsEnemyOnScreen(enemy.get(), cameraX);

        // ��ʊO�ɏo���G�͈ꎞ�I�ɔ�A�N�e�B�u�Ɂi�������ߖ�j
        // �������A���S�ɍ폜�͂��Ȃ��i�ēx��ʂɓ������ۂɕ����j
        if (!onScreen && enemy->IsActive()) {
            // �G����ʊO�ɏo���ꍇ�̏���
            // �K�v�ɉ����Ĉꎞ��~��X���[�v��Ԃɂ���
        }
        else if (onScreen && !enemy->IsActive() && !enemy->IsDead()) {
            // �G����ʂɓ������ꍇ�̕�������
            enemy->SetActive(true);
        }
    }
}