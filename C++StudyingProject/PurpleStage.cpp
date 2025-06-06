#include "PurpleStage.h"

PurpleStage::PurpleStage() {
    terrainType = "purple";
}

PurpleStage::~PurpleStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void PurpleStage::Initialize() {
    // メインテクスチャ読み込み（地面用）
    LoadTerrainTextures();

    // **プラットフォーム専用テクスチャ読み込み（terrain_purple_block.png）**
    LoadPlatformTexture();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void PurpleStage::CreateMainTerrain() {
    // 魔法の異世界風の浮遊する地形
    int groundHeight = 2;

    // 浮遊する島々 - より渡りやすく調整
    CreatePlatform(0, 13, 12, groundHeight + 1);   // スタート地点
    CreatePlatform(15, 11, 10, groundHeight + 1);  // 最初の浮遊島
    CreatePlatform(28, 9, 8, groundHeight + 1);    // 高い浮遊島
    CreatePlatform(39, 12, 12, groundHeight + 1);  // 大きな浮遊島
    CreatePlatform(54, 8, 10, groundHeight + 2);   // 最高の高い島
    CreatePlatform(67, 10, 12, groundHeight + 1);  // 中間の島
    CreatePlatform(82, 12, 15, groundHeight + 1);  // 大きな島
    CreatePlatform(100, 9, 20, groundHeight + 1);  // ゴール前の島
}

void PurpleStage::CreatePlatforms() {
    // **魔法的なプラットフォーム - terrain_purple_block.pngテクスチャ使用**
    // ジャンプしやすい配置
    CreatePlatformBlock(8, 9, 3, 1);    // 小さな魔法の足場
    CreatePlatformBlock(12, 7, 2, 1);   // 単独の魔法ブロック
    CreatePlatformBlock(20, 8, 4, 1);   // 中くらいの足場
    CreatePlatformBlock(26, 6, 2, 1);   // 高い魔法ブロック

    // 魔法の階段 - より登りやすく
    CreatePlatformBlock(33, 10, 2, 1);  // 1段目
    CreatePlatformBlock(35, 9, 2, 1);   // 2段目
    CreatePlatformBlock(37, 8, 2, 1);   // 3段目
    CreatePlatformBlock(39, 7, 2, 1);   // 4段目

    // 浮遊する魔法の粒 - ジャンプしやすい間隔
    CreatePlatformBlock(45, 8, 2, 1);
    CreatePlatformBlock(48, 6, 2, 1);
    CreatePlatformBlock(51, 4, 2, 1);
    CreatePlatformBlock(58, 6, 3, 1);   // ゴール付近の足場

    // 魔法の橋
    CreatePlatformBlock(72, 8, 6, 1);   // 長い魔法の橋
    CreatePlatformBlock(85, 6, 4, 1);   // 高い魔法の足場
    CreatePlatformBlock(92, 9, 3, 1);   // 中間の足場
    CreatePlatformBlock(105, 7, 4, 1);  // ゴール前の魔法足場

    // 魔法の塔
    CreatePlatformBlock(24, 11, 1, 1);
    CreatePlatformBlock(43, 5, 1, 1);
    CreatePlatformBlock(62, 8, 1, 1);
    CreatePlatformBlock(88, 4, 1, 1);
    CreatePlatformBlock(110, 10, 1, 1);
}