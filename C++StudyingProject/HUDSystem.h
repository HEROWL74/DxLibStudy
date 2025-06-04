#pragma once
#include "DxLib.h"
#include <string>
#include <vector>

class HUDSystem {
public:
    // �n�[�g�̏��
    enum HeartState {
        HEART_FULL,      // ���^��
        HEART_HALF,      // ����
        HEART_EMPTY      // ��
    };

    HUDSystem();
    ~HUDSystem();

    void Initialize();
    void Update();
    void Draw();

    // ���C�t�Ǘ�
    void SetMaxLife(int maxLife) { this->maxLife = maxLife; }
    void SetCurrentLife(int currentLife);
    void AddLife(int amount);
    void SubtractLife(int amount);
    int GetCurrentLife() const { return currentLife; }
    int GetMaxLife() const { return maxLife; }

    // �v���C���[�A�C�R���ݒ�
    void SetPlayerCharacter(int characterIndex);
    void SetPlayerCharacter(const std::string& colorName);

    // �R�C���Ǘ�
    void SetCoins(int coins) { this->coins = coins; }
    void AddCoins(int amount) { coins += amount; }
    void SubtractCoins(int amount) { coins = max(0, coins - amount); }
    int GetCoins() const { return coins; }

    // **���Ǘ��i�V�@�\�j**
    void SetCollectedStars(int collected) { this->collectedStars = collected; }
    void SetTotalStars(int total) { this->totalStars = total; }
    int GetCollectedStars() const { return collectedStars; }
    int GetTotalStars() const { return totalStars; }

    // HUD�\���ݒ�
    void SetPosition(int x, int y) { hudX = x; hudY = y; }
    void SetVisible(bool visible) { this->visible = visible; }
    bool IsVisible() const { return visible; }

private:
    // �e�N�X�`���n���h��
    struct HeartTextures {
        int full;
        int half;
        int empty;
    } heartTextures;

    struct PlayerIconTextures {
        int beige;
        int green;
        int pink;
        int purple;
        int yellow;
    } playerIconTextures;

    struct CoinTextures {
        int coin;
        int numbers[10]; // 0-9�̐���
    } coinTextures;

    // **���e�N�X�`���i�V�@�\�j**
    struct StarTextures {
        int starOutline;    // star_outline_depth.png
        int starFilled;     // star.png
    } starTextures;

    // �Q�[�����
    int maxLife;        // �ő僉�C�t (�ʏ��6 = 3�n�[�g �~ 2)
    int currentLife;    // ���݂̃��C�t
    int coins;          // �R�C����
    int currentPlayerCharacter; // ���݂̃v���C���[�L�����N�^�[

    // **���̏�ԁi�V�@�\�j**
    int collectedStars; // ���W�ςݐ���
    int totalStars;     // �������i�ʏ��3�j

    // HUD�\���ݒ�
    int hudX, hudY;     // HUD�̊�ʒu
    bool visible;       // HUD�\���t���O

    // **�V�ǉ��F�n�[�g�h��A�j���[�V�����p�ϐ�**
    int previousLife;           // �O�t���[���̃��C�t�i�_���[�W���o�p�j
    float heartShakeTimer;      // �n�[�g�h��^�C�}�[
    float heartShakeIntensity;  // �n�[�g�h�ꋭ�x (0.0f�`1.0f)
    float heartShakePhase;      // �n�[�g�h��ʑ�

    // **�n�[�g�h��֘A�萔**
    static constexpr float HEART_SHAKE_DURATION = 1.0f;  // �h�ꎝ�����ԁi�b�j
    static constexpr float HEART_SHAKE_AMOUNT = 3.0f;    // �h��̋��x�i�s�N�Z���j

    // ���C�A�E�g�萔�i�g���Łj
    static const int HEART_SIZE = 64;           // 32 �� 64�Ɋg��
    static const int PLAYER_ICON_SIZE = 80;     // 48 �� 80�Ɋg��
    static const int COIN_ICON_SIZE = 48;       // 32 �� 48�Ɋg��
    static const int NUMBER_SIZE = 36;          // 24 �� 36�Ɋg��
    static const int STAR_SIZE = 48;            // ���̃T�C�Y�i�V�@�\�j
    static const int ELEMENT_SPACING = 20;      // 16 �� 20�Ɋg��

    // �w���p�[�֐�
    void LoadTextures();
    void DrawHearts();
    void DrawPlayerIcon();
    void DrawCoins();
    void DrawStars();  // **���`��֐��i�V�@�\�j**
    void DrawNumber(int number, int x, int y);
    HeartState GetHeartState(int heartIndex) const;
    int GetPlayerIconHandle() const;
    std::string GetCharacterColorName(int characterIndex) const;
};