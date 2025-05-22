#include "TitleGenerate.h"
#include "DxLib.h"

TitleGenerate::TitleGenerate() {
    LoadTiles();
    SetupMap();
}

TitleGenerate::~TitleGenerate() {
    for (const auto& pair : tileHandles) {
        DeleteGraph(pair.second);
    }
}

void TitleGenerate::LoadTiles() {
    tileHandles["center"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_center.png");
    tileHandles["top"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_top.png");
    tileHandles["bottom"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_bottom.png");
    tileHandles["left"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_left.png");
    tileHandles["right"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_right.png");
    tileHandles["top_left"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_top_left.png");
    tileHandles["top_right"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_top_right.png");
    tileHandles["bottom_left"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_bottom_left.png");
    tileHandles["bottom_right"] = LoadGraph("Sprites/Tiles/Default/terrain_dirt_block_bottom_right.png");
}

void TitleGenerate::SetupMap() {
    mapLayout = {
        {"top_left", "top", "top", "top", "top_right"},
        {"left", "center", "center", "center", "right"},
        {"left", "center", "center", "center", "right"},
        {"bottom_left", "bottom", "bottom", "bottom", "bottom_right"}
    };
}

void TitleGenerate::DrawMap() {
    const int TILE_SIZE = 64;

    const int mapHeight = static_cast<int>(mapLayout.size());
    const int mapWidth = static_cast<int>(mapLayout[0].size());

    // 画面サイズ
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    // ステージの描画開始位置（中央横揃え＋下揃え）
    const int offsetX = (screenWidth - mapWidth * TILE_SIZE) / 2;
    const int offsetY = screenHeight - mapHeight * TILE_SIZE;

    for (int y = 0; y < mapLayout.size(); ++y) {
        for (int x = 0; x < mapLayout[y].size(); ++x) {
            const std::string& tile = mapLayout[y][x];
            if (tileHandles.count(tile)) {
                int drawX = offsetX + x * TILE_SIZE;
                int drawY = offsetY + y * TILE_SIZE;
                DrawGraph(drawX, drawY, tileHandles[tile], TRUE);
            }
        }
    }
}

