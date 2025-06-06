#include "CoinSystem.h"
#include <math.h>
#include <algorithm>
#include "SoundManager.h"

using namespace std;

CoinSystem::CoinSystem()
    : coinTexture(-1)
    , sparkleTexture(-1)
    , collectedCoinsCount(0)
{
}

CoinSystem::~CoinSystem()
{
    if (coinTexture != -1) DeleteGraph(coinTexture);
    if (sparkleTexture != -1) DeleteGraph(sparkleTexture);
}

void CoinSystem::Initialize()
{
    LoadTextures();
    collectedCoinsCount = 0;
}

void CoinSystem::LoadTextures()
{
    // �R�C���e�N�X�`����ǂݍ���
    coinTexture = LoadGraph("Sprites/Tiles/hud_coin.png");

    // ����߂����ʗp�i���݂���ꍇ�j
    // sparkleTexture = LoadGraph("Sprites/Effects/sparkle.png");
}

void CoinSystem::Update(Player* player, float hudCoinX, float hudCoinY)
{
    if (!player) return;

    // �v���C���[�̈ʒu���擾
    float playerX = player->GetX();
    float playerY = player->GetY();

    // �S�ẴR�C�����X�V
    for (auto it = coins.begin(); it != coins.end();) {
        if ((*it)->active) {
            UpdateCoin(**it, playerX, playerY, hudCoinX, hudCoinY);
            ++it;
        }
        else {
            // ��A�N�e�B�u�ȃR�C�����폜
            it = coins.erase(it);
        }
    }
}

void CoinSystem::UpdateCoin(Coin& coin, float playerX, float playerY, float hudCoinX, float hudCoinY)
{
    // �v���C���[�ƃR�C���̋������v�Z�i����p�j
    float playerDistance = GetDistance(coin.x, coin.y, playerX, playerY);

    // ��Ԃɉ���������
    switch (coin.state) {
    case COIN_IDLE:
        // �����񂹔͈͓����`�F�b�N�i�v���C���[�Ƃ̋����Ŕ���j
        if (playerDistance <= ATTRACT_DISTANCE) {
            coin.state = COIN_ATTRACTING;
            coin.attractTimer = 0.0f;
        }
        break;

    case COIN_ATTRACTING:
        // ���W�͈͓����`�F�b�N�i�v���C���[�Ƃ̋����Ŕ���j
        if (playerDistance <= COLLECTION_DISTANCE) {
            coin.state = COIN_COLLECTED;
            coin.attractTimer = 0.0f;
          
            SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);;
            collectedCoinsCount++;
        }
        break;

    case COIN_COLLECTED:
        // ���W�A�j���[�V�����i�������Ə�������������āj
        coin.attractTimer += 0.016f; // 60FPS�z��

        // HUD�ɓ��B�������`�F�b�N�i�������\���߂��Ȃ�����폜�j
        float hudDistance = GetDistance(coin.x, coin.y, hudCoinX, hudCoinY);

        if (coin.attractTimer >= 1.2f || hudDistance <= 20.0f) { // 1.2�b�o�߂܂���20px�ȓ��ɓ��B
            coin.active = false;
        }
        break;
    }

    // �����X�V
    UpdateCoinPhysics(coin, hudCoinX, hudCoinY);

    // �A�j���[�V�����X�V
    UpdateCoinAnimation(coin);
}

void CoinSystem::UpdateCoinPhysics(Coin& coin, float hudCoinX, float hudCoinY)
{
    switch (coin.state) {
    case COIN_IDLE:
        // �ʏ�̕��V����
        coin.y = coin.originalY + sinf(coin.bobPhase) * BOB_AMPLITUDE;
        break;

    case COIN_COLLECTED:
    {
        // ���W��̉��o�iHUD�̃R�C���A�C�R���Ɍ������ĉ����ړ��j
        float progress = coin.attractTimer / 1.2f; // 1.2�b�ł�����芮��
        float dirX = hudCoinX - coin.x;
        float dirY = hudCoinY - coin.y;
        float distance = sqrtf(dirX * dirX + dirY * dirY);

        if (distance > 0.1f) {
            // ���K�����ꂽ�x�N�g��
            dirX /= distance;
            dirY /= distance;

            // �����I�Ɉړ��iHUD�ɋ߂Â��قǑ����j
            float speed = 15.0f * (1.0f + progress * 3.0f);

            coin.x += dirX * speed;
            coin.y += dirY * speed;
        }

        // �t�F�[�h�A�E�g�Ɗg�����
        coin.alpha = (int)(255 * (1.0f - progress * 0.9f)); // ���L�r�L�r�ƃt�F�[�h�A�E�g
        coin.scale = 1.0f + progress * 0.3f; // �������Ɗg��
    }
    break;
    }
}

void CoinSystem::UpdateCoinAnimation(Coin& coin)
{
    // ���V�A�j���[�V����
    coin.bobPhase += BOB_SPEED;
    if (coin.bobPhase >= 2.0f * 3.14159265f) {
        coin.bobPhase -= 2.0f * 3.14159265f;
    }

    // ��]�A�j���[�V����
    coin.animationTimer += ROTATE_SPEED;
    if (coin.animationTimer >= 2.0f * 3.14159265f) {
        coin.animationTimer -= 2.0f * 3.14159265f;
    }

    // �����񂹎��̃X�P�[���A�j���[�V����
    if (coin.state == COIN_ATTRACTING) {
        float pulseScale = 1.0f + sinf(coin.animationTimer * 4.0f) * 0.1f;
        coin.scale = pulseScale;
    }
}

void CoinSystem::Draw(float cameraX)
{
    if (coinTexture == -1) return;

    for (const auto& coin : coins) {
        if (coin->active) {
            DrawCoin(*coin, cameraX);

            // ���W�G�t�F�N�g
            if (coin->state == COIN_COLLECTED) {
                DrawCollectionEffect(*coin, cameraX);
            }
        }
    }
}

void CoinSystem::DrawCoin(const Coin& coin, float cameraX)
{
    // ��ʍ��W�ɕϊ�
    int screenX = (int)(coin.x - cameraX);
    int screenY = (int)coin.y;

    // ��ʊO�Ȃ�`�悵�Ȃ��i�œK���j
    if (screenX < -COIN_SIZE || screenX > 1920 + COIN_SIZE) return;

    // �X�P�[���Ɠ����x��K�p
    int coinSize = (int)(COIN_SIZE * coin.scale);
    int offsetX = coinSize / 2;
    int offsetY = coinSize / 2;

    // �����x�ݒ�
    if (coin.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_ALPHA, coin.alpha);
    }

    // ��]���ʁi�ȈՔŁF�������̃X�P�[���ύX�ŉ�]��\���j
    float rotationScale = fabsf(cosf(coin.animationTimer));
    int rotatedWidth = (int)(coinSize * (0.3f + rotationScale * 0.7f));

    // **�R�C���`��i64px�Ɋg��j**
    DrawExtendGraph(
        screenX - offsetX, screenY - offsetY,
        screenX - offsetX + rotatedWidth, screenY - offsetY + coinSize,
        coinTexture, TRUE
    );

    // **�f�o�b�O�p�F�R�C���̓����蔻��͈͂�HUD�^�[�Q�b�g��\��**
#ifdef _DEBUG
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    DrawCircle(screenX, screenY, (int)COLLECTION_DISTANCE, GetColor(0, 255, 0), FALSE); // ���W�͈́i�΁j
    DrawCircle(screenX, screenY, (int)ATTRACT_DISTANCE, GetColor(255, 255, 0), FALSE);  // �����񂹔͈́i���j

    // HUD�^�[�Q�b�g�ʒu��\���i�Ԃ��~��j - �Œ肳�ꂽ�X�N���[�����W
    int hudTargetScreenX = 30 + 80 + 20 + 48 / 2;
    int hudTargetScreenY = 30 + 64 + 20 + 48 / 2;
    DrawLine(hudTargetScreenX - 10, hudTargetScreenY - 10, hudTargetScreenX + 10, hudTargetScreenY + 10, GetColor(255, 0, 0));
    DrawLine(hudTargetScreenX - 10, hudTargetScreenY + 10, hudTargetScreenX + 10, hudTargetScreenY - 10, GetColor(255, 0, 0));

    // HUD�܂ł̋�����\��
    if (coin.state == COIN_COLLECTED) {
        float hudWorldX = hudTargetScreenX + cameraX;
        float hudWorldY = hudTargetScreenY;
        float hudDistance = GetDistance(coin.x, coin.y, hudWorldX, hudWorldY);

        string distanceText = "Dist: " + to_string((int)hudDistance);
        DrawString(screenX - 30, screenY - 60, distanceText.c_str(), GetColor(255, 255, 0));
    }

    // ���[���h���W�ł̕����x�N�g����\��
    if (coin.state == COIN_ATTRACTING || coin.state == COIN_COLLECTED) {
        float hudWorldX = hudTargetScreenX + cameraX;
        float hudWorldY = hudTargetScreenY;
        float dirX = hudWorldX - coin.x;
        float dirY = hudWorldY - coin.y;

        // �����x�N�g������ŕ\��
        int endX = screenX + (int)(dirX * 0.1f);
        int endY = screenY + (int)(dirY * 0.1f);
        DrawLine(screenX, screenY, endX, endY, GetColor(255, 0, 255));
    }

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif

    // �����x�����Z�b�g
    if (coin.alpha < 255) {
        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void CoinSystem::DrawCollectionEffect(const Coin& coin, float cameraX)
{
    int screenX = (int)(coin.x - cameraX);
    int screenY = (int)coin.y;

    // ����߂����ʁi�������Ƃ������o�j
    float progress = coin.attractTimer / 1.2f; // 1.2�b�ł������
    int sparkleSize = (int)(25 + progress * 35); // �������Ƒ傫���Ȃ�
    int sparkleAlpha = (int)(160 * (1.0f - progress));

    if (sparkleAlpha > 0) {
        SetDrawBlendMode(DX_BLENDMODE_ADD, sparkleAlpha);

        // �����̉~�ł���߂����ʁi�������Ɖ�]�j
        for (int i = 0; i < 4; i++) {
            float angle = progress * 4.0f + i * 1.57f; // ��������]
            int sparkleX = screenX + (int)(cosf(angle) * 18);
            int sparkleY = screenY + (int)(sinf(angle) * 18);

            DrawCircle(sparkleX, sparkleY, sparkleSize / (i + 1),
                GetColor(255, 255, 150), TRUE);
        }

        // �����ɒǉ��̂���߂�
        DrawCircle(screenX, screenY, sparkleSize / 2,
            GetColor(255, 215, 0), TRUE);

        SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
    }
}

void CoinSystem::AddCoin(float x, float y)
{
    coins.push_back(std::make_unique<Coin>(x, y));
}

void CoinSystem::ClearAllCoins()
{
    coins.clear();
}

void CoinSystem::GenerateCoinsForStageIndex(int stageIndex)
{
    switch (stageIndex) {
    case 0: // Grass Stage
        GenerateCoinsForGrassStage();
        break;
    case 1: // Stone Stage
        GenerateCoinsForStoneStage();
        break;
    case 2: // Sand Stage
        GenerateCoinsForSandStage();
        break;
    case 3: // Snow Stage
        GenerateCoinsForSnowStage();
        break;
    case 4: // Purple Stage
        GenerateCoinsForPurpleStage();
        break;
    default:
        GenerateCoinsForGrassStage(); // �f�t�H���g
        break;
    }
}

void CoinSystem::GenerateCoinsForStage()
{
    // �f�t�H���g�z�u�i����݊����̂��߁j
    GenerateCoinsForGrassStage();
}

void CoinSystem::GenerateCoinsForGrassStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> grassCoins = {
        // �X�^�[�g�n�_���Ӂi�n�ʃ��x���j
        {300, 650}, {500, 600}, {700, 650}, {900, 600},

        // ���Ճv���b�g�t�H�[������
        {400, 450}, {800, 350}, {1200, 450}, {350, 500},

        // �M���b�v1���Ӂi���₷���ʒu�j
        {1000, 600}, {1400, 550}, {1300, 500}, {1600, 450},

        // ���Ղ̍����v���b�g�t�H�[��
        {2000, 400}, {2400, 350}, {2800, 400}, {2100, 500},

        // �M���b�v2�G���A
        {3200, 500}, {3600, 450}, {3100, 600}, {3800, 550},

        // �I�ՃG���A
        {4400, 600}, {4800, 550}, {5200, 600}, {5600, 500},
        {6000, 450}, {6400, 500}, {6800, 550},

        // �`�������W�R�C���i�����ꏊ�j
        {1500, 250}, {3000, 200}, {4500, 250}, {6000, 200}
    };

    for (const auto& pos : grassCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForStoneStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> stoneCoins = {
        // �X�^�[�g�n�_���Ӂi���S�Ȓn�ʃ��x���j
        {300, 700}, {600, 700}, {900, 700}, {1200, 700},

        // �Ⴂ���̎��Ӂi���₷�������j
        {500, 550}, {800, 500}, {1100, 550}, {1400, 500},

        // ���S�ȋ��̏�
        {700, 650}, {1000, 600}, {1600, 650}, {1900, 600},

        // ���ԃv���b�g�t�H�[����
        {1300, 550}, {1700, 500}, {2200, 550}, {2500, 500},

        // �΂̒ʘH�i�n�ʃ��x���j
        {2000, 700}, {2400, 700}, {2800, 700}, {3200, 700},

        // ���S�ȑ���̏�
        {2600, 600}, {3000, 550}, {3400, 600}, {3800, 550},

        // �I�ՃG���A�i���₷���ʒu�j
        {4200, 700}, {4600, 650}, {5000, 700}, {5400, 650},

        // �S�[���O�G���A
        {5800, 700}, {6200, 650}, {6600, 700},

        // �{�[�i�X�R�C���i���������ꏊ�������S�j
        {1000, 400}, {2500, 350}, {4000, 400}, {5500, 350}
    };

    for (const auto& pos : stoneCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSandStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> sandCoins = {
        // �I�A�V�X����
        {350, 650}, {750, 600}, {1150, 650}, {550, 700},

        // ���u�̒���
        {500, 450}, {1300, 400}, {2100, 450}, {900, 500},

        // �s���~�b�h�̎���
        {2800, 500}, {3200, 450}, {3600, 400}, {2600, 550},

        // �����̉B�ꂽ�ꏊ
        {1600, 550}, {2400, 500}, {4000, 550}, {1800, 600},

        // �������u
        {4400, 350}, {4800, 300}, {5200, 350}, {4600, 400},

        // ���̋�
        {5600, 600}, {6000, 550}, {6400, 600}, {5800, 650},

        // �S�[���O�̃I�A�V�X
        {6800, 650}, {7200, 600}, {7000, 550},

        // �����̂���i�`�������W�R�C���j
        {900, 200}, {2700, 150}, {4600, 200}, {6600, 250}
    };

    for (const auto& pos : sandCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSnowStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> snowCoins = {
        // �R�[
        {400, 700}, {800, 650}, {1200, 700}, {600, 750},

        // ��̑���
        {600, 550}, {1400, 500}, {2200, 550}, {1000, 600},

        // �X�̓��A��
        {1000, 450}, {1800, 400}, {2600, 450}, {1300, 500},

        // �R�̒���
        {3000, 600}, {3400, 550}, {3800, 600}, {3200, 650},

        // �Ⴞ��܂̎���
        {2400, 350}, {4200, 400}, {5000, 350}, {2600, 450},

        // �X���̋߂�
        {4600, 550}, {5400, 500}, {6200, 550}, {4800, 600},

        // �R���t��
        {5800, 300}, {6600, 250}, {7000, 300}, {6200, 400},

        // �ጴ�̔��
        {1300, 200}, {3500, 150}, {5700, 200}, {6800, 350}
    };

    for (const auto& pos : snowCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForPurpleStage()
{
    ClearAllCoins();

    std::vector<std::pair<float, float>> purpleCoins = {
        // ���@�̓��̎���
        {400, 650}, {900, 600}, {1400, 550}, {650, 700},

        // ���V���閂�@�̗�
        {700, 450}, {1200, 400}, {1700, 350}, {950, 500},

        // ���@�̋�
        {2000, 500}, {2500, 450}, {3000, 500}, {2250, 550},

        // ���@�̊K�i����
        {2200, 350}, {3400, 300}, {4600, 350}, {2800, 400},

        // �������@�̓�
        {3800, 400}, {4200, 350}, {4600, 400}, {4000, 450},

        // ���@�̒ʘH
        {5000, 550}, {5400, 500}, {5800, 550}, {5200, 600},

        // �ŏI�G���A�̖��@
        {6200, 450}, {6600, 400}, {7000, 450}, {6400, 500},

        // �B���ꂽ���@�̃R�C��
        {1100, 200}, {2800, 150}, {4400, 200}, {6400, 250}
    };

    for (const auto& pos : purpleCoins) {
        AddCoin(pos.first, pos.second);
    }
}

float CoinSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

float CoinSystem::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}