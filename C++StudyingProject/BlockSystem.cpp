#include "BlockSystem.h"
#include "SoundManager.h"
#include <math.h>
#include <algorithm>
#include "Player.h"
using namespace std;

BlockSystem::BlockSystem()
    : coinBlockActiveTexture(-1)
    , coinBlockEmptyTexture(-1)
    , brickBlockTexture(-1)
    , coinsFromBlocks(0)
{
    for (int i = 0; i < 4; i++) {
        brickFragmentTextures[i] = -1;
    }
}

BlockSystem::~BlockSystem()
{
    if (coinBlockActiveTexture != -1) DeleteGraph(coinBlockActiveTexture);
    if (coinBlockEmptyTexture != -1) DeleteGraph(coinBlockEmptyTexture);
    if (brickBlockTexture != -1) DeleteGraph(brickBlockTexture);

    for (int i = 0; i < 4; i++) {
        if (brickFragmentTextures[i] != -1) DeleteGraph(brickFragmentTextures[i]);
    }
}

void BlockSystem::Initialize()
{
    LoadTextures();
    CreateBrickFragmentTextures();
    coinsFromBlocks = 0;
}

void BlockSystem::LoadTextures()
{
    // �u���b�N�e�N�X�`���̓ǂݍ���
    coinBlockActiveTexture = LoadGraph("Sprites/Tiles/block_coin_active.png");
    coinBlockEmptyTexture = LoadGraph("Sprites/Tiles/block_coin.png");
    brickBlockTexture = LoadGraph("Sprites/Tiles/block_empty.png");

    // �f�o�b�O�o��
    if (coinBlockActiveTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_coin_active.png\n");
    }
    if (coinBlockEmptyTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_coin.png\n");
    }
    if (brickBlockTexture == -1) {
        OutputDebugStringA("BlockSystem: Failed to load block_empty.png\n");
    }
}

void BlockSystem::CreateBrickFragmentTextures()
{
    if (brickBlockTexture == -1) return;

    // �����K�u���b�N��4���������j�Ѓe�N�X�`�����쐬
    int originalWidth, originalHeight;
    GetGraphSize(brickBlockTexture, &originalWidth, &originalHeight);

    int fragmentWidth = originalWidth / 2;
    int fragmentHeight = originalHeight / 2;

    for (int i = 0; i < 4; i++) {
        // �j�Зp�̃e�N�X�`����DerivationGraph�ō쐬
        int srcX = (i % 2) * fragmentWidth;
        int srcY = (i / 2) * fragmentHeight;

        brickFragmentTextures[i] = DerivationGraph(
            srcX, srcY,                    // �؂�o���J�n�ʒu
            fragmentWidth, fragmentHeight, // �؂�o���T�C�Y
            brickBlockTexture             // ���摜
        );

        if (brickFragmentTextures[i] == -1) {
            OutputDebugStringA("BlockSystem: Failed to create brick fragment texture\n");
        }
    }

    OutputDebugStringA("BlockSystem: Created brick fragment textures using DerivationGraph\n");
}

void BlockSystem::Update(Player* player)
{
    if (!player) return;

    // �S�u���b�N�̍X�V
    for (auto& block : blocks) {
        if (block->state != DESTROYED) {
            UpdateBlock(*block, player);
        }
    }

    // �j�Ђ̍X�V
    UpdateFragments();
}

void BlockSystem::UpdateBlock(Block& block, Player* player)
{
    // **�C��: ��萳�m�ȉ�����̏Փ˔�����g�p**
    if (!block.wasHit && CheckPlayerHitFromBelowImproved(block, player)) {
        block.wasHit = true;

        switch (block.type) {
        case COIN_BLOCK:
            HandleCoinBlockHit(block);
            break;
        case BRICK_BLOCK:
            HandleBrickBlockHit(block);
            break;
        }
    }

    // �o�E���X�A�j���[�V�����̍X�V
    if (block.bounceTimer > 0.0f) {
        block.bounceTimer -= 0.016f;

        if (block.bounceTimer <= 0.0f) {
            block.bounceTimer = 0.0f;
            block.bounceAnimation = 0.0f;
        }
        else {
            float progress = 1.0f - (block.bounceTimer / BOUNCE_DURATION);
            block.bounceAnimation = sinf(progress * 3.14159265f) * BOUNCE_HEIGHT;
        }
    }

    // **�C��: �q�b�g�t���O�̃��Z�b�g���������P**
    if (block.wasHit && !CheckPlayerHitFromBelowImproved(block, player)) {
        float distance = GetDistance(block.x, block.y, player->GetX(), player->GetY());
        if (distance > HIT_DETECTION_SIZE * 2.0f) {
            block.wasHit = false;
        }
    }
}
void BlockSystem::UpdateFragments()
{
    for (auto it = fragments.begin(); it != fragments.end();) {
        auto& fragment = *it;

        // �����X�V
        fragment->x += fragment->velocityX;
        fragment->y += fragment->velocityY;
        fragment->velocityY += FRAGMENT_GRAVITY; // �d�͓K�p

        // **�n�ʂƂ̊ȈՃo�E���X����**
        const float GROUND_LEVEL = 800.0f; // �n�ʃ��x���i�K�X�����j
        if (fragment->y >= GROUND_LEVEL && fragment->velocityY > 0 && !fragment->bounced) {
            fragment->y = GROUND_LEVEL;
            fragment->velocityY *= -0.4f; // 40%�̔����W��
            fragment->velocityX *= 0.8f;  // �n�ʖ��C�ŉ����x����
            fragment->bounced = true;

            // �o�E���X��͉�]���x������
            fragment->rotationSpeed *= 0.6f;
        }

        // **��C��R�̓K�p**
        fragment->velocityX *= 0.995f; // ���X�ɉ����x������

        // ��]�X�V
        fragment->rotation += fragment->rotationSpeed;

        // �������Ԃ̌���
        fragment->life -= 0.016f; // 60FPS�z��

        // �������Ԃ��؂ꂽ��폜
        if (fragment->life <= 0.0f) {
            it = fragments.erase(it);
        }
        else {
            ++it;
        }
    }
}

void BlockSystem::HandleCoinBlockHit(Block& block)
{
    if (block.state == ACTIVE) {
        // �R�C���u���b�N���A�N�e�B�u�����ɕύX
        block.state = EMPTY;
        block.textureHandle = coinBlockEmptyTexture;

        // �o�E���X�A�j���[�V�����J�n
        block.bounceTimer = BOUNCE_DURATION;

        // **�C��: �R�C���l�����m���Ɏ��s**
        coinsFromBlocks++;

        // �T�E���h�Đ�
        SoundManager::GetInstance().PlaySE(SoundManager::SFX_COIN);

        // **�ڍׂȃf�o�b�O�o��**
        char debugMsg[128];
        sprintf_s(debugMsg, "BlockSystem: COIN BLOCK HIT! Total coins from blocks: %d\n", coinsFromBlocks);
        OutputDebugStringA(debugMsg);

        // **�����m�F�̃f�o�b�O**
        OutputDebugStringA("BlockSystem: Coin count increased successfully!\n");
    }
    else {
        // ���ɋ�̃u���b�N��@�����ꍇ
        OutputDebugStringA("BlockSystem: Hit empty coin block (no effect)\n");
    }
}

void BlockSystem::HandleBrickBlockHit(Block& block)
{
    // �����K�u���b�N��j��
    block.state = DESTROYED;

    // �j�Ђ𐶐�
    CreateBrickFragments(block.x, block.y);

    // �T�E���h�Đ��i�u���b�N�j�󉹁j
    // SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK); // �K�v�ɉ����Ēǉ�

    // �f�o�b�O�o��
    OutputDebugStringA("BlockSystem: Brick block destroyed!\n");
}

void BlockSystem::CreateBrickFragments(float blockX, float blockY)
{
    // 4�̔j�Ђ��쐬�i����A�E��A�����A�E���j
    for (int i = 0; i < 4; i++) {
        // �j�Ђ̏����ʒu�i�u���b�N����4�����ʒu�j
        float fragmentX = blockX + (i % 2) * (BLOCK_SIZE / 2) + (BLOCK_SIZE / 4);
        float fragmentY = blockY + (i / 2) * (BLOCK_SIZE / 2) + (BLOCK_SIZE / 4);

        // ��莩�R�Ȕj�Ђ̔�юU������v�Z
        float baseVelX = (i % 2 == 0) ? -1.0f : 1.0f;  // ���E����
        float baseVelY = (i / 2 == 0) ? -1.5f : -0.8f; // �㉺�����i��̔j�Ђ���荂����ԁj

        // �����_���v�f��ǉ�
        float randomFactorX = (rand() % 100 - 50) * 0.01f; // -0.5 ~ 0.5
        float randomFactorY = (rand() % 50) * 0.01f;       // 0.0 ~ 0.5�i������̂݁j

        float velocityX = baseVelX * (2.0f + randomFactorX);
        float velocityY = baseVelY - randomFactorY;

        // �j�Ђ�ǉ�
        if (brickFragmentTextures[i] != -1) {
            auto fragment = std::make_unique<BlockFragment>(
                fragmentX, fragmentY, velocityX, velocityY, brickFragmentTextures[i]
            );

            // **��莩�R�ȉ�]���x��ݒ�**
            fragment->rotationSpeed = (rand() % 40 - 20) * 0.05f; // -1.0 ~ 1.0 rad/frame

            fragments.push_back(std::move(fragment));
        }
    }

    // **�u���b�N�j�󎞂̃G�t�F�N�g��**
    // SoundManager::GetInstance().PlaySE(SoundManager::SFX_BREAK);

    OutputDebugStringA("BlockSystem: Created 4 brick fragments with improved physics\n");
}

void BlockSystem::Draw(float cameraX)
{
    // �u���b�N�̕`��
    for (const auto& block : blocks) {
        if (block->state != DESTROYED) {
            DrawBlock(*block, cameraX);
        }
    }

    // �j�Ђ̕`��
    DrawFragments(cameraX);
}

void BlockSystem::DrawBlock(const Block& block, float cameraX)
{
    // ��ʍ��W�ɕϊ�
    int screenX = (int)(block.x - cameraX);
    int screenY = (int)(block.y - block.bounceAnimation);

    // ��ʊO�Ȃ�`�悵�Ȃ��i�œK���j
    if (screenX < -BLOCK_SIZE || screenX > 1920 + BLOCK_SIZE) return;

    // �e�N�X�`���n���h��������
    int textureHandle = -1;
    switch (block.type) {
    case COIN_BLOCK:
        textureHandle = (block.state == ACTIVE) ? coinBlockActiveTexture : coinBlockEmptyTexture;
        break;
    case BRICK_BLOCK:
        textureHandle = brickBlockTexture;
        break;
    }

    // �u���b�N�`��
    if (textureHandle != -1) {
        DrawExtendGraph(
            screenX, screenY,
            screenX + (int)BLOCK_SIZE, screenY + (int)BLOCK_SIZE,
            textureHandle, TRUE
        );
    }

#ifdef _DEBUG
    // �f�o�b�O�p�����蔻��\��
    SetDrawBlendMode(DX_BLENDMODE_ALPHA, 50);
    DrawBox(screenX, screenY, screenX + (int)BLOCK_SIZE, screenY + (int)BLOCK_SIZE,
        GetColor(255, 0, 0), FALSE);
    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
#endif
}

void BlockSystem::DrawFragments(float cameraX)
{
    for (const auto& fragment : fragments) {
        // ��ʍ��W�ɕϊ�
        int screenX = (int)(fragment->x - cameraX);
        int screenY = (int)fragment->y;

        // ��ʊO�Ȃ�`�悵�Ȃ�
        if (screenX < -64 || screenX > 1920 + 64) continue;

        // �����x�𐶑����ԂɊ�Â��Đݒ�
        float lifeRatio = fragment->life / FRAGMENT_LIFE;
        int alpha = (int)(255 * lifeRatio);

        if (alpha > 0 && fragment->textureHandle != -1) {
            SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);

            // �j�ЃT�C�Y�i���̃u���b�N��1/2�j
            int fragmentSize = 32;

            // **��]�`��iDerivationGraph�ō쐬�����j�Ѓe�N�X�`������]�j**
            if (fragment->rotation != 0.0f) {
                // ��]�̒��S�_
                int centerX = screenX + fragmentSize / 2;
                int centerY = screenY + fragmentSize / 2;

                // ��]�`��iDxLib��DrawRotaGraph�����̏����j
                DrawRotaGraph(
                    centerX, centerY,                    // �`�撆�S
                    1.0,                                 // �g�嗦
                    fragment->rotation,                  // ��]�p�x�i���W�A���j
                    fragment->textureHandle,            // DerivationGraph�ō쐬�����e�N�X�`��
                    TRUE                                // �����F����
                );
            }
            else {
                // �ʏ�`��i��]�Ȃ��j
                DrawExtendGraph(
                    screenX, screenY,
                    screenX + fragmentSize, screenY + fragmentSize,
                    fragment->textureHandle, TRUE
                );
            }

            SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);

            // **�f�o�b�O�F�j�Ђ̋O����\��**
#ifdef _DEBUG
// �j�Ђ̑��x�x�N�g������ŕ\��
            int velocityEndX = screenX + (int)(fragment->velocityX * 10);
            int velocityEndY = screenY + (int)(fragment->velocityY * 10);
            DrawLine(screenX + fragmentSize / 2, screenY + fragmentSize / 2,
                velocityEndX, velocityEndY, GetColor(255, 0, 0));

            // �������Ԃ𐔒l�ŕ\��
            string lifeText = to_string(fragment->life).substr(0, 3);
            DrawString(screenX, screenY - 20, lifeText.c_str(), GetColor(255, 255, 0));
#endif
        }
    }
}

bool BlockSystem::CheckPlayerHitFromBelow(const Block& block, Player* player)
{
    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // �v���C���[��������Ɉړ����Ă���ꍇ�̂ݔ���
    if (playerVelY >= 0) return false;

    // X���̏d�Ȃ蔻��
    const float PLAYER_WIDTH = 80.0f;
    bool xOverlap = (playerX + PLAYER_WIDTH / 2 > block.x &&
        playerX - PLAYER_WIDTH / 2 < block.x + BLOCK_SIZE);

    if (!xOverlap) return false;

    // Y���̈ʒu�֌W����i�v���C���[���u���b�N�̉��ɂ���j
    const float PLAYER_HEIGHT = 100.0f;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float blockBottom = block.y + BLOCK_SIZE;

    // �v���C���[�̓����u���b�N�̉��ʕt�߂ɂ���
    float distance = abs(playerTop - blockBottom);

    return (distance <= 20.0f); // 20�s�N�Z���ȓ��Ŕ���
}

// **�V�ǉ��F�v���C���[�ƃu���b�N�̓����蔻��**
bool BlockSystem::CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight)
{
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        if (CheckAABBCollision(playerX - playerWidth / 2, playerY - playerHeight / 2, playerWidth, playerHeight,
            block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {
            return true;
        }
    }
    return false;
}

// **�V�ǉ��F�v���C���[�Ƃ̏Փˏ���**
void BlockSystem::HandlePlayerCollision(Player* player, float newX, float newY)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        // �V�����ʒu�ł̓����蔻��
        if (CheckAABBCollision(newX - PLAYER_WIDTH / 2, newY - PLAYER_HEIGHT / 2, PLAYER_WIDTH, PLAYER_HEIGHT,
            block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {
            ResolveCollision(player, *block, newX, newY);
        }
    }
}

// **�V�ǉ��FAABB�����蔻��**
bool BlockSystem::CheckAABBCollision(float x1, float y1, float w1, float h1,
    float x2, float y2, float w2, float h2)
{
    return (x1 < x2 + w2 &&
        x1 + w1 > x2 &&
        y1 < y2 + h2 &&
        y1 + h1 > y2);
}

void BlockSystem::ResolveCollision(Player* player, const Block& block, float playerX, float playerY)
{
    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float currentX = player->GetX();
    float currentY = player->GetY();
    float velX = player->GetVelocityX();
    float velY = player->GetVelocityY();

    // �v���C���[�̋��E
    float playerLeft = playerX - PLAYER_WIDTH / 2;
    float playerRight = playerX + PLAYER_WIDTH / 2;
    float playerTop = playerY - PLAYER_HEIGHT / 2;
    float playerBottom = playerY + PLAYER_HEIGHT / 2;

    // �u���b�N�̋��E
    float blockLeft = block.x;
    float blockRight = block.x + BLOCK_SIZE;
    float blockTop = block.y;
    float blockBottom = block.y + BLOCK_SIZE;

    // �d�Ȃ�̌v�Z
    float overlapLeft = playerRight - blockLeft;
    float overlapRight = blockRight - playerLeft;
    float overlapTop = playerBottom - blockTop;
    float overlapBottom = blockBottom - playerTop;

    // �ŏ��̏d�Ȃ�������āA���̕����ɉ����Ԃ�
    float minOverlapX = min(overlapLeft, overlapRight);
    float minOverlapY = min(overlapTop, overlapBottom);

    if (minOverlapX < minOverlapY) {
        // �������̏Փ�
        if (overlapLeft < overlapRight) {
            // �E���獶�֏Փ�
            player->SetPosition(blockLeft - PLAYER_WIDTH / 2 - 1, currentY);
        }
        else {
            // ������E�֏Փ�
            player->SetPosition(blockRight + PLAYER_WIDTH / 2 + 1, currentY);
        }

        // �������̑��x�����Z�b�g
        player->SetVelocityX(0.0f);
    }
    else {
        // �c�����̏Փ�
        if (overlapTop < overlapBottom) {
            // **�������֏Փˁi�v���C���[���u���b�N�̏�ɏ��j**
            player->SetPosition(currentX, blockTop - PLAYER_HEIGHT / 2 - 1);
            player->SetVelocityY(0.0f);
            player->SetOnGround(true);

            // **���n��̏�Ԑݒ�**
            if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
                if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                    player->SetState(Player::WALKING);
                }
                else {
                    player->SetState(Player::IDLE);
                }
            }

            OutputDebugStringA("BlockSystem: Player landed on block via collision resolution!\n");
        }
        else {
            // **�ォ�牺�֏Փˁi�v���C���[���u���b�N�̉��ɂԂ���j**
            player->SetPosition(currentX, blockBottom + PLAYER_HEIGHT / 2 + 1);
            player->SetVelocityY(0.0f);

            // �W�����v���ɓV��ɂԂ������ꍇ�͗�����Ԃ�
            if (player->GetState() == Player::JUMPING) {
                player->SetState(Player::FALLING);
            }

            OutputDebugStringA("BlockSystem: Player hit block from below via collision resolution!\n");
        }
    }
}
// **�V�ǉ��F�u���b�N���ő̂��ǂ����̔���**
bool BlockSystem::IsBlockSolid(const Block& block)
{
    // �j�󂳂ꂽ�u���b�N�͌ő̂ł͂Ȃ�
    if (block.state == DESTROYED) return false;

    // �R�C���u���b�N�͋�ɂȂ��Ă��ő�
    return true;
}

void BlockSystem::AddCoinBlock(float x, float y)
{
    auto block = std::make_unique<Block>(x, y, COIN_BLOCK);
    block->textureHandle = coinBlockActiveTexture;
    blocks.push_back(std::move(block));
}

void BlockSystem::AddBrickBlock(float x, float y)
{
    auto block = std::make_unique<Block>(x, y, BRICK_BLOCK);
    block->textureHandle = brickBlockTexture;
    blocks.push_back(std::move(block));
}

void BlockSystem::ClearAllBlocks()
{
    blocks.clear();
    fragments.clear();
    coinsFromBlocks = 0;
}

void BlockSystem::GenerateBlocksForStageIndex(int stageIndex)
{
    switch (stageIndex) {
    case 0: // Grass Stage
        GenerateBlocksForGrassStage();
        break;
    case 1: // Stone Stage
        GenerateBlocksForStoneStage();
        break;
    case 2: // Sand Stage
        GenerateBlocksForSandStage();
        break;
    case 3: // Snow Stage
        GenerateBlocksForSnowStage();
        break;
    case 4: // Purple Stage
        GenerateBlocksForPurpleStage();
        break;
    default:
        GenerateBlocksForGrassStage(); // �f�t�H���g
        break;
    }
}

void BlockSystem::GenerateBlocksForGrassStage()
{
    ClearAllBlocks();

    // **�C��: �X�e�[�W�̃v���b�g�t�H�[���Ɣ��Ȃ��ʒu�Ƀu���b�N�z�u**
    // �R�C���u���b�N�̔z�u�i�󒆂̈��S�ȏꏊ�j
    std::vector<std::pair<float, float>> coinBlocks = {
        {320, 480},   // �����G���A���i�O���b�h5���j
        {960, 420},   // ���ԃG���A�i�O���b�h15���j
        {1920, 360},  // �M���b�v�G���A�i�O���b�h30���j
        {3200, 400},  // �㔼�G���A�i�O���b�h50���j
        {4480, 320},  // �I�ՃG���A�i�O���b�h70���j
        {5760, 380}   // �S�[���O�i�O���b�h90���j
    };

    // �����K�u���b�N�̔z�u�i��Q���Ƃ��ċ@�\�j
    std::vector<std::pair<float, float>> brickBlocks = {
        {640, 500},   // �����G���A�̏�Q��
        {1280, 440},  // ���ԃG���A
        {2560, 380},  // �M���b�v2�G���A
        {3840, 420},  // �㔼�G���A
        {5120, 340},  // �I�ՃG���A
        {6400, 400}   // �ŏI�G���A
    };

    // �u���b�N��z�u
    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }

    OutputDebugStringA("BlockSystem: Generated blocks for Grass Stage (Fixed positioning)\n");
}

void BlockSystem::GenerateBlocksForStoneStage()
{
    ClearAllBlocks();

    // **�C��: �΃X�e�[�W�̒n�`���l�������z�u**
    std::vector<std::pair<float, float>> coinBlocks = {
        {480, 460},   // �΂̌���
        {1152, 380},  // ���̏��
        {2304, 420},  // �΂̓��̏�
        {3456, 340},  // �������̏�
        {4608, 380},  // �΂̒J�̏�
        {5760, 300}   // �I�Ղ̊��
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {768, 480},   // �������
        {1920, 400},  // ���Ԋ��
        {3072, 360},  // �����
        {4224, 400},  // �΂̓�
        {5376, 320},  // �I�Պ��
        {6528, 380}   // �ŏI���
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForSandStage()
{
    ClearAllBlocks();

    // **�C��: �����X�e�[�W�̃s���~�b�h�ƍ��u��������z�u**
    std::vector<std::pair<float, float>> coinBlocks = {
        {448, 440},   // �I�A�V�X����
        {1344, 360},  // ���u�̌���
        {2688, 400},  // �s���~�b�h��
        {4032, 320},  // ���̒J���
        {5376, 380},  // ���u���
        {6720, 300}   // �I�Ս���
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {896, 460},   // ���u��Q
        {2240, 380},  // �s���~�b�h����
        {3584, 340},  // ���̓�
        {4928, 400},  // ���u��
        {6272, 320},  // �I�Ս��u
        {7104, 380}   // �ŏI����
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForSnowStage()
{
    ClearAllBlocks();

    // **�C��: ��R�̒n�`��������z�u**
    std::vector<std::pair<float, float>> coinBlocks = {
        {640, 420},   // �R�[�G���A
        {1408, 340},  // ��̑�����
        {2816, 380},  // �R���G���A
        {4224, 300},  // �X�̓G���A
        {5632, 360},  // �ጴ�G���A
        {6976, 280}   // �ŏI�R��
    };

    std::vector<std::pair<float, float>> brickBlocks = {
        {1024, 440},  // ��R��Q
        {2432, 360},  // �X�̓�
        {3840, 320},  // �Ⴞ��܎���
        {5248, 380},  // �ጴ��Q
        {6656, 300},  // �I�Ր�R
        {7680, 340}   // �ŏI�ጴ
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

void BlockSystem::GenerateBlocksForPurpleStage()
{
    ClearAllBlocks();

    // **�C��: ���@�X�e�[�W�̕��V������������@�I�z�u**
    std::vector<std::pair<float,float>> coinBlocks = {
        {768, 380},   // ���@�̓�����
        {1728, 320},  // ���@�̋����
        {3456, 360},  // �������@�����
        {5184, 280},  // ���@�̓����
        {6144, 340},  // �ŏI���@�G���A
        {7296, 260}   // �S�[���O���@���
    };


    std::vector<std::pair<float, float>> brickBlocks = {
        {1152, 400},  // ���@��Q
        {2880, 340},  // ���@�̎���
        {4608, 300},  // ���x���@��Q
        {5760, 360},  // ���@�̓���Q
        {6912, 280},  // �I�Ֆ��@��Q
        {7680, 320}   // �ŏI���@����
    };

    for (const auto& pos : coinBlocks) {
        AddCoinBlock(pos.first, pos.second);
    }

    for (const auto& pos : brickBlocks) {
        AddBrickBlock(pos.first, pos.second);
    }
}

float BlockSystem::GetDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

void BlockSystem::CheckAndResolvePlayerCollisions(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float velX = player->GetVelocityX();
    float velY = player->GetVelocityY();

    // **1. �������̏Փˏ���**
    if (fabsf(velX) > 0.1f) {
        float newX = playerX + velX;

        for (const auto& block : blocks) {
            if (!IsBlockSolid(*block)) continue;

            // �������̏Փ˃`�F�b�N
            if (CheckAABBCollision(newX - PLAYER_WIDTH / 2, playerY - PLAYER_HEIGHT / 2,
                PLAYER_WIDTH, PLAYER_HEIGHT,
                block->x, block->y, BLOCK_SIZE, BLOCK_SIZE)) {

                // ������Փ˂����ꍇ�̏���
                if (velX > 0) {
                    // �E�Ɉړ����A�u���b�N�̍����ɏՓ�
                    player->SetPosition(block->x - PLAYER_WIDTH / 2 - 1, playerY);
                }
                else {
                    // ���Ɉړ����A�u���b�N�̉E���ɏՓ�
                    player->SetPosition(block->x + BLOCK_SIZE + PLAYER_WIDTH / 2 + 1, playerY);
                }
                player->SetVelocityX(0.0f);
                break;
            }
        }
    }

    // **2. �������̈ړ��i���n�����j**
    if (velY > 0.0f && !player->IsOnGround()) {
        HandleBlockLandingStable(player);
    }

    // **3. ������̈ړ��i�u���b�N���ʂւ̏Փˁj**
    else if (velY < 0.0f) {
        HandleBlockCeilingCollision(player);
    }

    // **4. �n�ʏ�Ԃ̈��艻�`�F�b�N**
    else if (player->IsOnGround()) {
        StabilizeGroundState(player);
    }
}

void BlockSystem::HandleBlockLandingStable(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float LANDING_TOLERANCE = 15.0f; // **��芰�e�Ȕ���**

    float playerX = player->GetX();
    float playerY = player->GetY();
    float velY = player->GetVelocityY();

    // **�������Ɉړ������n�ʂɂ��Ȃ��ꍇ�̂ݏ���**
    if (velY <= 0.0f || player->IsOnGround()) return;

    float footY = playerY + PLAYER_HEIGHT / 2;
    float nearestBlockTop = -1.0f;
    float minDistance = 999.0f;

    // **������5�_�ł��m���Ƀ`�F�b�N**
    float checkPoints[] = {
        playerX - PLAYER_WIDTH / 2 + 8,    // ���[
        playerX - PLAYER_WIDTH / 4,        // �����
        playerX,                         // ����
        playerX + PLAYER_WIDTH / 4,        // �E���
        playerX + PLAYER_WIDTH / 2 - 8     // �E�[
    };

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **��茵����X�����̏d�Ȃ�`�F�b�N**
        bool xOverlap = false;
        for (float checkX : checkPoints) {
            if (checkX >= blockLeft - 2 && checkX <= blockRight + 2) { // �����]�T����������
                xOverlap = true;
                break;
            }
        }

        if (!xOverlap) continue;

        // **Y�����̒��n����**
        float distanceToTop = footY - blockTop;
        if (distanceToTop >= -2.0f && distanceToTop <= LANDING_TOLERANCE && distanceToTop < minDistance) {
            minDistance = distanceToTop;
            nearestBlockTop = blockTop;
        }
    }

    // **���n����**
    if (nearestBlockTop != -1.0f) {
        // **���m�Ƀu���b�N��ʂɔz�u**
        float targetY = nearestBlockTop - PLAYER_HEIGHT / 2;

        player->SetPosition(playerX, targetY);
        player->SetVelocityY(0.0f);
        player->SetOnGround(true);


        // **��Ԃ�K�؂ɐݒ�**
        if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
            bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);
            if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
                player->SetState(Player::DUCKING);
            }
            else if (hasInput) {
                player->SetState(Player::WALKING);
            }
            else {
                player->SetState(Player::IDLE);
            }
        }

        // **�f�o�b�O�o��**
        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Player landed on block! Y: %.1f -> %.1f\n",
            playerY, targetY);
        OutputDebugStringA(debugMsg);
    }
}

// �u���b�N���ʂւ̏Փˏ���
void BlockSystem::HandleBlockCeilingCollision(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float headY = playerY - PLAYER_HEIGHT / 2;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockBottom = block->y + BLOCK_SIZE;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // �����̏Փ˃`�F�b�N
        bool xOverlap = (playerX + PLAYER_WIDTH / 2 > blockLeft &&
            playerX - PLAYER_WIDTH / 2 < blockRight);

        if (xOverlap && headY <= blockBottom && headY >= block->y) {
            // �u���b�N���ʂɂԂ�����
            player->SetPosition(playerX, blockBottom + PLAYER_HEIGHT / 2 + 1);
            player->SetVelocityY(0.0f);

            if (player->GetState() == Player::JUMPING) {
                player->SetState(Player::FALLING);
            }
            break;
        }
    }
}

// �n�ʏ�Ԃ̈��艻
void BlockSystem::StabilizeGroundState(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;
    const float GROUND_CHECK_TOLERANCE = 8.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float footY = playerY + PLAYER_HEIGHT / 2;

    bool stillOnBlock = false;

    // ���݃u���b�N��ɂ��邩�`�F�b�N
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // ���L���͈͂Ń`�F�b�N�i���艻�̂��߁j
        bool xOverlap = (playerX + PLAYER_WIDTH / 2 - 8 > blockLeft &&
            playerX - PLAYER_WIDTH / 2 + 8 < blockRight);

        float distanceToTop = footY - blockTop;

        if (xOverlap && distanceToTop >= -2.0f && distanceToTop <= GROUND_CHECK_TOLERANCE) {
            stillOnBlock = true;
            // �ʒu��������i�u���b�N��ʂɌŒ�j
            player->SetPosition(playerX, blockTop - PLAYER_HEIGHT / 2);
            break;
        }
    }

    // �u���b�N���痣�ꂽ�ꍇ
    if (!stillOnBlock) {
        player->SetOnGround(false);
        if (player->GetState() != Player::JUMPING) {
            player->SetState(Player::FALLING);
        }
    }
}

void BlockSystem::HandleBlockLandingOnly(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **�����ȏ���: �������Ɉړ����ŁA�n�ʂɂ��Ȃ��ꍇ�̂�**
    if (playerVelY <= 1.0f || player->IsOnGround()) return;

    // **�v���C���[�̑����̐��m�Ȉʒu**
    float footY = playerY + PLAYER_HEIGHT / 2;

    // **���ɋ�������͈́i�v���C���[�̒��������̂݁j**
    float checkLeft = playerX - 20.0f;    // �������獶20px
    float checkRight = playerX + 20.0f;   // ��������E20px

    bool validLanding = false;
    float targetBlockTop = -1.0f;
    float minDistance = 999.0f;

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **X���̏d�Ȃ�`�F�b�N�i���ɋ����j**
        bool xOverlap = (checkRight > blockLeft && checkLeft < blockRight);
        if (!xOverlap) continue;

        // **Y���̒��n����i���Ɍ����j**
        float distanceToTop = footY - blockTop;

        if (distanceToTop >= 0 && distanceToTop <= 6.0f && distanceToTop < minDistance) {
            minDistance = distanceToTop;
            targetBlockTop = blockTop;
            validLanding = true;
        }
    }

    if (validLanding && targetBlockTop != -1.0f) {
        // **�u���b�N��ɐ��m�ɒ��n**
        player->SetPosition(playerX, targetBlockTop - PLAYER_HEIGHT / 2);
        player->SetVelocityY(0.0f);
        player->SetOnGround(true);

        // **�K�؂ȏ�ԂɕύX**
        if (player->GetState() == Player::FALLING) {
            bool hasInput = CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT);
            if (CheckHitKey(KEY_INPUT_DOWN) && !hasInput) {
                player->SetState(Player::DUCKING);
            }
            else if (hasInput) {
                player->SetState(Player::WALKING);
            }
            else {
                player->SetState(Player::IDLE);
            }
        }

        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Block landing! Distance: %.1f, BlockTop: %.1f\n",
            minDistance, targetBlockTop);
        OutputDebugStringA(debugMsg);
    }
}
// **������CheckPlayerHitFromBelowImproved�֐��̎������m�F**
bool BlockSystem::CheckPlayerHitFromBelowImproved(const Block& block, Player* player)
{
    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **�C��: �v���C���[��������Ɉړ����Ă���ꍇ�̂ݔ���**
    if (playerVelY >= 0) return false;

    // **�C��: ��茵����X���̏d�Ȃ蔻��**
    const float PLAYER_WIDTH = 80.0f;
    const float COLLISION_MARGIN = 4.0f; // ����̗]�T�������ɂ炷

    float playerLeft = playerX - PLAYER_WIDTH / 2 + COLLISION_MARGIN;
    float playerRight = playerX + PLAYER_WIDTH / 2 - COLLISION_MARGIN;
    float blockLeft = block.x;
    float blockRight = block.x + BLOCK_SIZE;

    bool xOverlap = (playerRight > blockLeft && playerLeft < blockRight);
    if (!xOverlap) return false;

    // **�C��: ��萳�m��Y���̈ʒu�֌W����**
    const float PLAYER_HEIGHT = 100.0f;
    const float HIT_TOLERANCE = 20.0f; // ���e�͈�

    float playerHead = playerY - PLAYER_HEIGHT / 2;
    float blockBottom = block.y + BLOCK_SIZE;

    // �v���C���[�̓����u���b�N�̉��ʕt�߂ɂ���
    float distance = abs(playerHead - blockBottom);
    bool isHittingFromBelow = (distance <= HIT_TOLERANCE) && (playerHead <= blockBottom);

    // **�f�o�b�O�o��**
    if (isHittingFromBelow) {
        char debugMsg[256];
        sprintf_s(debugMsg, "BlockSystem: Player hit block from below! Distance: %.1f, PlayerHead: %.1f, BlockBottom: %.1f\n",
            distance, playerHead, blockBottom);
        OutputDebugStringA(debugMsg);
    }

    return isHittingFromBelow;
}




// **�V����: CheckPlayerLandingOnBlocks - �u���b�N��ւ̒��n����**
bool BlockSystem::CheckPlayerLandingOnBlocks(float playerX, float playerY, float playerWidth, float playerHeight)
{
    const float LANDING_TOLERANCE = 8.0f;

    // �v���C���[�̑����ʒu
    float footY = playerY + playerHeight / 2;
    float leftFoot = playerX - playerWidth / 3;
    float rightFoot = playerX + playerWidth / 3;
    float centerFoot = playerX;

    // �e�u���b�N�Ƃ̏Փ˂��`�F�b�N
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        // �u���b�N�̏��
        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // X���̏d�Ȃ�`�F�b�N
        bool xOverlap = (rightFoot > blockLeft && leftFoot < blockRight);
        if (!xOverlap) continue;

        // Y���̒��n����i�v���C���[�̑����u���b�N�̏�ʕt�߁j
        float distanceToBlockTop = abs(footY - blockTop);
        if (distanceToBlockTop <= LANDING_TOLERANCE && footY >= blockTop) {

            char debugMsg[256];
            sprintf_s(debugMsg, "BlockSystem: Player landing on block! FootY: %.1f, BlockTop: %.1f, Distance: %.1f\n",
                footY, blockTop, distanceToBlockTop);
            OutputDebugStringA(debugMsg);

            return true;
        }
    }

    return false;
}

bool BlockSystem::CheckPlayerLandingOnBlocksImproved(float playerX, float playerY, float playerWidth, float playerHeight)
{
    const float LANDING_TOLERANCE = 6.0f; // **�����Ȕ���**

    // **�v���C���[�̑����ʒu�i���������̂݁j**
    float footY = playerY + playerHeight / 2;
    float checkLeft = playerX - 20.0f;    // �������獶20px
    float checkRight = playerX + 20.0f;   // ��������E20px

    // �e�u���b�N�Ƃ̏Փ˂��`�F�b�N
    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **X���̏d�Ȃ�`�F�b�N�i�����͈́j**
        bool xOverlap = (checkRight > blockLeft && checkLeft < blockRight);
        if (!xOverlap) continue;

        // **Y���̒��n����i�����j**
        float distanceToBlockTop = footY - blockTop;

        if (distanceToBlockTop >= 0 && distanceToBlockTop <= LANDING_TOLERANCE) {
            char debugMsg[256];
            sprintf_s(debugMsg, "BlockSystem: Improved landing detected! FootY: %.1f, BlockTop: %.1f, Distance: %.1f\n",
                footY, blockTop, distanceToBlockTop);
            OutputDebugStringA(debugMsg);
            return true;
        }
    }

    return false;
}

// **�V����: HandleBlockLanding - �u���b�N�㒅�n�̏���**
void BlockSystem::HandleBlockLanding(Player* player)
{
    if (!player) return;

    const float PLAYER_WIDTH = 80.0f;
    const float PLAYER_HEIGHT = 100.0f;

    float playerX = player->GetX();
    float playerY = player->GetY();
    float playerVelY = player->GetVelocityY();

    // **�������Ɉړ����ŁA�܂��n��ɂ��Ȃ��ꍇ�̂݃`�F�b�N**
    if (playerVelY <= 0.0f || player->IsOnGround()) return;

    // **�u���b�N��ւ̒��n���`�F�b�N**
    if (CheckPlayerLandingOnBlocks(playerX, playerY, PLAYER_WIDTH, PLAYER_HEIGHT)) {

        // **�ł��߂��u���b�N�̏�ʂɐ��m�ɔz�u**
        float targetY = FindNearestBlockTop(playerX, playerY, PLAYER_WIDTH);

        if (targetY != -1.0f) {
            // �v���C���[���u���b�N�̏�ɔz�u
            player->SetPosition(playerX, targetY - PLAYER_HEIGHT / 2);
            player->SetVelocityY(0.0f);
            player->SetOnGround(true);  // **�d�v: �n�ʏ�Ԃ��m���ɐݒ�**

            // **�K�؂ȏ�ԂɕύX**
            if (player->GetState() == Player::FALLING || player->GetState() == Player::JUMPING) {
                if (CheckHitKey(KEY_INPUT_LEFT) || CheckHitKey(KEY_INPUT_RIGHT)) {
                    player->SetState(Player::WALKING);
                }
                else {
                    player->SetState(Player::IDLE);
                }
            }

            OutputDebugStringA("BlockSystem: Player successfully landed on block!\n");
        }
    }
}

float BlockSystem::FindNearestBlockTop(float playerX, float playerY, float playerWidth)
{
    float nearestBlockTop = -1.0f;
    float minDistance = 999.0f;

    const float LANDING_TOLERANCE = 4.0f; // **�����Ȕ���**
    float footY = playerY + 50.0f; // �v���C���[�̑����t��

    for (const auto& block : blocks) {
        if (!IsBlockSolid(*block)) continue;

        float blockTop = block->y;
        float blockLeft = block->x;
        float blockRight = block->x + BLOCK_SIZE;

        // **X���̏d�Ȃ�`�F�b�N�i��茵���Ɂj**
        float playerLeft = playerX - playerWidth / 4;   // **�͈͂����߂�**
        float playerRight = playerX + playerWidth / 4;  // **�͈͂����߂�**

        bool xOverlap = (playerRight >= blockLeft && playerLeft <= blockRight);
        if (!xOverlap) continue;

        // **Y���̋����`�F�b�N**
        float distance = footY - blockTop;
        if (distance >= 0 && distance <= LANDING_TOLERANCE && distance < minDistance) {
            minDistance = distance;
            nearestBlockTop = blockTop;
        }
    }

    return nearestBlockTop;
}