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

    // **�C��: �X�e�[�W�̒n�`��u���b�N�Ɣ��Ȃ��ʒu�ɔz�u**
    std::vector<std::pair<float, float>> grassCoins = {
        // �󒆂̕��V�R�C���i�n�`�u���b�N��������z�u�j
        {400, 400}, {600, 350}, {900, 300},      // �����G���A���
        {1300, 250}, {1700, 200}, {2100, 300},  // ���ԃG���A���
        {2800, 250}, {3200, 180}, {3600, 220},  // ����x�G���A
        {4200, 300}, {4800, 200}, {5200, 250},  // �I�ՃG���A
        {5800, 180}, {6200, 220}, {6600, 160},  // �S�[���O�G���A

        // �`�������W�R�C���i���ɍ����ʒu�j
        {1500, 150}, {3000, 120}, {4500, 140}, {6000, 100}
    };

    for (const auto& pos : grassCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForStoneStage()
{
    ClearAllCoins();

    // **�C��: �΃X�e�[�W�̒n�`��������z�u**
    std::vector<std::pair<float, float>> stoneCoins = {
        // ��̌��Ԃ̈��S�ȏꏊ
        {500, 380}, {800, 320}, {1100, 280},    // �����G���A
        {1600, 250}, {2000, 200}, {2400, 300},  // ���Ԃ̊��
        {3000, 220}, {3500, 160}, {3900, 240},  // �������
        {4400, 280}, {4900, 200}, {5300, 180},  // �΂̓�
        {5800, 300}, {6200, 150}, {6600, 200},  // �I�ՃG���A

        // �{�[�i�X�R�C���i�����j
        {1200, 150}, {2800, 100}, {4200, 120}, {5600, 90}
    };

    for (const auto& pos : stoneCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSandStage()
{
    ClearAllCoins();

    // **�C��: �����X�e�[�W�̃s���~�b�h�⍻�u��������z�u**
    std::vector<std::pair<float, float>> sandCoins = {
        // �I�A�V�X���ӂƃs���~�b�h����
        {400, 350}, {700, 300}, {1000, 250},    // �I�A�V�X����
        {1400, 280}, {1800, 200}, {2200, 320},  // ���u�̊�
        {2700, 180}, {3100, 150}, {3500, 220},  // �s���~�b�h����
        {4000, 260}, {4400, 180}, {4800, 300},  // ���̒J
        {5300, 200}, {5700, 150}, {6100, 240},  // �I�Ղ̍��u

        // �����̕�i�B���ꂽ�R�C���j
        {1100, 120}, {2900, 80}, {4600, 100}, {6400, 60}
    };

    for (const auto& pos : sandCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForSnowStage()
{
    ClearAllCoins();

    // **�C��: ��R�̒n�`����������S�Ȕz�u**
    std::vector<std::pair<float, float>> snowCoins = {
        // �ጴ�ƕX�̑���̊�
        {500, 380}, {900, 320}, {1300, 260},    // �R�[
        {1800, 280}, {2200, 200}, {2600, 240},  // ��̑������
        {3200, 180}, {3600, 140}, {4000, 220},  // �R���t��
        {4500, 260}, {4900, 180}, {5300, 300},  // �Ⴞ��܎���
        {5800, 160}, {6200, 200}, {6600, 120},  // �ጴ�̔閧

        // �X�͂̕�
        {1400, 100}, {3400, 60}, {5100, 80}, {6800, 40}
    };

    for (const auto& pos : snowCoins) {
        AddCoin(pos.first, pos.second);
    }
}

void CoinSystem::GenerateCoinsForPurpleStage()
{
    ClearAllCoins();

    // **�C��: ���@�X�e�[�W�̕��V������������@�I�z�u**
    std::vector<std::pair<float, float>> purpleCoins = {
        // ���V���閂�@�̃R�C��
        {500, 320}, {900, 280}, {1300, 220},    // ���@�̓�����
        {1800, 200}, {2200, 160}, {2600, 260},  // ���@�̋��t��
        {3200, 180}, {3600, 120}, {4000, 240},  // �������@�̓�
        {4500, 200}, {4900, 140}, {5300, 280},  // ���@�̓�
        {5800, 160}, {6200, 100}, {6600, 180},  // �ŏI�G���A

        // �B���ꂽ���@�̃R�C��
        {1200, 80}, {3000, 40}, {4800, 60}, {6400, 20}
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