#include "CoinSystem.h"
#include <math.h>
#include <algorithm>

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
        // �z���񂹔͈͓����`�F�b�N�i�v���C���[�Ƃ̋����Ŕ���j
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
            collectedCoinsCount++;
        }
        break;

    case COIN_COLLECTED:
        // ���W�A�j���[�V�����i������莞�Ԃ������āj
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
        // ���W���̉��o�iHUD�̃R�C���A�C�R���Ɍ������ĉ����ړ��j
        float progress = coin.attractTimer / 1.2f; // 1.2�b�ł�����芮��
        float dirX = hudCoinX - coin.x;
        float dirY = hudCoinY - coin.y;
        float distance = sqrtf(dirX * dirX + dirY * dirY);

        if (distance > 0.1f) {
            // ���K�����ꂽ�����x�N�g��
            dirX /= distance;
            dirY /= distance;

            // �����I�Ɉړ��iHUD�ɋ߂Â��قǑ����j
            float speed = 15.0f * (1.0f + progress * 3.0f);

            coin.x += dirX * speed;
            coin.y += dirY * speed;
        }

        // �t�F�[�h�A�E�g�Ɗg�����
        coin.alpha = (int)(255 * (1.0f - progress * 0.9f)); // ��苭���t�F�[�h�A�E�g
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

    // �z���񂹎��̃X�P�[���A�j���[�V����
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
    DrawCircle(screenX, screenY, (int)ATTRACT_DISTANCE, GetColor(255, 255, 0), FALSE);  // �z���񂹔͈́i���j

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

void CoinSystem::GenerateCoinsForStage()
{
    // �X�e�[�W�ɓK���ɃR�C����z�u�i�g�����ꂽ�X�e�[�W�p�j
    ClearAllCoins();

    // ��{�I�Ȕz�u�p�^�[���i�v���C���[���ʂ�₷���ʒu�ɔz�u�j
    std::vector<std::pair<float, float>> coinPositions = {
        // ���ՃG���A�i�n�ʋ߂��Ŏ��₷���j
        {400, 700}, {600, 650}, {800, 700}, {1000, 650}, {1200, 700},

        // ���ՃG���A1
        {1400, 650}, {1600, 600}, {1800, 650}, {2000, 600}, {2200, 650},

        // ���ՃG���A2
        {2400, 700}, {2600, 650}, {2800, 700}, {3000, 650}, {3200, 700},

        // ���ՃG���A3
        {3400, 650}, {3600, 600}, {3800, 650}, {4000, 600}, {4200, 650},

        // �I�ՃG���A1
        {4400, 700}, {4600, 650}, {4800, 700}, {5000, 650}, {5200, 700},

        // �I�ՃG���A2
        {5400, 650}, {5600, 600}, {5800, 650}, {6000, 600}, {6200, 650},

        // �ŏI�G���A
        {6400, 700}, {6600, 650}, {6800, 700}, {7000, 650}, {7200, 700},

        // �v���b�g�t�H�[����i�������߁j
        {1000, 500}, {1800, 450}, {2600, 500}, {3400, 450}, {4200, 500},
        {5000, 450}, {5800, 500}, {6600, 450},

        // �{�[�i�X�R�C���i�����ꏊ�A�`�������W�p�j
        {900, 400}, {1500, 300}, {2100, 350}, {2700, 300}, {3300, 350},
        {3900, 300}, {4500, 350}, {5100, 300}, {5700, 350}, {6300, 300},

        // �������₷���ʒu
        {500, 600}, {1100, 550}, {1700, 600}, {2300, 550}, {2900, 600},
        {3500, 550}, {4100, 600}, {4700, 550}, {5300, 600}, {5900, 550},
        {6500, 600}
    };

    // �R�C����z�u
    for (const auto& pos : coinPositions) {
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