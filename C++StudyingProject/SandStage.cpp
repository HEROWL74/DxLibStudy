#include "SandStage.h"

SandStage::SandStage() {
    terrainType = "sand";
}

SandStage::~SandStage() {
    // 基底クラスのデストラクタが呼ばれる
}

void SandStage::Initialize() {
    // メインテクスチャ読み込み（地面用）
    LoadTerrainTextures();

    // **プラットフォーム専用テクスチャ読み込み（terrain_sand_block.png）**
    LoadPlatformTexture();

    // 地形生成
    CreateMainTerrain();
    CreatePlatforms();
}

void SandStage::CreateMainTerrain() {
    // 砂漠の起伏のある地形
    int groundHeight = 3;

    // 起伏のある地面 - より登りやすく調整
    CreatePlatform(0, 13, 15, groundHeight);     // 平い地面
    CreatePlatform(15, 11, 10, groundHeight + 2); // 少し高い地面
    CreatePlatform(25, 14, 12, groundHeight);    // また平い地面
    CreatePlatform(37, 10, 12, groundHeight + 3); // 高い丘
    CreatePlatform(49, 12, 15, groundHeight + 1); // 中程度の高さ
    CreatePlatform(64, 9, 15, groundHeight + 4);  // さらに高い丘
    CreatePlatform(79, 11, 15, groundHeight + 2); // 下り坂
    CreatePlatform(94, 13, 26, groundHeight);     // 最後の平地
}

void SandStage::CreatePlatforms() {
    // **砂漠のオアシス風のプラットフォーム（terrain_sand_block.pngテクスチャ使用）**
    CreatePlatformBlock(5, 9, 4, 1);   // 小さな島
    CreatePlatformBlock(12, 7, 3, 1);  // ジャンプ台
    CreatePlatformBlock(20, 8, 5, 1);  // 中くらいの島
    CreatePlatformBlock(30, 6, 4, 1);  // 高い足場

    // ピラミッド風の構造 - より登りやすく
    CreatePlatformBlock(42, 8, 1, 1);  // 頂上
    CreatePlatformBlock(41, 9, 3, 1);  // 2段目
    CreatePlatformBlock(40, 10, 5, 1); // 1段目（実際は3段目）
    CreatePlatformBlock(39, 11, 7, 1); // 基部

    // 砂丘を表現するプラットフォーム - ジャンプしやすい配置
    CreatePlatformBlock(55, 7, 3, 1);  // 砂丘の頂上
    CreatePlatformBlock(52, 9, 2, 1);  // 砂丘の中腹
    CreatePlatformBlock(58, 9, 3, 1);  // 砂丘の反対側

    // 追加の足場
    CreatePlatformBlock(68, 6, 4, 1);  // 高い足場
    CreatePlatformBlock(75, 8, 3, 1);  // 中間の足場
    CreatePlatformBlock(85, 5, 3, 1);  // さらに高い足場
    CreatePlatformBlock(92, 9, 2, 1);  // ゴール前の足場

    // サボテンのような細い足場
    CreatePlatformBlock(26, 12, 1, 1);
    CreatePlatformBlock(47, 7, 1, 1);
    CreatePlatformBlock(73, 4, 1, 1);
    CreatePlatformBlock(98, 10, 1, 1);
}