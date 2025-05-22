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
    for (int y = 0; y < mapLayout.size(); ++y) {
        for (int x = 0; x < mapLayout[y].size(); ++x) {
            const std::string& tile = mapLayout[y][x];
            if (tileHandles.count(tile)) {
                DrawGraph(x * TILE_SIZE, y * TILE_SIZE, tileHandles[tile], TRUE);
            }
        }
    }
}
