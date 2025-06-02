#include "PurpleStage.h"

PurpleStage::PurpleStage() {
    terrainType = "purple";
}

PurpleStage::~PurpleStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void PurpleStage::Initialize() {
    // テクスチャ読み込み
    LoadTerrainTextures();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void PurpleStage::CreateMainTerrain() {
    // 魔法の世界風の浮遊する地形
    int groundHeight = 2;

    // 浮遊する島々
    CreatePlatform(0, 13, 10, groundHeight + 1);   // スタート地点
    CreatePlatform(15, 11, 8, groundHeight);     // 最初の浮遊島
    CreatePlatform(28, 9, 6, groundHeight);      // 高い浮遊島
    CreatePlatform(38, 12, 10, groundHeight + 1);  // 大きな浮遊島
    CreatePlatform(52, 7, 8, groundHeight + 2);    // 最後の高い島
}

void PurpleStage::CreatePlatforms() {
    // 魔法的なプラットフォーム
    CreatePlatform(8, 9, 2, 1);    // 小さな魔法の足場
    CreatePlatform(12, 7, 1, 1);   // 単独の魔法ブロック
    CreatePlatform(20, 8, 3, 1);   // 中くらいの足場
    CreatePlatform(25, 6, 1, 1);   // 高い魔法ブロック

    // 魔法の階段
    CreatePlatform(32, 10, 1, 1);  // 1段目
    CreatePlatform(33, 9, 1, 1);   // 2段目
    CreatePlatform(34, 8, 1, 1);   // 3段目
    CreatePlatform(35, 7, 1, 1);   // 4段目

    // 浮遊する魔法の輪
    CreatePlatform(42, 8, 1, 1);
    CreatePlatform(45, 6, 1, 1);
    CreatePlatform(48, 4, 1, 1);
    CreatePlatform(55, 3, 2, 1);   // ゴール付近の足場
}