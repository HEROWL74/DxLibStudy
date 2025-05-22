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
    const int width = 30;  // 1920 / 64
    const int height = 4;  //自在に変更可能

    mapLayout.clear();
    mapLayout.resize(height, std::vector<std::string>(width, "center"));

    // 上段
    mapLayout[0][0] = "top_left";
    mapLayout[0][width - 1] = "top_right";
    for (int x = 1; x < width - 1; ++x) {
        mapLayout[0][x] = "top";
    }

    // 中段
    for (int y = 1; y < height - 1; ++y) {
        mapLayout[y][0] = "left";
        mapLayout[y][width - 1] = "right";
    }

    // 下段
    mapLayout[height - 1][0] = "bottom_left";
    mapLayout[height - 1][width - 1] = "bottom_right";
    for (int x = 1; x < width - 1; ++x) {
        mapLayout[height - 1][x] = "bottom";
    }
}


void TitleGenerate::DrawMap() {
    const int TILE_SIZE = 64;
    const int mapHeight = static_cast<int>(mapLayout.size());

    const int offsetX = 0;  // 左端から描画を開始
    const int offsetY = 1080 - mapHeight * TILE_SIZE;  // 画面下揃え

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


