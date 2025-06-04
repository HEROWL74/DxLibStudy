#include "HUDSystem.h"
#include <algorithm>
#include <math.h>

// HUDSystem.cpp �̃R���X�g���N�^�ɒǉ�
HUDSystem::HUDSystem()
    : maxLife(6)          // 3�n�[�g �~ 2 = 6���C�t
    , currentLife(6)      // �����l�͖��^��
    , coins(0)            // �����R�C����
    , collectedStars(0)   // **���������i�V�@�\�j**
    , totalStars(3)       // **�������i�V�@�\�j**
    , currentPlayerCharacter(0) // �f�t�H���g��beige
    , hudX(30)            // ���ォ��30�s�N�Z���i�����]�T����������j
    , hudY(30)            // �ォ��30�s�N�Z���i�����]�T����������j
    , visible(true)       // ������Ԃŕ\��
    , previousLife(6)     // **�O�t���[���̃��C�t�i�_���[�W���o�p�j**
    , heartShakeTimer(0.0f) // **�n�[�g�h��^�C�}�[**
    , heartShakeIntensity(0.0f) // **�n�[�g�h�ꋭ�x**
    , heartShakePhase(0.0f)     // **�n�[�g�h��ʑ�**
{
    // �e�N�X�`���n���h����������
    heartTextures.full = heartTextures.half = heartTextures.empty = -1;
    playerIconTextures.beige = playerIconTextures.green = playerIconTextures.pink = -1;
    playerIconTextures.purple = playerIconTextures.yellow = -1;
    coinTextures.coin = -1;

    // **���e�N�X�`���������i�V�@�\�j**
    starTextures.starOutline = starTextures.starFilled = -1;

    for (int i = 0; i < 10; i++) {
        coinTextures.numbers[i] = -1;
    }
}

HUDSystem::~HUDSystem()
{
    // �n�[�g�e�N�X�`���̉��
    if (heartTextures.full != -1) DeleteGraph(heartTextures.full);
    if (heartTextures.half != -1) DeleteGraph(heartTextures.half);
    if (heartTextures.empty != -1) DeleteGraph(heartTextures.empty);

    // �v���C���[�A�C�R���e�N�X�`���̉��
    if (playerIconTextures.beige != -1) DeleteGraph(playerIconTextures.beige);
    if (playerIconTextures.green != -1) DeleteGraph(playerIconTextures.green);
    if (playerIconTextures.pink != -1) DeleteGraph(playerIconTextures.pink);
    if (playerIconTextures.purple != -1) DeleteGraph(playerIconTextures.purple);
    if (playerIconTextures.yellow != -1) DeleteGraph(playerIconTextures.yellow);

    // �R�C���e�N�X�`���̉��
    if (coinTextures.coin != -1) DeleteGraph(coinTextures.coin);

    // **���e�N�X�`���̉���i�V�@�\�j**
    if (starTextures.starOutline != -1) DeleteGraph(starTextures.starOutline);
    if (starTextures.starFilled != -1) DeleteGraph(starTextures.starFilled);

    for (int i = 0; i < 10; i++) {
        if (coinTextures.numbers[i] != -1) {
            DeleteGraph(coinTextures.numbers[i]);
        }
    }
}

void HUDSystem::Initialize()
{
    LoadTextures();
}

// LoadTextures�֐��ɒǉ�
void HUDSystem::LoadTextures()
{
    // �n�[�g�e�N�X�`���̓ǂݍ���
    heartTextures.full = LoadGraph("Sprites/Tiles/hud_heart.png");
    heartTextures.half = LoadGraph("Sprites/Tiles/hud_heart_half.png");
    heartTextures.empty = LoadGraph("Sprites/Tiles/hud_heart_empty.png");

    // �v���C���[�A�C�R���e�N�X�`���̓ǂݍ���
    playerIconTextures.beige = LoadGraph("Sprites/Tiles/hud_player_beige.png");
    playerIconTextures.green = LoadGraph("Sprites/Tiles/hud_player_green.png");
    playerIconTextures.pink = LoadGraph("Sprites/Tiles/hud_player_pink.png");
    playerIconTextures.purple = LoadGraph("Sprites/Tiles/hud_player_purple.png");
    playerIconTextures.yellow = LoadGraph("Sprites/Tiles/hud_player_yellow.png");

    // �R�C���e�N�X�`���̓ǂݍ���
    coinTextures.coin = LoadGraph("Sprites/Tiles/hud_coin.png");

    // **���e�N�X�`���̓ǂݍ��݁i�V�@�\�j**
    starTextures.starOutline = LoadGraph("UI/PNG/Yellow/star_outline_depth.png");
    starTextures.starFilled = LoadGraph("UI/PNG/Yellow/star.png");

    // �����e�N�X�`���̓ǂݍ��� (0-9)
    for (int i = 0; i < 10; i++) {
        std::string numberPath = "Sprites/Tiles/hud_character_" + std::to_string(i) + ".png";
        coinTextures.numbers[i] = LoadGraph(numberPath.c_str());
    }
}

void HUDSystem::Update()
{
    // **�_���[�W���o�ƃn�[�g�h��J�n**
    if (currentLife < previousLife) {
        // ���C�t���������ꍇ�A�n�[�g�h����J�n
        heartShakeTimer = 0.0f;
        heartShakeIntensity = 1.0f;
        heartShakePhase = 0.0f;

        // �f�o�b�O�o��
        OutputDebugStringA("HUDSystem: Player took damage! Starting heart shake animation.\n");
    }

    // �O�t���[���̃��C�t���X�V
    previousLife = currentLife;

    // **�n�[�g�h��A�j���[�V�����̍X�V**
    if (heartShakeIntensity > 0.0f) {
        heartShakeTimer += 0.016f; // 60FPS�z��
        heartShakePhase += 0.4f;   // �h��̑��x

        // �h�ꋭ�x�̌����i1�b�Ŋ��S�ɒ�~�j
        heartShakeIntensity = max(0.0f, 1.0f - (heartShakeTimer / HEART_SHAKE_DURATION));

        if (heartShakeIntensity <= 0.0f) {
            heartShakeTimer = 0.0f;
            heartShakePhase = 0.0f;
        }
    }
}

// Draw�֐��ɐ��`���ǉ�
void HUDSystem::Draw()
{
    if (!visible) return;

    // HUD�v�f�����Ԃɕ`��
    DrawHearts();      // ���C�t�n�[�g�i�h��@�\�t���j
    DrawPlayerIcon();  // �v���C���[�A�C�R��
    DrawCoins();       // �R�C���\��
    DrawStars();       // **���\���i�V�@�\�j**
}

void HUDSystem::DrawHearts()
{
    int heartsCount = 3; // ���3�̃n�[�g��\��
    int heartY = hudY;

    for (int i = 0; i < heartsCount; i++) {
        int heartX = hudX + (i * (HEART_SIZE + 12)); // 12�s�N�Z���̊Ԋu�i�g��j
        HeartState state = GetHeartState(i);

        int textureHandle = -1;
        switch (state) {
        case HEART_FULL:
            textureHandle = heartTextures.full;
            break;
        case HEART_HALF:
            textureHandle = heartTextures.half;
            break;
        case HEART_EMPTY:
            textureHandle = heartTextures.empty;
            break;
        }

        if (textureHandle != -1) {
            // **�n�[�g�h��G�t�F�N�g�̌v�Z**
            float shakeOffsetX = 0.0f;
            float shakeOffsetY = 0.0f;

            if (heartShakeIntensity > 0.0f) {
                // ���������̃����_���ȗh��
                float shakeAmount = heartShakeIntensity * HEART_SHAKE_AMOUNT;

                // X���̗h��i�����g�j
                shakeOffsetX = sinf(heartShakePhase * 18.0f + i * 0.5f) * shakeAmount;

                // Y���̗h��i�����Ⴂ���g���j
                shakeOffsetY = cosf(heartShakePhase * 15.0f + i * 0.3f) * shakeAmount * 0.7f;

                // �h��̃o���G�[�V�������e�n�[�g�ŏ����ς���
                if (i == 1) {
                    shakeOffsetX *= 1.2f;
                    shakeOffsetY *= 0.8f;
                }
                else if (i == 2) {
                    shakeOffsetX *= 0.9f;
                    shakeOffsetY *= 1.1f;
                }
            }

            // ���炩�Ȋg��\���i�h��ʒu��K�p�j
            int finalHeartX = heartX + (int)shakeOffsetX;
            int finalHeartY = heartY + (int)shakeOffsetY;

            DrawExtendGraph(
                finalHeartX, finalHeartY,
                finalHeartX + HEART_SIZE, finalHeartY + HEART_SIZE,
                textureHandle, TRUE
            );

            // **�_���[�W���̒ǉ��G�t�F�N�g�i�I�v�V�����j**
            if (heartShakeIntensity > 0.5f && state != HEART_EMPTY) {
                // �Ԃ����̃G�t�F�N�g
                SetDrawBlendMode(DX_BLENDMODE_ADD, (int)(50 * heartShakeIntensity));
                DrawExtendGraph(
                    finalHeartX - 2, finalHeartY - 2,
                    finalHeartX + HEART_SIZE + 2, finalHeartY + HEART_SIZE + 2,
                    textureHandle, TRUE
                );
                SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 255);
            }
        }
    }
}

void HUDSystem::DrawPlayerIcon()
{
    int iconX = hudX;
    int iconY = hudY + HEART_SIZE + ELEMENT_SPACING; // �n�[�g�̉��ɔz�u

    int iconHandle = GetPlayerIconHandle();
    if (iconHandle != -1) {
        // ���炩�Ȋg��\��
        DrawExtendGraph(
            iconX, iconY,
            iconX + PLAYER_ICON_SIZE, iconY + PLAYER_ICON_SIZE,
            iconHandle, TRUE
        );
    }
}

void HUDSystem::DrawCoins()
{
    int coinStartX = hudX + PLAYER_ICON_SIZE + ELEMENT_SPACING; // �v���C���[�A�C�R���̉E
    int coinY = hudY + HEART_SIZE + ELEMENT_SPACING + (PLAYER_ICON_SIZE - COIN_ICON_SIZE) / 2; // ��������

    // �R�C���A�C�R����`��i�g��\���j
    if (coinTextures.coin != -1) {
        DrawExtendGraph(
            coinStartX, coinY,
            coinStartX + COIN_ICON_SIZE, coinY + COIN_ICON_SIZE,
            coinTextures.coin, TRUE
        );
    }

    // �R�C������`��
    int numberX = coinStartX + COIN_ICON_SIZE + 12; // �R�C���A�C�R������12�s�N�Z���E�i�g��j
    int numberY = coinY + (COIN_ICON_SIZE - NUMBER_SIZE) / 2; // ��������

    DrawNumber(coins, numberX, numberY);
}

void HUDSystem::DrawNumber(int number, int x, int y)
{
    // �����𕶎���ɕϊ�
    std::string numberStr = std::to_string(number);

    // �Œ�ł�1���͕\���i0�̏ꍇ�j
    if (numberStr.empty()) {
        numberStr = "0";
    }

    // �e����`��i�g��\���j
    for (size_t i = 0; i < numberStr.length(); i++) {
        int digit = numberStr[i] - '0'; // �����𐔎��ɕϊ�

        if (digit >= 0 && digit <= 9 && coinTextures.numbers[digit] != -1) {
            int digitX = x + (i * (NUMBER_SIZE - 6)); // �����Ԃ̊Ԋu�𒲐��i�g��Łj

            // ���炩�Ȋg��\��
            DrawExtendGraph(
                digitX, y,
                digitX + NUMBER_SIZE, y + NUMBER_SIZE,
                coinTextures.numbers[digit], TRUE
            );
        }
    }
}

HUDSystem::HeartState HUDSystem::GetHeartState(int heartIndex) const
{
    // �e�n�[�g��2���C�t����\��
    int heartLife = currentLife - (heartIndex * 2);

    if (heartLife >= 2) {
        return HEART_FULL;
    }
    else if (heartLife == 1) {
        return HEART_HALF;
    }
    else {
        return HEART_EMPTY;
    }
}

int HUDSystem::GetPlayerIconHandle() const
{
    switch (currentPlayerCharacter) {
    case 0: return playerIconTextures.beige;
    case 1: return playerIconTextures.green;
    case 2: return playerIconTextures.pink;
    case 3: return playerIconTextures.purple;
    case 4: return playerIconTextures.yellow;
    default: return playerIconTextures.beige;
    }
}

std::string HUDSystem::GetCharacterColorName(int characterIndex) const
{
    switch (characterIndex) {
    case 0: return "beige";
    case 1: return "green";
    case 2: return "pink";
    case 3: return "purple";
    case 4: return "yellow";
    default: return "beige";
    }
}

void HUDSystem::SetCurrentLife(int currentLife)
{
    this->currentLife = max(0, min(currentLife, maxLife));
}

void HUDSystem::AddLife(int amount)
{
    currentLife = min(currentLife + amount, maxLife);
}

void HUDSystem::SubtractLife(int amount)
{
    currentLife = max(0, currentLife - amount);
}

void HUDSystem::SetPlayerCharacter(int characterIndex)
{
    if (characterIndex >= 0 && characterIndex <= 4) {
        currentPlayerCharacter = characterIndex;
    }
}

void HUDSystem::SetPlayerCharacter(const std::string& colorName)
{
    if (colorName == "beige") currentPlayerCharacter = 0;
    else if (colorName == "green") currentPlayerCharacter = 1;
    else if (colorName == "pink") currentPlayerCharacter = 2;
    else if (colorName == "purple") currentPlayerCharacter = 3;
    else if (colorName == "yellow") currentPlayerCharacter = 4;
    else currentPlayerCharacter = 0; // �f�t�H���g
}

void HUDSystem::DrawStars()
{
    int starStartX = hudX + PLAYER_ICON_SIZE + ELEMENT_SPACING +
        COIN_ICON_SIZE + 60 + ELEMENT_SPACING; // �R�C���̉E��
    int starY = hudY + HEART_SIZE + ELEMENT_SPACING + (PLAYER_ICON_SIZE - STAR_SIZE) / 2; // ��������

    for (int i = 0; i < totalStars; i++) {
        int starX = starStartX + i * (STAR_SIZE + 8); // 8�s�N�Z���Ԋu

        // ���̃e�N�X�`���I��
        int starTexture = (i < collectedStars) ? starTextures.starFilled : starTextures.starOutline;

        if (starTexture != -1) {
            // ���炩�Ȋg��\��
            DrawExtendGraph(
                starX, starY,
                starX + STAR_SIZE, starY + STAR_SIZE,
                starTexture, TRUE
            );
        }
    }
}