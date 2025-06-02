#include "Stage.h"
#include <algorithm>
#include <cmath>

Stage::Stage() {
    // �e�N�X�`���n���h����������
    textures.bottom = textures.bottom_left = textures.bottom_right = -1;
    textures.center = textures.left = textures.right = -1;
    textures.top = textures.top_left = textures.top_right = -1;
}

Stage::~Stage() {
    // �e�N�X�`���n���h�����폜
    if (textures.bottom != -1) DeleteGraph(textures.bottom);
    if (textures.bottom_left != -1) DeleteGraph(textures.bottom_left);
    if (textures.bottom_right != -1) DeleteGraph(textures.bottom_right);
    if (textures.center != -1) DeleteGraph(textures.center);
    if (textures.left != -1) DeleteGraph(textures.left);
    if (textures.right != -1) DeleteGraph(textures.right);
    if (textures.top != -1) DeleteGraph(textures.top);
    if (textures.top_left != -1) DeleteGraph(textures.top_left);
    if (textures.top_right != -1) DeleteGraph(textures.top_right);
}

void Stage::Update(float cameraX) {
    // ���N���X�ł͓��ɏ����Ȃ�
    // �h���N���X�œ��I�ȗv�f������Ύ���
}

void Stage::Draw(float cameraX) {
    for (const auto& tile : tiles) {
        // �J�������W���l�������`��ʒu
        int screenX = tile->x - (int)cameraX;
        int screenY = tile->y;

        // ��ʊO�̃^�C���͕`�悵�Ȃ��i�œK���j
        if (screenX > -TILE_SIZE && screenX < 1920 + TILE_SIZE) {
            DrawGraph(screenX, screenY, tile->textureHandle, TRUE);
        }
    }
}

bool Stage::CheckCollision(float playerX, float playerY, float playerWidth, float playerHeight) {
    for (const auto& tile : tiles) {
        if (!tile->isSolid) continue;

        // AABB�����蔻��
        if (playerX < tile->x + TILE_SIZE &&
            playerX + playerWidth > tile->x &&
            playerY < tile->y + TILE_SIZE &&
            playerY + playerHeight > tile->y) {
            return true;
        }
    }
    return false;
}

float Stage::GetGroundY(float playerX, float playerWidth) {
    float groundY = STAGE_HEIGHT; // �f�t�H���g�͉�ʉ��[

    for (const auto& tile : tiles) {
        if (!tile->isSolid) continue;

        // �v���C���[��X�͈͂ƃ^�C����X�͈͂��d�Ȃ��Ă��邩
        if (playerX < tile->x + TILE_SIZE && playerX + playerWidth > tile->x) {
            // ��荂���n�ʂ�I��
            groundY = min(groundY, (float)tile->y);
        }
    }

    return groundY;
}

void Stage::LoadTerrainTextures() {
    std::string basePath = "Sprites/Tiles/terrain_" + terrainType + "_block_";

    LoadTileTexture(textures.bottom, basePath + "bottom.png");
    LoadTileTexture(textures.bottom_left, basePath + "bottom_left.png");
    LoadTileTexture(textures.bottom_right, basePath + "bottom_right.png");
    LoadTileTexture(textures.center, basePath + "center.png");
    LoadTileTexture(textures.left, basePath + "left.png");
    LoadTileTexture(textures.right, basePath + "right.png");
    LoadTileTexture(textures.top, basePath + "top.png");
    LoadTileTexture(textures.top_left, basePath + "top_left.png");
    LoadTileTexture(textures.top_right, basePath + "top_right.png");
}

void Stage::LoadTileTexture(int& handle, const std::string& filePath) {
    handle = LoadGraph(filePath.c_str());
}

void Stage::PlaceTile(const std::string& tileType, int gridX, int gridY) {
    int textureHandle = -1;

    // �^�C���^�C�v�ɉ����ăe�N�X�`����I��
    if (tileType == "bottom") textureHandle = textures.bottom;
    else if (tileType == "bottom_left") textureHandle = textures.bottom_left;
    else if (tileType == "bottom_right") textureHandle = textures.bottom_right;
    else if (tileType == "center") textureHandle = textures.center;
    else if (tileType == "left") textureHandle = textures.left;
    else if (tileType == "right") textureHandle = textures.right;
    else if (tileType == "top") textureHandle = textures.top;
    else if (tileType == "top_left") textureHandle = textures.top_left;
    else if (tileType == "top_right") textureHandle = textures.top_right;

    if (textureHandle != -1) {
        tiles.push_back(std::make_unique<Tile>(
            textureHandle,
            gridX * TILE_SIZE,
            gridY * TILE_SIZE
        ));
    }
}

void Stage::CreatePlatform(int startX, int startY, int width, int height) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            std::string tileType;

            // �p�̔���
            if (y == 0 && x == 0) {
                tileType = "top_left";
            }
            else if (y == 0 && x == width - 1) {
                tileType = "top_right";
            }
            else if (y == height - 1 && x == 0) {
                tileType = "bottom_left";
            }
            else if (y == height - 1 && x == width - 1) {
                tileType = "bottom_right";
            }
            // �ӂ̔���
            else if (y == 0) {
                tileType = "top";
            }
            else if (y == height - 1) {
                tileType = "bottom";
            }
            else if (x == 0) {
                tileType = "left";
            }
            else if (x == width - 1) {
                tileType = "right";
            }
            // ����
            else {
                tileType = "center";
            }

            PlaceTile(tileType, startX + x, startY + y);
        }
    }
}