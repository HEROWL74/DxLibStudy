#pragma once
#include "DxLib.h"
#include <vector>
#include <memory>
#include <string>

// �^�C�����\����
struct Tile {
    int textureHandle;
    int x, y;           // ���[���h���W
    bool isSolid;       // �����蔻�肪���邩

    Tile(int handle, int posX, int posY, bool solid = true)
        : textureHandle(handle), x(posX), y(posY), isSolid(solid) {}
};

// �X�e�[�W���N���X
class Stage {
public:
    static const int TILE_SIZE = 64;        // �^�C���T�C�Y
    static const int STAGE_WIDTH = 7680;    // �X�e�[�W���i60�^�C�� �� 120�^�C���Ɋg���j
    static const int STAGE_HEIGHT = 1080;   // �X�e�[�W����
    static const int GROUND_LEVEL = 800;    // �n�ʃ��x��

    Stage();
    virtual ~Stage();

    virtual void Initialize() = 0;
    virtual void Update(float cameraX);
    virtual void Draw(float cameraX);

    // �����蔻��
    bool CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight);
    float GetGroundY(float playerX, float playerWidth);

protected:
    std::vector<std::unique_ptr<Tile>> tiles;
    std::string terrainType;

    // �^�C���z�u�p�w���p�[
    void PlaceTile(const std::string& tileType, int gridX, int gridY);
    void PlacePlatformBlock(int gridX, int gridY); // **�V�ǉ��F�P��u���b�N�z�u**
    void CreatePlatform(int startX, int startY, int width, int height);
    void LoadTerrainTextures();

    // **�V�ǉ��F�v���b�g�t�H�[����p�e�N�X�`���Ǘ�**
    void LoadPlatformTexture();
    void CreatePlatformBlock(int startX, int startY, int width, int height);

    // �e�N�X�`���n���h���Ǘ�
    struct TerrainTextures {
        int bottom, bottom_left, bottom_right;
        int center, left, right;
        int top, top_left, top_right;
    } textures;

    // **�V�ǉ��F�P��v���b�g�t�H�[���e�N�X�`��**
    int platformBlockTexture;

private:
    void LoadTileTexture(int& handle, const std::string& filePath);
};